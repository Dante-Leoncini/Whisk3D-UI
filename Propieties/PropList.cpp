#include "PropList.h"
#include "w3dGraphics.h"
#include "objects/Armature.h"                 // modo 5: clips de animacion del esqueleto
#include "animation/SkeletalAnimation.h"      // SkeletalAnimation (name de cada clip)
#include "animation/VertexAnimation.h"        // modo 7: animaciones del objeto + hooks
#include "animation/Armature2DAnimation.h"    // modo 10: Arm2DSetActivo (cambiar de rig 2D recarga el rango)
#include "script/W3dScript.h"                 // modo 12: W3dScriptDatos (los scripts lua del objeto)
#include "WhiskUI/draw/icons.h"               // modo 13: el ojo de cada capa del mix
#include "WhiskUI/draw/glesdraw.h"            // W3dDrawStrip4 (el icono)
#include "WhiskUI/theme/colores.h"            // modo 14: el gris del checkbox apagado
#include "WhiskUI/widgets/Button.h"           // el campo de renombrar en linea

// modo 13 (arbol del MIX): los instala Properties.cpp
int  (*MixFilasCount)() = 0;
std::string (*MixFilaTexto)(int) = 0;
int  (*MixFilaNivel)(int) = 0;
int  (*MixFilaOjo)(int) = 0;
void (*MixFilaSeleccionar)(int) = 0;
int (*RutinaFilasCount)(Object*) = 0;
std::string (*RutinaFilaTexto)(Object*, int) = 0;
void (*RutinaFilaSeleccionar)(Object*, int) = 0;
int (*RutinaFilaIcono)(Object*, int) = 0;
int (*RutinaFilaValor)(Object*, int, float*) = 0;
int g_rutinaFilaDestino = -1;
int (*CapaTexFilasCount)() = 0;
std::string (*CapaTexFilaTexto)(int) = 0;
void (*CapaTexFilaSeleccionar)(int) = 0;
int (*CapaTexFilaActiva)() = 0;
int (*CapaMatFilasCount)() = 0;
std::string (*CapaMatFilaTexto)(int) = 0;
void (*CapaMatFilaSeleccionar)(int) = 0;
int (*CapaMatFilaActiva)() = 0;
int  (*MixFilaActiva)() = 0;
int  (*MixFilaPlegada)(int) = 0;
int  (*MixFilaIcono)(int) = 0;

PropList::PropList(const std::string& Name):
    PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    selectIndex = 0;
    originalIndex = 0;

    listBox = new Card(NULL, 300, 300);
};

// las listas viven lo que su tarjeta: el panel de propiedades se libera con el layout
// (abrir un proyecto) y cada lista dejaba su caja viva
PropList::~PropList(){
    delete listBox;
    listBox = NULL;
}

PropertyType PropList::GetType(){
    return PropertyType::List;
}

extern void RebindMaterialMeshPart(); // Properties.cpp

