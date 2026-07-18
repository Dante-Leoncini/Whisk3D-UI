#include "PropList.h"
#include "w3dGraphics.h"
#include "objects/Armature.h"                 // modo 5: clips de animacion del esqueleto
#include "animation/SkeletalAnimation.h"      // SkeletalAnimation (name de cada clip)

PropList::PropList(const std::string& Name):
    PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    selectIndex = 0;
    originalIndex = 0;

    listBox = new Card(NULL, 300, 300);
};

PropertyType PropList::GetType(){
    return PropertyType::List;
}

extern void RebindMaterialMeshPart(); // Properties.cpp

PropListMeshParts::PropListMeshParts(const std::string& Name):
    PropList(Name){
    mesh = NULL; // (era inicializador de clase: C++03)
    arm = NULL;  // fuente cuando modo == 5 (clips de animacion del esqueleto)
    filasMax = 3;
    scrollFila = 0;
    modo = 0;    // por defecto: mesh parts (materialsGroup)
};

// la MISMA lista sirve para parts / uv maps / color layers / anim clips (segun 'modo')
int PropListMeshParts::ListaCount() const {
    if (modo == 5) return arm ? (int)arm->animations.size() : 0; // clips de animacion (armature)
    if (modo == 6) return arm ? (int)arm->bones.size() : 0;      // huesos (Pose Mode)
    if (!mesh) return 0;
    if (modo == 1) return (int)mesh->uvMaps.size();
    if (modo == 2) return (int)mesh->colorLayers.size();
    if (modo == 3) return (int)mesh->modificadores.size();    // stack de modificadores
    if (modo == 4) return (int)mesh->vertexGroups.size();     // grupos de vertices (huesos del rig)
    return (int)mesh->materialsGroup.size();
}
std::string PropListMeshParts::ListaNombre(int i) const {
    if (modo == 5) return (arm && i >= 0 && i < (int)arm->animations.size()) ? arm->animations[i]->name : std::string();
    if (modo == 6) return (arm && i >= 0 && i < (int)arm->bones.size()) ? arm->bones[i].name : std::string();
    if (!mesh || i < 0) return std::string();
    if (modo == 1) return (i < (int)mesh->uvMaps.size())      ? mesh->uvMaps[i]->nombre   : std::string();
    if (modo == 2) return (i < (int)mesh->colorLayers.size()) ? mesh->colorLayers[i]->nombre : std::string();
    if (modo == 3) return mesh->NombreModificador(i);         // sin exponer la clase Modifier (metodo del Mesh)
    if (modo == 4) return (i < (int)mesh->vertexGroups.size())? mesh->vertexGroups[i]->nombre : std::string();
    return (i < (int)mesh->materialsGroup.size())             ? mesh->materialsGroup[i].name : std::string();
}
void PropListMeshParts::ListaSeleccionar(int i) {
    selectIndex = i;
    if (modo == 5) { if (arm){ arm->animActiva = i;                              // clip: activo + sincroniza la seleccion
                               if (OnSeleccionarAnimClip) OnSeleccionarAnimClip(arm, i); } return; } // app-wide (timeline + Start/End)
    if (modo == 6) { if (arm){ for (size_t b=0;b<arm->bones.size();b++) arm->bones[b].select = false;  // hueso: activo + select
                               if (i>=0 && i<(int)arm->bones.size()){ arm->bones[i].select = true; arm->boneActivo = i; } } return; }
    if (!mesh) return;
    if (modo == 1)      { mesh->uvMapActivo = i; mesh->AplicarCapasAlRender(); } // re-hornea la UV activa
    else if (modo == 2) { mesh->colorActivo = i; mesh->AplicarCapasAlRender(); } // re-hornea el color activo
    else if (modo == 3) { mesh->modificadorActivo = i; }                         // modificadores: solo cambia el activo
    else if (modo == 4) { mesh->grupoActivo = i; }                               // grupos de vertices: solo el activo
    else RebindMaterialMeshPart();                                              // mesh parts: re-bind material
}

// la ventana sigue a la seleccion (y nunca pasa del final)
void PropListMeshParts::AjustarVentana(){
    if (ListaCount() == 0) { scrollFila = 0; return; }
    int n = ListaCount();
    int visibles = n < filasMax ? n : filasMax;
    if (scrollFila > n - visibles) scrollFila = n - visibles;
    if (scrollFila < 0) scrollFila = 0;
    if (selectIndex < scrollFila) scrollFila = selectIndex;
    if (selectIndex >= scrollFila + visibles) scrollFila = selectIndex - visibles + 1;
};