PropListMeshParts::PropListMeshParts(const std::string& Name):
    PropList(Name){
    mesh = NULL; // (era inicializador de clase: C++03)
    arm = NULL;  // fuente cuando modo == 5 (clips de animacion del esqueleto)
    obj = NULL;  // fuente cuando modo == 11 (stack de constraints del objeto)
    filasMax = 3;
    scrollFila = 0;
    modo = 0;    // por defecto: mesh parts (materialsGroup)
    opBorrar = opRenombrar = opSubir = opBajar = NULL; opSoloLectura = NULL; opCambiar = NULL;
    minItems = 0; nExtras = 0;
    renBoton = new Button("", -1, false);
    renFila = -1; filaDestino = -1;
    altoFijo = false; minFilas = 0; mostrarVacia = false; oculto = false; altoPx = 0; asaEstado = 0; sbEstado = 0; hoverFila = -1;
    botonesLado = false;
    ladoBtn[0] = new Button("", (int)IconType::arrow, false); ladoBtn[0]->iconFlip = 2;   // subir (la flecha, dada vuelta)
    ladoBtn[1] = new Button("", (int)IconType::arrow, false);                             // bajar
    ladoBtn[2] = new Button("", (int)IconType::lista, false);                             // opciones
    for (int k = 0; k < 3; k++) { ladoBtn[k]->altoRenglon = true; ladoBtn[k]->centrado = true; }
    ladoBtn[0]->ayuda = "Move Up"; ladoBtn[1]->ayuda = "Move Down"; ladoBtn[2]->ayuda = "Options";   // (el cartelito)
};
bool PropListMeshParts::LadoHabilitado(int k) const {
    const int n = ListaCount();
    if (n <= 0 || selectIndex < 0 || selectIndex >= n) return false;
    const bool mueve = Reordenable();
    if (k == 0) return mueve && selectIndex > 0;
    if (k == 1) return mueve && selectIndex < n - 1;
    return true;
}
bool PropListMeshParts::ScrollGeo(int* riel, int* agarre, int* pos) const {
    const int n = ListaCount(), vis = n < FilasAlto() ? n : FilasAlto();
    if (n <= vis || vis <= 0) return false;
    const int track = vis * (RenglonHeightGS + gapGS) - gapGS;
    int th = track * vis / n; if (th < GlobalScale * 6) th = GlobalScale * 6;
    if (riel) *riel = track; if (agarre) *agarre = th;
    if (pos) *pos = (track - th) * scrollFila / (n - vis);
    return true;
}
void PropListMeshParts::ScrollDesdeY(int y) {
    int track = 0, th = 0;
    if (!ScrollGeo(&track, &th, NULL)) return;
    const int n = ListaCount(), vis = n < FilasAlto() ? n : FilasAlto();
    const int rango = track - th;
    int t = y - th / 2; if (t < 0) t = 0; if (t > rango) t = rango;
    scrollFila = rango > 0 ? (t * (n - vis) + rango / 2) / rango : 0;
    if (scrollFila < 0) scrollFila = 0; if (scrollFila > n - vis) scrollFila = n - vis;
}
int PropListMeshParts::AltoFilasPx() const {
    const int rowH = RenglonHeightGS + gapGS;
    if (altoFijo && altoPx > 0) { const int mn = (minFilas > 0 ? minFilas : 1) * rowH - gapGS; return altoPx < mn ? mn : altoPx; }
    return FilasAlto() * rowH - gapGS;
}
int PropListMeshParts::FilasAlto() const {
    if (altoFijo && altoPx > 0) {   // (a mano, en pixeles: los renglones ENTEROS que entran)
        const int rowH = RenglonHeightGS + gapGS;
        const int mn = (minFilas > 0 ? minFilas : 1) * rowH - gapGS;
        const int f = ((altoPx < mn ? mn : altoPx) + gapGS) / rowH;
        return f < 1 ? 1 : f;
    }
    int f = filasMax;
    if (f < minFilas) f = minFilas;
    if (!altoFijo && minFilas == 0) { const int n = ListaCount(); if (n < f) f = n; }
    return f < 1 ? 1 : f;
}
int PropListMeshParts::AnchoLado() const { return botonesLado ? RenglonHeightGS + GlobalScale * 4 + gapGS : 0; }
void PropListMeshParts::AgregarExtra(const char* texto, void (*accion)(), int icono, bool soloEdit) {
    if (nExtras >= 4) return;
    extras[nExtras].texto = texto; extras[nExtras].accion = accion; extras[nExtras].icono = icono; extras[nExtras].soloEdit = soloEdit;
    nExtras++;
}
bool PropListMeshParts::ConTacho() const {
    return opBorrar && ListaCount() > minItems && !(opSoloLectura && opSoloLectura());
}
bool PropListMeshParts::Reordenable() const {
    return opSubir && opBajar && ListaCount() > 1 && !(opSoloLectura && opSoloLectura());
}
int PropListMeshParts::Asa() { return GlobalScale * 8; }

// la MISMA lista sirve para parts / uv maps / color layers / anim clips / vertex groups /
// UV groups (segun 'modo'). Los dos grupos de pesos son entidades DISTINTAS (ver Mesh.h):
// modo 4 = vertexGroups (por control-point, rig 3D); modo 9 = uvGroups (por corner, rig 2D).
int (*PostFilasCount)() = NULL;
std::string (*PostFilaTexto)(int) = NULL;
void (*PostFilaSeleccionar)(int) = NULL;
int (*CurvaModFilasCount)() = NULL;
std::string (*CurvaModFilaTexto)(int) = NULL;
void (*CurvaModFilaSeleccionar)(int) = NULL;
int PropListMeshParts::ListaCount() const {
    if (modo == 17) return CapaTexFilasCount ? CapaTexFilasCount() : 0;   // las capas de textura del material
    if (modo == 18) return CapaMatFilasCount ? CapaMatFilasCount() : 0;   // las capas de material del mesh part
    if (modo == 16) return CurvaModFilasCount ? CurvaModFilasCount() : 0;   // los modificadores de la curva activa
    if (modo == 15) return PostFilasCount ? PostFilasCount() : 0;   // la pila de postprocesado
    if (modo == 13) return MixFilasCount ? MixFilasCount() : 0;   // arbol del MIX
    if (modo == 14) return RutinaFilasCount ? RutinaFilasCount(obj) : 0;   // pasos de una rutina
    if (modo == 5) return arm ? (int)arm->animations.size() : 0; // clips de animacion (armature)
    if (modo == 7) return mesh ? (int)mesh->animations.size() : 0; // animaciones del objeto (vertex anims)
    if (modo == 6) return arm ? (int)arm->bones.size() : 0;      // huesos (Pose Mode)
    if (modo == 8) return mesh ? (int)mesh->Arm2DHuesos().size() : 0; // huesos 2D del ARMATURE 2D ACTIVO
    if (modo == 10) return mesh ? (int)mesh->armatures2d.size() : 0;  // ARMATURES 2D del mesh (una malla puede tener varios)
    if (modo == 11) return obj ? obj->ConstraintsCount() : 0;         // constraints: los tiene CUALQUIER objeto, va por 'obj'
    if (modo == 12) return (obj && obj->scriptDatos) ? (int)obj->scriptDatos->scripts.size() : 0; // scripts lua
    if (!mesh) return 0;
    if (modo == 1) return (int)mesh->uvMaps.size();
    if (modo == 2) return (int)mesh->colorLayers.size();
    if (modo == 3) return (int)mesh->modificadores.size();    // stack de modificadores
    if (modo == 4) return (int)mesh->vertexGroups.size();     // grupos de vertices (huesos del rig 3D)
    if (modo == 9) return (int)mesh->uvGroups.size();         // UV groups (pesos por corner: rig 2D)
    return (int)mesh->materialsGroup.size();
}
std::string PropListMeshParts::ListaNombre(int i) const {
    if (modo == 17) return CapaTexFilaTexto ? CapaTexFilaTexto(i) : std::string();
    if (modo == 18) return CapaMatFilaTexto ? CapaMatFilaTexto(i) : std::string();
    if (modo == 16) return CurvaModFilaTexto ? CurvaModFilaTexto(i) : std::string();
    if (modo == 15) return PostFilaTexto ? PostFilaTexto(i) : std::string();
    if (modo == 13) return MixFilaTexto ? MixFilaTexto(i) : std::string();
    if (modo == 14) return RutinaFilaTexto ? RutinaFilaTexto(obj, i) : std::string();
    if (modo == 5) return (arm && i >= 0 && i < (int)arm->animations.size()) ? arm->animations[i]->name : std::string();
    if (modo == 7) return (mesh && i >= 0 && i < (int)mesh->animations.size() && mesh->animations[i]) ? mesh->animations[i]->name : std::string();
    if (modo == 6) return (arm && i >= 0 && i < (int)arm->bones.size()) ? arm->bones[i].name : std::string();
    if (modo == 8) return (mesh && i >= 0 && i < (int)mesh->Arm2DHuesos().size()) ? mesh->Arm2DHuesos()[i].nombre : std::string();
    if (modo == 10) return (mesh && i >= 0 && i < (int)mesh->armatures2d.size() && mesh->armatures2d[i]) ? mesh->armatures2d[i]->nombre : std::string();
    if (modo == 11) return obj ? obj->NombreConstraint(i) : std::string(); // (el accesor ya clampea el indice)
    if (modo == 12) {   // scripts lua: se muestra el NOMBRE DE ARCHIVO (la ruta no entra ni sirve)
        if (!obj || !obj->scriptDatos) return std::string();
        if (i < 0 || i >= (int)obj->scriptDatos->scripts.size()) return std::string();
        const std::string& r = obj->scriptDatos->scripts[i].ruta;
        size_t s = r.find_last_of("/\\");
        return (s == std::string::npos) ? r : r.substr(s + 1);
    }
    if (!mesh || i < 0) return std::string();
    if (modo == 1) return (i < (int)mesh->uvMaps.size())      ? mesh->uvMaps[i]->nombre   : std::string();
    if (modo == 2) return (i < (int)mesh->colorLayers.size()) ? mesh->colorLayers[i]->nombre : std::string();
    if (modo == 3) return mesh->NombreModificador(i);         // sin exponer la clase Modifier (metodo del Mesh)
    if (modo == 4) return (i < (int)mesh->vertexGroups.size())? mesh->vertexGroups[i]->nombre : std::string();
    if (modo == 9) return (i < (int)mesh->uvGroups.size())    ? mesh->uvGroups[i]->nombre : std::string();
    return (i < (int)mesh->materialsGroup.size())             ? mesh->materialsGroup[i].name : std::string();
}
void PropListMeshParts::ListaSeleccionar(int i) {
    selectIndex = i;
    if (modo == 17) { if (CapaTexFilaSeleccionar) CapaTexFilaSeleccionar(i); return; }
    if (modo == 18) { if (CapaMatFilaSeleccionar) CapaMatFilaSeleccionar(i); return; }
    if (modo == 16) { if (CurvaModFilaSeleccionar) CurvaModFilaSeleccionar(i); return; }
    if (modo == 15) { if (PostFilaSeleccionar) PostFilaSeleccionar(i); return; }
    if (modo == 13) { if (MixFilaSeleccionar) MixFilaSeleccionar(i); return; }
    if (modo == 14) { if (RutinaFilaSeleccionar) RutinaFilaSeleccionar(obj, i); return; }
    if (modo == 5) { if (arm){ arm->animActiva = i;                              // clip: activo + sincroniza la seleccion
                               if (OnSeleccionarAnimClip) OnSeleccionarAnimClip(arm, i); } return; } // app-wide (timeline + Start/End)
    if (modo == 7) { if (mesh && OnSeleccionarAnimVertex) OnSeleccionarAnimVertex(mesh, i); return; } // anim del objeto -> timeline
    if (modo == 6) { if (arm){ for (size_t b=0;b<arm->bones.size();b++) arm->bones[b].select = false;  // hueso: activo + select
                               if (i>=0 && i<(int)arm->bones.size()){ arm->bones[i].select = true; arm->boneActivo = i; } } return; }
    if (modo == 8) { if (mesh){ // hueso 2D: activo + hueso ENTERO seleccionado (ambas puntas, como el click de cuerpo)
                               for (size_t b=0;b<mesh->Arm2DHuesos().size();b++){ W3dBone2D& hb = mesh->Arm2DHuesos()[b];
                                   hb.select = false; hb.selHead = false; hb.selTail = false; }
                               if (i>=0 && i<(int)mesh->Arm2DHuesos().size()){ W3dBone2D& hb = mesh->Arm2DHuesos()[i];
                                   hb.select = true; hb.selHead = true; hb.selTail = true; mesh->Arm2DBoneActivo() = i; } } return; }
    // armature 2D activo: SIEMPRE por Arm2DSetActivo (escribir el indice a mano dejaba el timeline
    // con el Start/End/FPS del clip del rig ANTERIOR -> el Play usaba el rango equivocado y editar
    // el campo End lo escribia en el clip del rig nuevo)
    if (modo == 10) { Arm2DSetActivo(mesh, i); return; }
    // constraints: SIEMPRE por SetConstraintActivo (clampea; -1 = ninguno). Elegir uno no
    // recalcula nada -- el stack se evalua al vuelo al dibujar, no hornea geometria.
    if (modo == 11) { if (obj) obj->SetConstraintActivo(i); return; }
    // scripts lua: elegir uno solo cambia CUAL se muestra abajo (no re-ejecuta nada;
    // el orden de la lista sigue siendo el orden de ejecucion)
    if (modo == 12) { if (obj && obj->scriptDatos) obj->scriptDatos->activo = i; return; }
    if (!mesh) return;
    if (modo == 1)      { mesh->uvMapActivo = i; mesh->AplicarCapasAlRender(); } // re-hornea la UV activa
    else if (modo == 2) { mesh->colorActivo = i; mesh->AplicarCapasAlRender(); } // re-hornea el color activo
    else if (modo == 3) { mesh->modificadorActivo = i; }                         // modificadores: solo cambia el activo
    else if (modo == 4) { mesh->grupoActivo = i; }                               // grupos de vertices: solo el activo
    else if (modo == 9) { mesh->uvGrupoActivo = i; }                             // UV groups: solo el activo
    else RebindMaterialMeshPart();                                              // mesh parts: re-bind material
}