void PropListMeshParts::RenderPropertiBox(Card* propertiBox){
    if (ListaCount() == 0) return; // lista vacia: no ocupa fila ni se dibuja (ver Resize)
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, 0, 0);
    listBox->Render(false);
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, listBox->height + gapGS, 0);
}

void PropListMeshParts::RenderPropertiBoxBorder(Card* propertiBox){
    if (ListaCount() == 0) return;
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, -listBox->height - gapGS, 0);
    listBox->RenderBorder(false);
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, listBox->height + gapGS, 0);
};

void PropListMeshParts::RenderPropertiValue(Card* propertiBox){
    if (ListaCount() == 0) return;
    w3dEngine::Translatef(0, listBox->height + gapGS, 0);
}

void PropListMeshParts::RenderPropertiLabel(Card* propertiBox){
    if (ListaCount() == 0) return; // lista vacia: sin label/scroll
    {
        int n = ListaCount();
        int visibles = n < filasMax ? n : filasMax;
        w3dEngine::Translatef((GLfloat)(-PropColEtiqueta + gapGS + gapGS), borderGS, 0);

        // mini barra de scroll cuando hay mas filas que la ventana
        if (n > visibles && visibles > 0){
            static Card* thumb = NULL;
            if (!thumb) thumb = new Card(NULL, 4, 10);
            int track = visibles * (RenglonHeightGS + gapGS) - gapGS;
            int th = track * visibles / n;
            if (th < GlobalScale * 6) th = GlobalScale * 6;
            int ty = (n - visibles) > 0
                     ? (track - th) * scrollFila / (n - visibles) : 0;
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - gapGS * 3), (GLfloat)ty, 0);
            thumb->Resize(GlobalScale * 3, th);
            SetColorID(ColorID::grisUI, 0.6f);
            thumb->RenderObject(false);
            w3dEngine::PopMatrix();
        }

        SetColorID(ColorID::grisUI, 1.0f);

        // solo la VENTANA visible (culling de filas enteras)
        for (int i = scrollFila; i < scrollFila + visibles && i < n; ++i){
            if (selectIndex == i){
                SetColorID(ColorID::accent, 1.0f);
            }
            RenderBitmapText(ListaNombre(i), textAlign::left, width -bordersGS);

            if (selectIndex == i){
                SetColorID(ColorID::grisUI, 1.0f);
            }
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
        w3dEngine::Translatef((GLfloat)(PropColEtiqueta - gapGS - gapGS), borderGS, 0);
    }
}

void PropListMeshParts::button_left(){};
void PropListMeshParts::button_right(){};

bool PropList::Cancel(){
    selectIndex = originalIndex;
    editando = false;
    return editando;
};

bool PropList::EditPropertie(){
    editando = !editando;
    if (editando){
        originalIndex = selectIndex;
    }
    return editando;
};

void PropListMeshParts::button_up(){
    int n = ListaCount(); if (n == 0) return;
    int i = selectIndex - 1; if (i < 0) i = n - 1;
    ListaSeleccionar(i); AjustarVentana(); // setea el activo (segun modo) + sigue a la seleccion
};

void PropListMeshParts::button_down(){
    int n = ListaCount(); if (n == 0) return;
    int i = selectIndex + 1; if (i >= n) i = 0;
    ListaSeleccionar(i); AjustarVentana();
};

int PropListMeshParts::Resize(int w){
    width = w -bordersGS;
    // LISTA VACIA (ej: modificadores sin ninguno): NO ocupa fila (0) y no se dibuja -> queda solo el boton "Add",
    // sin el recuadro aplastado y vacio que parecia un error (pedido Dante). Igual que un PropText 'oculto'.
    if (ListaCount() == 0){ listBox->Resize(w - bordersGS, 0); return 0; }
    int altura = bordersGS;
    // los modos uvmaps/colors/anim siguen el ACTIVO de su fuente (selectIndex refleja uvMapActivo/colorActivo/etc)
    if (mesh && modo == 1) selectIndex = mesh->uvMapActivo;
    else if (mesh && modo == 2) selectIndex = mesh->colorActivo;
    else if (mesh && modo == 4) selectIndex = mesh->grupoActivo;
    else if (arm && modo == 5) selectIndex = arm->animActiva;
    else if (arm && modo == 6) selectIndex = arm->boneActivo;
    if (ListaCount() > 0){
        // VENTANA de filasMax filas como maximo (el resto scrollea)
        int n = ListaCount();
        int visibles = n < filasMax ? n : filasMax;
        altura += (RenglonHeightGS + gapGS) * visibles - gapGS;
        if (scrollFila > n - visibles) scrollFila = n - visibles;
        if (scrollFila < 0) scrollFila = 0;
    }

    listBox->Resize(w- bordersGS, altura);

    return altura + gapGS;
}