// la ventana sigue a la seleccion (y nunca pasa del final)
void PropListMeshParts::AjustarVentana(){
    if (ListaCount() == 0) { scrollFila = 0; return; }
    int n = ListaCount();
    const int alto = FilasAlto();
    int visibles = n < alto ? n : alto;
    if (scrollFila > n - visibles) scrollFila = n - visibles;
    if (scrollFila < 0) scrollFila = 0;
    if (selectIndex < scrollFila) scrollFila = selectIndex;
    if (selectIndex >= scrollFila + visibles) scrollFila = selectIndex - visibles + 1;
};

void PropListMeshParts::RenderPropertiBox(Card* propertiBox){
    if (oculto || (ListaCount() == 0 && !mostrarVacia)) return; // lista vacia: no ocupa fila ni se dibuja (ver Resize)
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, 0, 0);
    listBox->Render(false);
    // EL ASA: abajo de la caja, DOS RAYAS finas (la agarradera), un poco mas cortas que la caja, del gris de las lineas de
    // los menus; con el puntero encima, blancas; agarradas, verdes. Arrastrarla (mouse o dedo) agranda o achica la lista
    {
        const GLfloat u = CardUV[5 * 2], v = CardUV[5 * 2 + 1];   // (un texel liso de la tarjeta: sin esquinas)
        const int gs = GlobalScale > 0 ? GlobalScale : 1;
        const int x0 = gs * 3, x1 = listBox->width - gs * 3;
        const int y0 = listBox->height + (Asa() - gs * 3) / 2;
        if (asaEstado == 2) SetColorID(ColorID::accent, 1.0f);
        else if (asaEstado == 1) SetColorID(ColorID::blanco, 1.0f);
        else SetColorID(ColorID::grisLinea, 1.0f);
        for (int k = 0; k < 2; k++) {
            const int yy = y0 + k * gs * 2;
            GLshort q[8] = { (GLshort)x0, (GLshort)yy, (GLshort)x1, (GLshort)yy,
                             (GLshort)x0, (GLshort)(yy + gs), (GLshort)x1, (GLshort)(yy + gs) };
            GLfloat uv[8] = { u, v, u, v, u, v, u, v };
            W3dDrawStrip4(q, uv);
        }
        SetColorID(ColorID::background, 1.0f);
    }
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, listBox->height + gapGS + Asa(), 0);
}

void PropListMeshParts::RenderPropertiBoxBorder(Card* propertiBox){
    if (oculto || (ListaCount() == 0 && !mostrarVacia)) return;
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, -listBox->height - gapGS - Asa(), 0);
    listBox->RenderBorder(false);
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, listBox->height + gapGS + Asa(), 0);
};

void PropListMeshParts::RenderPropertiValue(Card* propertiBox){
    if (oculto || (ListaCount() == 0 && !mostrarVacia)) return;
    w3dEngine::Translatef(0, listBox->height + gapGS + Asa(), 0);
}

void PropListMeshParts::RenderPropertiLabel(Card* propertiBox){
    if (oculto || (ListaCount() == 0 && !mostrarVacia)) return; // lista vacia: sin label/scroll
    w3dEngine::PushMatrix();   // (al final se avanza EXACTO lo que mide la caja: ver abajo)
    {
        int n = ListaCount();
        const int alto = FilasAlto();
        int visibles = n < alto ? n : alto;
        // LOS TRES BOTONES de la derecha (subir, bajar, opciones), a lo alto de la caja
        if (botonesLado) {
            const int colW = AnchoLado() - gapGS;
            int libre = listBox->height;
            // APAGADO (no hay a donde mover): el fondo y el borde, oscuros como el fondo del panel; el icono en un gris que
            // se llega a ver. Apagado no se resalta con el puntero
            static const float kIconoApagado[4] = { 0.36f, 0.36f, 0.36f, 1.0f };
            const float* fondo = ListaColores[static_cast<int>(ColorID::background)];
            for (int k = 0; k < 3; k++) {
                const bool on = LadoHabilitado(k);
                ladoBtn[k]->tinte = on ? NULL : fondo;
                ladoBtn[k]->colorBorde = on ? NULL : fondo;
                ladoBtn[k]->colorTexto = on ? NULL : kIconoApagado;
                if (!on) ladoBtn[k]->hover = false;
            }
            for (int k = 0; k < 3; k++) { ladoBtn[k]->Resize(colW); libre -= ladoBtn[k]->height; }
            (void)libre;
            const int sep = gapGS;          // (un gap fijo: agrandar la lista no los separa)
            int yb = -GlobalScale;          // (arriba de todo, a la altura del borde de la caja)
            for (int k = 0; k < 3; k++) {
                w3dEngine::PushMatrix();
                // (pegados al borde derecho de la tarjeta, como los demas botones: el origen de este pase esta un gap a
                //  la izquierda de la caja)
                w3dEngine::Translatef((GLfloat)(-PropColEtiqueta + gapGS + width + AnchoLado() - ladoBtn[k]->width), (GLfloat)yb, 0);
                ladoBtn[k]->Render();
                w3dEngine::PopMatrix();
                yb += ladoBtn[k]->height + sep;
            }
            SetColorID(ColorID::grisUI, 1.0f);
        }
        // (las filas NO llevan tacho ni flecha: estaban justo donde se agarra la barrita de scroll y se terminaban borrando
        //  partes. Borrar y cambiar estan en el menu de la fila -click derecho, el boton de opciones- y en los botones)
        const bool tacho = false, flecha = false;
        const int wTacho = 0;
        w3dEngine::Translatef((GLfloat)(-PropColEtiqueta + gapGS + gapGS), borderGS, 0);

        // mini barra de scroll cuando hay mas filas que la ventana
        // (como las barras de los paneles: con el puntero encima, mas ancha y blanca; agarrada, verde)
        { int track = 0, th = 0, ty = 0;
          if (ScrollGeo(&track, &th, &ty)) {
            static Card* thumb = NULL;
            if (!thumb) thumb = new Card(NULL, 4, 10);
            const int tw = GlobalScale * (sbEstado ? 5 : 3);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - gapGS * 3 - (tw - GlobalScale * 3)), (GLfloat)ty, 0);
            thumb->Resize(tw, th);
            if (sbEstado == 2) SetColorID(ColorID::accent, 1.0f);
            else if (sbEstado == 1) SetColorID(ColorID::blanco, 1.0f);
            else SetColorID(ColorID::grisUI, 0.6f);
            thumb->RenderObject(false);
            w3dEngine::PopMatrix();
          } }

        SetColorID(ColorID::grisUI, 1.0f);

        // solo la VENTANA visible (culling de filas enteras)
        for (int i = scrollFila; i < scrollFila + visibles && i < n; ++i){
            if (selectIndex == i){
                SetColorID(ColorID::accent, 1.0f);
            } else if (hoverFila == i) {
                SetColorID(ColorID::blanco, 1.0f);   // (el puntero encima: en blanco)
            }
            if (modo == 13) {
                // ARBOL (como el outliner): flecha de plegar + icono en los objetos; en las capas, la LINEA
                // del costado (baja desde la flecha del padre) + icono; el OJO de la capa a la derecha
                int nivel = MixFilaNivel ? MixFilaNivel(i) : 0;
                int pleg  = MixFilaPlegada ? MixFilaPlegada(i) : -1;
                int ico   = MixFilaIcono ? MixFilaIcono(i) : (int)IconType::armature;
                w3dEngine::PushMatrix();
                if (nivel > 0) {
                    // la linea: tapa tambien el hueco entre filas (gapGS) para que se vea continua
                    SetColorID(ColorID::grisUI, 1.0f);
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef(0, (GLfloat)-gapGS, 0);
                    W3dDrawStrip4(IconLineMesh, IconsUV[static_cast<size_t>(IconType::line)]->uvs);
                    w3dEngine::PopMatrix();
                    W3dDrawStrip4(IconLineMesh, IconsUV[static_cast<size_t>(IconType::line)]->uvs);
                    w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                } else if (pleg >= 0) {
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(pleg ? IconType::arrowRight : IconType::arrow)]->uvs);
                    w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                }
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(ico)]->uvs);
                w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                int usado = (int)(IconSizeGS + gapGS) * 2;
                RenderBitmapText(ListaNombre(i), textAlign::left, width - bordersGS - usado - (int)IconSizeGS - gapGS * 4);
                w3dEngine::PopMatrix();
                int ojo = MixFilaOjo ? MixFilaOjo(i) : -1;
                if (ojo >= 0) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(width - (int)IconSizeGS - gapGS * 4), 0, 0);
                    SetColorID(ColorID::grisUI, ojo ? 1.0f : 0.45f);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(ojo ? IconType::visible : IconType::hidden)]->uvs);
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
                }
            } else if (modo == 14 && RutinaFilaIcono && RutinaFilaIcono(obj, i) >= 0) {
                // los PASOS de una rutina: el icono de su grupo (Transform = camara, Fog = niebla...), el texto y A LA
                // DERECHA lo que hace de un vistazo: el checkbox de un on/off o un cuadro con su color
                const int lado = CheckboxLado();
                float rgba[4] = { 1, 1, 1, 1 };
                const int valor = RutinaFilaValor ? RutinaFilaValor(obj, i, rgba) : 0;
                w3dEngine::PushMatrix();
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(RutinaFilaIcono(obj, i))]->uvs);
                w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                RenderBitmapText(ListaNombre(i), textAlign::left,
                                 width - bordersGS - (int)IconSizeGS - gapGS - (valor ? lado + gapGS * 5 : 0));
                w3dEngine::PopMatrix();
                if (valor) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(width - lado - gapGS * 4), 0, 0);
                    if (valor == 3) DibujarMuestraColor(lado, rgba);
                    else if (valor == 2) DibujarCheckbox(lado, true);
                    else {   // apagado: la caja en un gris tenue (la del panel es del color del fondo: aca no se veia)
                        const float* g = ListaColores[static_cast<int>(ColorID::grisUI)];
                        const float* f = ListaColores[static_cast<int>(ColorID::background)];
                        float c[4] = { f[0] + (g[0] - f[0]) * 0.3f, f[1] + (g[1] - f[1]) * 0.3f, f[2] + (g[2] - f[2]) * 0.3f, 1.0f };
                        DibujarMuestraColor(lado, c);
                    }
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
                }
                // arrastrando un paso: la linea donde cae (arriba de esta fila)
                if (g_rutinaFilaDestino == i || (g_rutinaFilaDestino == n && i == n - 1)) {
                    static Card* linea = NULL;
                    if (!linea) linea = new Card(NULL, width, GlobalScale * 2);
                    linea->Resize(width - bordersGS, GlobalScale * 2);
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef(0, (GLfloat)(g_rutinaFilaDestino == i ? -gapGS : RenglonHeightGS), 0);
                    SetColorID(ColorID::accent, 1.0f);
                    linea->RenderObject(false);
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
                }
            } else if (modo == 12 && obj && obj->scriptDatos && i < (int)obj->scriptDatos->scripts.size()) {
                // los SCRIPTS: el nombre del archivo y A LA DERECHA el OJO (prendido / apagado: el motor no llama a
                // ninguna de sus funciones). Apagado, el nombre y el ojo van tenues. El click en el ojo lo maneja
                // Properties::ClickEn y su cartelito de ayuda PropsAyudaEn (Properties.cpp)
                const bool prendido = !obj->scriptDatos->scripts[i].apagado;
                const ColorID colTexto = selectIndex == i ? ColorID::accent : ColorID::grisUI;
                SetColorID(colTexto, prendido ? 1.0f : 0.45f);
                RenderBitmapText(ListaNombre(i), textAlign::left, width - bordersGS - (int)IconSizeGS - gapGS * 4);
                w3dEngine::PushMatrix();
                w3dEngine::Translatef((GLfloat)(width - (int)IconSizeGS - gapGS * 4), 0, 0);
                SetColorID(ColorID::grisUI, prendido ? 1.0f : 0.45f);
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(prendido ? IconType::visible : IconType::hidden)]->uvs);
                w3dEngine::PopMatrix();
                SetColorID(colTexto, 1.0f);
            } else if (renFila == i && renBoton && renBoton->editField) {
                // RENOMBRANDO esta fila: el campo de texto en su lugar (el mismo input de los botones "Rename")
                w3dEngine::PushMatrix();
                w3dEngine::Translatef(0, (GLfloat)(-(GlobalScale)), 0);
                renBoton->altoRenglon = true;
                renBoton->Resize(width - bordersGS - gapGS * 3);
                renBoton->Render();
                w3dEngine::PopMatrix();
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
            } else
            RenderBitmapText(ListaNombre(i), textAlign::left, width -bordersGS - wTacho);
            // EL TACHO de la fila (borrarla), a la derecha; y la linea de donde cae una fila que se arrastra
            if ((tacho || flecha) && !(renFila == i && renBoton && renBoton->editField)) {
                const int xT = width - (int)IconSizeGS - gapGS * 4 - (n > visibles ? GlobalScale * 4 : 0);
                SetColorID(ColorID::grisUI, 0.7f);
                if (tacho) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)xT, 0, 0);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::borrar)]->uvs);
                    w3dEngine::PopMatrix();
                }
                if (flecha) {   // (la flecha hacia abajo: cambiar lo de la fila; a la izquierda del tacho)
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(tacho ? xT - (int)IconSizeGS - gapGS * 2 : xT), 0, 0);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::arrow)]->uvs);
                    w3dEngine::PopMatrix();
                }
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
            }
            if (modo != 14 && filaDestino >= 0 && (filaDestino == i || (filaDestino == n && i == n - 1))) {
                static Card* lineaD = NULL;
                if (!lineaD) lineaD = new Card(NULL, width, GlobalScale * 2);
                lineaD->Resize(width - bordersGS, GlobalScale * 2);
                w3dEngine::PushMatrix();
                w3dEngine::Translatef(0, (GLfloat)(filaDestino == i ? -gapGS : RenglonHeightGS), 0);
                SetColorID(ColorID::accent, 1.0f);
                lineaD->RenderObject(false);
                w3dEngine::PopMatrix();
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grisUI, 1.0f);
            }

            if (selectIndex == i || hoverFila == i){
                SetColorID(ColorID::grisUI, 1.0f);
            }
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
    }
    // (antes se avanzaba lo que ocupaban las filas CON contenido: con renglones en blanco -la lista agrandada, las capas
    //  vacias- los textos de todo lo que venia abajo quedaban corridos para arriba, encima de la caja)
    w3dEngine::PopMatrix();
    w3dEngine::Translatef(0, (GLfloat)(listBox->height + gapGS + Asa()), 0);
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
    width = w - bordersGS - AnchoLado();
    // LISTA VACIA (ej: modificadores sin ninguno): NO ocupa fila (0) y no se dibuja -> queda solo el boton "Add",
    // sin el recuadro aplastado y vacio que parecia un error. Igual que un PropText 'oculto'.
    if (oculto || (ListaCount() == 0 && !mostrarVacia)){ listBox->Resize(w - bordersGS, 0); return 0; }
    int altura = bordersGS;
    // los modos uvmaps/colors/anim siguen el ACTIVO de su fuente (selectIndex refleja uvMapActivo/colorActivo/etc)
    if (mesh && modo == 1) selectIndex = mesh->uvMapActivo;
    else if (mesh && modo == 2) selectIndex = mesh->colorActivo;
    else if (mesh && modo == 4) selectIndex = mesh->grupoActivo;
    else if (mesh && modo == 9) selectIndex = mesh->uvGrupoActivo;
    else if (arm && modo == 5) selectIndex = arm->animActiva;
    else if (mesh && modo == 7 && ActivaAnimVertexDe) selectIndex = ActivaAnimVertexDe(mesh);
    else if (arm && modo == 6) selectIndex = arm->boneActivo;
    else if (mesh && modo == 8) selectIndex = mesh->Arm2DBoneActivo();
    else if (mesh && modo == 10) selectIndex = mesh->armature2dActivo;
    // constraints: la lista sigue al ACTIVO del objeto. Va aca (y no solo en el helper del
    // panel, como los modificadores) porque el stack es POR OBJETO: cambiar de objeto
    // seleccionado tiene que mover el resaltado sin que haga falta tocar ningun boton.
    else if (obj && modo == 11) selectIndex = obj->constraintActivo;
    else if (modo == 13) selectIndex = MixFilaActiva ? MixFilaActiva() : -1;
    else if (modo == 17) selectIndex = CapaTexFilaActiva ? CapaTexFilaActiva() : -1;
    else if (modo == 18) selectIndex = CapaMatFilaActiva ? CapaMatFilaActiva() : -1;
    {
        // VENTANA de FilasAlto() renglones (el resto scrollea; si sobran, quedan en blanco)
        int n = ListaCount();
        const int alto = FilasAlto();
        int visibles = n < alto ? n : alto;
        (void)alto;
        altura += AltoFilasPx();
        if (scrollFila > n - visibles) scrollFila = n - visibles;
        if (scrollFila < 0) scrollFila = 0;
    }

    listBox->Resize(width, altura);

    return altura + gapGS + Asa();   // (+ la franja del asa: la linea para agrandar / achicar)
}