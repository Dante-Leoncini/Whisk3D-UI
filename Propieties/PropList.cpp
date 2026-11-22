#include "PropList.h"
#include "w3dGraphics.h"
#include "objects/Armature.h"                 // modo 5: clips de animacion del esqueleto
#include "animation/SkeletalAnimation.h"      // SkeletalAnimation (name de cada clip)
#include "animation/VertexAnimation.h"        // modo 7: animaciones del objeto + hooks
#include "animation/Armature2DAnimation.h"    // modo 10: Arm2DSetActive (cambiar de rig 2D recarga el rango)
#include "script/W3dScript.h"                 // modo 12: W3dScriptData (los scripts lua del objeto)
#include "WhiskUI/draw/icons.h"               // modo 13: el ojo de cada capa del mix
#include "WhiskUI/draw/glesdraw.h"            // W3dDrawStrip4 (el icono)
#include "WhiskUI/theme/colors.h"             // modo 14: el gris del checkbox apagado
#include "WhiskUI/widgets/Button.h"           // el campo de renombrar en linea

// modo 13 (arbol del MIX): los instala Properties.cpp
int  (*MixRowsCount)() = 0;
std::string (*MixRowText)(int) = 0;
int  (*MixRowLevel)(int) = 0;
int  (*MixRowEye)(int) = 0;
void (*MixRowSelect)(int) = 0;
int (*RoutineRowsCount)(Object*) = 0;
std::string (*RoutineRowText)(Object*, int) = 0;
void (*RoutineRowSelect)(Object*, int) = 0;
int (*RoutineRowIcon)(Object*, int) = 0;
int (*RoutineRowValue)(Object*, int, float*) = 0;
int g_routineRowDestination = -1;
int (*LayerTexRowsCount)() = 0;
std::string (*LayerTexRowText)(int) = 0;
void (*LayerTexRowSelect)(int) = 0;
int (*LayerTexActiveRow)() = 0;
int (*LayerMatRowsCount)() = 0;
std::string (*LayerMatRowText)(int) = 0;
void (*LayerMatRowSelect)(int) = 0;
int (*LayerMatActiveRow)() = 0;
int  (*MixActiveRow)() = 0;
int  (*MixCollapsedRow)(int) = 0;
int  (*MixRowIcon)(int) = 0;

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
    rowsMax = 3;
    scrollRow = 0;
    mode = 0;    // por defecto: mesh parts (materialsGroup)
    opDelete  = opRename = opUpload = opLower = NULL; opReadOnly = NULL; opChange = NULL;
    minItems  = 0; nExtras = 0;
    rowButton = new Button("", -1, false);
    rowRow = -1; rowDestination = -1;
    heightFixed = false; minRows = 0; showEmpty = false; hidden = false; heightPx = 0; handleState = 0; sbState = 0; hoverRow = -1;
    buttonsSide = false;
    sideBtn[0] = new Button("", (int)IconType::arrow, false); sideBtn[0]->iconFlip = 2;   // subir (la flecha, dada vuelta)
    sideBtn[1] = new Button("", (int)IconType::arrow, false);                             // bajar
    sideBtn[2] = new Button("", (int)IconType::list, false);                              // opciones
    for (int k = 0; k < 3; k++) { sideBtn[k]->rowHeight = true; sideBtn[k]->centered = true; }
    sideBtn[0]->help = "Move Up"; sideBtn[1]->help = "Move Down"; sideBtn[2]->help = "Options";   // (el cartelito)
};
bool PropListMeshParts::SideEnabled(int k) const {
    const int n = ListCount();
    if (n <= 0 || selectIndex < 0 || selectIndex >= n) return false;
    const bool moves = Reordenable();
    if (k == 0) return moves && selectIndex > 0;
    if (k == 1) return moves && selectIndex < n - 1;
    return true;
}
bool PropListMeshParts::ScrollGeo(int* rail, int* grab, int* pos) const {
    const int n = ListCount(), vis = n < RowsHeight() ? n : RowsHeight();
    if (n <= vis || vis <= 0) return false;
    const int track = vis * (RowHeightGS + gapGS) - gapGS;
    int th = track * vis / n; if (th < GlobalScale * 6) th = GlobalScale * 6;
    if (rail) *rail = track; if (grab) *grab = th;
    if (pos) *pos = (track - th) * scrollRow / (n - vis);
    return true;
}
void PropListMeshParts::ScrollFromY(int y) {
    int track = 0, th = 0;
    if (!ScrollGeo(&track, &th, NULL)) return;
    const int n = ListCount(), vis = n < RowsHeight() ? n : RowsHeight();
    const int range = track - th;
    int t = y - th / 2; if (t < 0) t = 0; if (t > range) t = range;
    scrollRow = range > 0 ? (t * (n - vis) + range / 2) / range : 0;
    if (scrollRow < 0) scrollRow = 0; if (scrollRow > n - vis) scrollRow = n - vis;
}
int PropListMeshParts::RowHeightPx() const {
    const int rowH = RowHeightGS + gapGS;
    if (heightFixed && heightPx > 0) { const int mn = (minRows > 0 ? minRows : 1) * rowH - gapGS; return heightPx < mn ? mn : heightPx; }
    return RowsHeight() * rowH - gapGS;
}
int PropListMeshParts::RowsHeight() const {
    if (heightFixed && heightPx > 0) {   // (a mano, en pixeles: los renglones ENTEROS que entran)
        const int rowH = RowHeightGS + gapGS;
        const int mn = (minRows > 0 ? minRows : 1) * rowH - gapGS;
        const int f = ((heightPx < mn ? mn : heightPx) + gapGS) / rowH;
        return f < 1 ? 1 : f;
    }
    int f = rowsMax;
    if (f < minRows) f = minRows;
    if (!heightFixed && minRows == 0) { const int n = ListCount(); if (n < f) f = n; }
    return f < 1 ? 1 : f;
}
int PropListMeshParts::SideWidth() const { return buttonsSide ? RowHeightGS + GlobalScale * 4 + gapGS : 0; }
void PropListMeshParts::AddExtra(const char* text, void (*action)(), int icon, bool onlyEdit) {
    if (nExtras >= 4) return;
    extras[nExtras].text = text; extras[nExtras].action = action; extras[nExtras].icon = icon; extras[nExtras].onlyEdit = onlyEdit;
    nExtras++;
}
bool PropListMeshParts::WithTrash() const {
    return opDelete && ListCount() > minItems && !(opReadOnly && opReadOnly());
}
bool PropListMeshParts::Reordenable() const {
    return opUpload && opLower && ListCount() > 1 && !(opReadOnly && opReadOnly());
}
int PropListMeshParts::Handle() { return GlobalScale * 8; }

// la MISMA lista sirve para parts / uv maps / color layers / anim clips / vertex groups /
// UV groups (segun 'modo'). Los dos grupos de pesos son entidades DISTINTAS (ver Mesh.h):
// modo 4 = vertexGroups (por control-point, rig 3D); modo 9 = uvGroups (por corner, rig 2D).
int (*PostRowsCount)() = NULL;
std::string (*PostRowText)(int) = NULL;
void (*PostRowSelect)(int) = NULL;
int (*CurveModRowsCount)() = NULL;
std::string (*CurveModRowText)(int) = NULL;
void (*CurveModRowSelect)(int) = NULL;
int PropListMeshParts::ListCount() const {
    if (mode == 17) return LayerTexRowsCount ? LayerTexRowsCount() : 0;   // las capas de textura del material
    if (mode == 18) return LayerMatRowsCount ? LayerMatRowsCount() : 0;   // las capas de material del mesh part
    if (mode == 16) return CurveModRowsCount ? CurveModRowsCount() : 0;   // los modificadores de la curva activa
    if (mode == 15) return PostRowsCount ? PostRowsCount() : 0;   // la pila de postprocesado
    if (mode == 13) return MixRowsCount ? MixRowsCount() : 0;   // arbol del MIX
    if (mode == 14) return RoutineRowsCount ? RoutineRowsCount(obj) : 0;   // pasos de una rutina
    if (mode == 5) return arm ? (int)arm->animations.size() : 0; // clips de animacion (armature)
    if (mode == 7) return mesh ? (int)mesh->animations.size() : 0; // animaciones del objeto (vertex anims)
    if (mode == 6) return arm ? (int)arm->bones.size() : 0;      // huesos (Pose Mode)
    if (mode == 8) return mesh ? (int)mesh->Arm2DBones().size() : 0; // huesos 2D del ARMATURE 2D ACTIVO
    if (mode == 10) return mesh ? (int)mesh->armatures2d.size() : 0; // ARMATURES 2D del mesh (una malla puede tener varios)
    if (mode == 11) return obj ? obj->ConstraintsCount() : 0;        // constraints: los tiene CUALQUIER objeto, va por 'obj'
    if (mode == 12) return (obj && obj->scriptData) ? (int)obj->scriptData->scripts.size() : 0; // scripts lua
    if (!mesh) return 0;
    if (mode == 1) return (int)mesh->uvMaps.size();
    if (mode == 2) return (int)mesh->colorLayers.size();
    if (mode == 3) return (int)mesh->modifiers.size();       // stack de modificadores
    if (mode == 4) return (int)mesh->vertexGroups.size();    // grupos de vertices (huesos del rig 3D)
    if (mode == 9) return (int)mesh->uvGroups.size();        // UV groups (pesos por corner: rig 2D)
    return (int)mesh->materialsGroup.size();
}
std::string PropListMeshParts::ListName(int i) const {
    if (mode == 17) return LayerTexRowText ? LayerTexRowText(i) : std::string();
    if (mode == 18) return LayerMatRowText ? LayerMatRowText(i) : std::string();
    if (mode == 16) return CurveModRowText ? CurveModRowText(i) : std::string();
    if (mode == 15) return PostRowText ? PostRowText(i) : std::string();
    if (mode == 13) return MixRowText ? MixRowText(i) : std::string();
    if (mode == 14) return RoutineRowText ? RoutineRowText(obj, i) : std::string();
    if (mode == 5) return (arm && i >= 0 && i < (int)arm->animations.size()) ? arm->animations[i]->name : std::string();
    if (mode == 7) return (mesh && i >= 0 && i < (int)mesh->animations.size() && mesh->animations[i]) ? mesh->animations[i]->name : std::string();
    if (mode == 6) return (arm && i >= 0 && i < (int)arm->bones.size()) ? arm->bones[i].name : std::string();
    if (mode == 8) return (mesh && i >= 0 && i < (int)mesh->Arm2DBones().size()) ? mesh->Arm2DBones()[i].name : std::string();
    if (mode == 10) return (mesh && i >= 0 && i < (int)mesh->armatures2d.size() && mesh->armatures2d[i]) ? mesh->armatures2d[i]->name : std::string();
    if (mode == 11) return obj ? obj->ConstraintName(i) : std::string(); // (el accesor ya clampea el indice)
    if (mode == 12) {   // scripts lua: se muestra el NOMBRE DE ARCHIVO (la ruta no entra ni sirve)
        if (!obj || !obj->scriptData) return std::string();
        if (i < 0 || i >= (int)obj->scriptData->scripts.size()) return std::string();
        const std::string& r = obj->scriptData->scripts[i].path;
        size_t s = r.find_last_of("/\\");
        return (s == std::string::npos) ? r : r.substr(s + 1);
    }
    if (!mesh || i < 0) return std::string();
    if (mode == 1) return (i < (int)mesh->uvMaps.size())      ? mesh->uvMaps[i]->name   : std::string();
    if (mode == 2) return (i < (int)mesh->colorLayers.size()) ? mesh->colorLayers[i]->name : std::string();
    if (mode == 3) return mesh->ModifierName(i);         // sin exponer la clase Modifier (metodo del Mesh)
    if (mode == 4) return (i < (int)mesh->vertexGroups.size())? mesh->vertexGroups[i]->name : std::string();
    if (mode == 9) return (i < (int)mesh->uvGroups.size())    ? mesh->uvGroups[i]->name : std::string();
    return (i < (int)mesh->materialsGroup.size())             ? mesh->materialsGroup[i].name : std::string();
}
void PropListMeshParts::ListSelect(int i) {
    selectIndex = i;
    if (mode == 17) { if (LayerTexRowSelect) LayerTexRowSelect(i); return; }
    if (mode == 18) { if (LayerMatRowSelect) LayerMatRowSelect(i); return; }
    if (mode == 16) { if (CurveModRowSelect) CurveModRowSelect(i); return; }
    if (mode == 15) { if (PostRowSelect) PostRowSelect(i); return; }
    if (mode == 13) { if (MixRowSelect) MixRowSelect(i); return; }
    if (mode == 14) { if (RoutineRowSelect) RoutineRowSelect(obj, i); return; }
    if (mode == 5) { if (arm){ arm->activeAnim = i;                              // clip: activo + sincroniza la seleccion
                               if (OnSelectAnimClip) OnSelectAnimClip(arm, i); } return; } // app-wide (timeline + Start/End)
    if (mode == 7) { if (mesh && OnSelectAnimVertex) OnSelectAnimVertex(mesh, i); return; } // anim del objeto -> timeline
    if (mode == 6) { if (arm){ for (size_t b=0;b<arm->bones.size();b++) arm->bones[b].select = false;  // hueso: activo + select
                               if (i>=0 && i<(int)arm->bones.size()){ arm->bones[i].select = true; arm->activeBone = i; } } return; }
    if (mode == 8) { if (mesh){ // hueso 2D: activo + hueso ENTERO seleccionado (ambas puntas, como el click de cuerpo)
                               for (size_t b=0;b<mesh->Arm2DBones().size();b++){ W3dBone2D& hb = mesh->Arm2DBones()[b];
                                   hb.select = false; hb.selHead = false; hb.selTail = false; }
                               if (i>=0 && i<(int)mesh->Arm2DBones().size()){ W3dBone2D& hb = mesh->Arm2DBones()[i];
                                   hb.select = true; hb.selHead = true; hb.selTail = true; mesh->Arm2DActiveBone() = i; } } return; }
    // armature 2D activo: SIEMPRE por Arm2DSetActive (escribir el indice a mano dejaba el timeline
    // con el Start/End/FPS del clip del rig ANTERIOR -> el Play usaba el rango equivocado y editar
    // el campo End lo escribia en el clip del rig nuevo)
    if (mode == 10) { Arm2DSetActive(mesh, i); return; }
    // constraints: SIEMPRE por SetActiveConstraint (clampea; -1 = ninguno). Elegir uno no
    // recalcula nada -- el stack se evalua al vuelo al dibujar, no hornea geometria.
    if (mode == 11) { if (obj) obj->SetActiveConstraint(i); return; }
    // scripts lua: elegir uno solo cambia CUAL se muestra abajo (no re-ejecuta nada;
    // el orden de la lista sigue siendo el orden de ejecucion)
    if (mode == 12) { if (obj && obj->scriptData) obj->scriptData->active = i; return; }
    if (!mesh) return;
    if (mode == 1)      { mesh->uvActiveMap = i; mesh->ApplyLayersToRender(); } // re-hornea la UV activa
    else if (mode == 2) { mesh->activeColor = i; mesh->ApplyLayersToRender(); } // re-hornea el color activo
    else if (mode == 3) { mesh->activeModifier = i; }                           // modificadores: solo cambia el activo
    else if (mode == 4) { mesh->activeGroup = i; }                              // grupos de vertices: solo el activo
    else if (mode == 9) { mesh->uvActiveGroup = i; }                            // UV groups: solo el activo
    else RebindMaterialMeshPart();                                              // mesh parts: re-bind material
}

// la ventana sigue a la seleccion (y nunca pasa del final)
void PropListMeshParts::FitWindow(){
    if (ListCount() == 0) { scrollRow = 0; return; }
    int n = ListCount();
    const int height = RowsHeight();
    int visible = n < height ? n : height;
    if (scrollRow > n - visible) scrollRow = n - visible;
    if (scrollRow < 0) scrollRow = 0;
    if (selectIndex < scrollRow) scrollRow = selectIndex;
    if (selectIndex >= scrollRow + visible) scrollRow = selectIndex - visible + 1;
};

void PropListMeshParts::RenderPropertiBox(Card* propertiBox){
    if (hidden || (ListCount() == 0 && !showEmpty)) return; // lista vacia: no ocupa fila ni se dibuja (ver Resize)
    w3dEngine::Translatef((GLfloat)-PropColLabel, 0, 0);
    listBox->Render(false);
    // EL ASA: abajo de la caja, DOS RAYAS finas (la agarradera), un poco mas cortas que la caja, del gris de las lineas de
    // los menus; con el puntero encima, blancas; agarradas, verdes. Arrastrarla (mouse o dedo) agranda o achica la lista
    {
        const GLfloat u = CardUV[5 * 2], v = CardUV[5 * 2 + 1];   // (un texel liso de la tarjeta: sin esquinas)
        const int gs = GlobalScale > 0 ? GlobalScale : 1;
        const int x0 = gs * 3, x1 = listBox->width - gs * 3;
        const int y0 = listBox->height + (Handle() - gs * 3) / 2;
        if (handleState == 2) SetColorID(ColorID::accent, 1.0f);
        else if (handleState == 1) SetColorID(ColorID::white, 1.0f);
        else SetColorID(ColorID::grayLine, 1.0f);
        for (int k = 0; k < 2; k++) {
            const int yy = y0 + k * gs * 2;
            GLshort q[8] = { (GLshort)x0, (GLshort)yy, (GLshort)x1, (GLshort)yy,
                             (GLshort)x0, (GLshort)(yy + gs), (GLshort)x1, (GLshort)(yy + gs) };
            GLfloat uv[8] = { u, v, u, v, u, v, u, v };
            W3dDrawStrip4(q, uv);
        }
        SetColorID(ColorID::background, 1.0f);
    }
    w3dEngine::Translatef((GLfloat)PropColLabel, listBox->height + gapGS + Handle(), 0);
}

void PropListMeshParts::RenderPropertiBoxBorder(Card* propertiBox){
    if (hidden || (ListCount() == 0 && !showEmpty)) return;
    w3dEngine::Translatef((GLfloat)-PropColLabel, -listBox->height - gapGS - Handle(), 0);
    listBox->RenderBorder(false);
    w3dEngine::Translatef((GLfloat)PropColLabel, listBox->height + gapGS + Handle(), 0);
};

void PropListMeshParts::RenderPropertiValue(Card* propertiBox){
    if (hidden || (ListCount() == 0 && !showEmpty)) return;
    w3dEngine::Translatef(0, listBox->height + gapGS + Handle(), 0);
}

void PropListMeshParts::RenderPropertiLabel(Card* propertiBox){
    if (hidden || (ListCount() == 0 && !showEmpty)) return; // lista vacia: sin label/scroll
    w3dEngine::PushMatrix();   // (al final se avanza EXACTO lo que mide la caja: ver abajo)
    {
        int n = ListCount();
        const int height = RowsHeight();
        int visible = n < height ? n : height;
        // LOS TRES BOTONES de la derecha (subir, bajar, opciones), a lo alto de la caja
        if (buttonsSide) {
            const int colW = SideWidth() - gapGS;
            int free = listBox->height;
            // APAGADO (no hay a donde mover): el fondo y el borde, oscuros como el fondo del panel; el icono en un gris que
            // se llega a ver. Apagado no se resalta con el puntero
            static const float kOffIcon[4] = { 0.36f, 0.36f, 0.36f, 1.0f };
            const float* background = ColorList[static_cast<int>(ColorID::background)];
            for (int k = 0; k < 3; k++) {
                const bool on = SideEnabled(k);
                sideBtn[k]->tint = on ? NULL : background;
                sideBtn[k]->borderColor = on ? NULL : background;
                sideBtn[k]->textColor   = on ? NULL : kOffIcon;
                if (!on) sideBtn[k]->hover = false;
            }
            for (int k = 0; k < 3; k++) { sideBtn[k]->Resize(colW); free -= sideBtn[k]->height; }
            (void)free;
            const int sep = gapGS;          // (un gap fijo: agrandar la lista no los separa)
            int yb = -GlobalScale;          // (arriba de todo, a la altura del borde de la caja)
            for (int k = 0; k < 3; k++) {
                w3dEngine::PushMatrix();
                // (pegados al borde derecho de la tarjeta, como los demas botones: el origen de este pase esta un gap a
                //  la izquierda de la caja)
                w3dEngine::Translatef((GLfloat)(-PropColLabel + gapGS + width + SideWidth() - sideBtn[k]->width), (GLfloat)yb, 0);
                sideBtn[k]->Render();
                w3dEngine::PopMatrix();
                yb += sideBtn[k]->height + sep;
            }
            SetColorID(ColorID::grayUI, 1.0f);
        }
        // (las filas NO llevan tacho ni flecha: estaban justo donde se agarra la barrita de scroll y se terminaban borrando
        //  partes. Borrar y cambiar estan en el menu de la fila -click derecho, el boton de opciones- y en los botones)
        const bool trash = false, arrow = false;
        const int wTrash = 0;
        w3dEngine::Translatef((GLfloat)(-PropColLabel + gapGS + gapGS), borderGS, 0);

        // mini barra de scroll cuando hay mas filas que la ventana
        // (como las barras de los paneles: con el puntero encima, mas ancha y blanca; agarrada, verde)
        { int track = 0, th = 0, ty = 0;
          if (ScrollGeo(&track, &th, &ty)) {
            static Card* thumb = NULL;
            if (!thumb) thumb = new Card(NULL, 4, 10);
            const int tw = GlobalScale * (sbState ? 5 : 3);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - gapGS * 3 - (tw - GlobalScale * 3)), (GLfloat)ty, 0);
            thumb->Resize(tw, th);
            if (sbState == 2) SetColorID(ColorID::accent, 1.0f);
            else if (sbState == 1) SetColorID(ColorID::white, 1.0f);
            else SetColorID(ColorID::grayUI, 0.6f);
            thumb->RenderObject(false);
            w3dEngine::PopMatrix();
          } }

        SetColorID(ColorID::grayUI, 1.0f);

        // solo la VENTANA visible (culling de filas enteras)
        for (int i = scrollRow; i < scrollRow + visible && i < n; ++i){
            if (selectIndex == i){
                SetColorID(ColorID::accent, 1.0f);
            } else if (hoverRow == i) {
                SetColorID(ColorID::white, 1.0f);   // (el puntero encima: en blanco)
            }
            if (mode == 13) {
                // ARBOL (como el outliner): flecha de plegar + icono en los objetos; en las capas, la LINEA
                // del costado (baja desde la flecha del padre) + icono; el OJO de la capa a la derecha
                int level = MixRowLevel ? MixRowLevel(i) : 0;
                int fold  = MixCollapsedRow ? MixCollapsedRow(i) : -1;
                int ico   = MixRowIcon ? MixRowIcon(i) : (int)IconType::armature;
                w3dEngine::PushMatrix();
                if (level > 0) {
                    // la linea: tapa tambien el hueco entre filas (gapGS) para que se vea continua
                    SetColorID(ColorID::grayUI, 1.0f);
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef(0, (GLfloat)-gapGS, 0);
                    W3dDrawStrip4(IconLineMesh, IconsUV[static_cast<size_t>(IconType::line)]->uvs);
                    w3dEngine::PopMatrix();
                    W3dDrawStrip4(IconLineMesh, IconsUV[static_cast<size_t>(IconType::line)]->uvs);
                    w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                } else if (fold >= 0) {
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(fold ? IconType::arrowRight : IconType::arrow)]->uvs);
                    w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                }
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(ico)]->uvs);
                w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                int used = (int)(IconSizeGS + gapGS) * 2;
                RenderBitmapText(ListName(i), textAlign::left, width - bordersGS - used - (int)IconSizeGS - gapGS * 4);
                w3dEngine::PopMatrix();
                int eye = MixRowEye ? MixRowEye(i) : -1;
                if (eye >= 0) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(width - (int)IconSizeGS - gapGS * 4), 0, 0);
                    SetColorID(ColorID::grayUI, eye ? 1.0f : 0.45f);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(eye ? IconType::visible : IconType::hidden)]->uvs);
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
                }
            } else if (mode == 14 && RoutineRowIcon && RoutineRowIcon(obj, i) >= 0) {
                // los PASOS de una rutina: el icono de su grupo (Transform = camara, Fog = niebla...), el texto y A LA
                // DERECHA lo que hace de un vistazo: el checkbox de un on/off o un cuadro con su color
                const int side = CheckboxSide();
                float rgba[4] = { 1, 1, 1, 1 };
                const int value = RoutineRowValue ? RoutineRowValue(obj, i, rgba) : 0;
                w3dEngine::PushMatrix();
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(RoutineRowIcon(obj, i))]->uvs);
                w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
                RenderBitmapText(ListName(i), textAlign::left,
                                 width - bordersGS - (int)IconSizeGS - gapGS - (value ? side + gapGS * 5 : 0));
                w3dEngine::PopMatrix();
                if (value) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(width - side - gapGS * 4), 0, 0);
                    if (value == 3) DrawSampleColor(side, rgba);
                    else if (value == 2) DrawCheckbox(side, true);
                    else {   // apagado: la caja en un gris tenue (la del panel es del color del fondo: aca no se veia)
                        const float* g = ColorList[static_cast<int>(ColorID::grayUI)];
                        const float* f = ColorList[static_cast<int>(ColorID::background)];
                        float c[4] = { f[0] + (g[0] - f[0]) * 0.3f, f[1] + (g[1] - f[1]) * 0.3f, f[2] + (g[2] - f[2]) * 0.3f, 1.0f };
                        DrawSampleColor(side, c);
                    }
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
                }
                // arrastrando un paso: la linea donde cae (arriba de esta fila)
                if (g_routineRowDestination == i || (g_routineRowDestination == n && i == n - 1)) {
                    static Card* line = NULL;
                    if (!line) line = new Card(NULL, width, GlobalScale * 2);
                    line->Resize(width - bordersGS, GlobalScale * 2);
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef(0, (GLfloat)(g_routineRowDestination == i ? -gapGS : RowHeightGS), 0);
                    SetColorID(ColorID::accent, 1.0f);
                    line->RenderObject(false);
                    w3dEngine::PopMatrix();
                    SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
                }
            } else if (mode == 12 && obj && obj->scriptData && i < (int)obj->scriptData->scripts.size()) {
                // los SCRIPTS: el nombre del archivo y A LA DERECHA el OJO (prendido / apagado: el motor no llama a
                // ninguna de sus funciones). Apagado, el nombre y el ojo van tenues. El click en el ojo lo maneja
                // Properties::ClickIn y su cartelito de ayuda PropsHelpIn (Properties.cpp)
                const bool isOn = !obj->scriptData->scripts[i].off;
                const ColorID colText = selectIndex == i ? ColorID::accent : ColorID::grayUI;
                SetColorID(colText, isOn ? 1.0f : 0.45f);
                RenderBitmapText(ListName(i), textAlign::left, width - bordersGS - (int)IconSizeGS - gapGS * 4);
                w3dEngine::PushMatrix();
                w3dEngine::Translatef((GLfloat)(width - (int)IconSizeGS - gapGS * 4), 0, 0);
                SetColorID(ColorID::grayUI, isOn ? 1.0f : 0.45f);
                W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(isOn ? IconType::visible : IconType::hidden)]->uvs);
                w3dEngine::PopMatrix();
                SetColorID(colText, 1.0f);
            } else if (rowRow == i && rowButton && rowButton->editField) {
                // RENOMBRANDO esta fila: el campo de texto en su lugar (el mismo input de los botones "Rename")
                w3dEngine::PushMatrix();
                w3dEngine::Translatef(0, (GLfloat)(-(GlobalScale)), 0);
                rowButton->rowHeight = true;
                rowButton->Resize(width - bordersGS - gapGS * 3);
                rowButton->Render();
                w3dEngine::PopMatrix();
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
            } else
            RenderBitmapText(ListName(i), textAlign::left, width -bordersGS - wTrash);
            // EL TACHO de la fila (borrarla), a la derecha; y la linea de donde cae una fila que se arrastra
            if ((trash || arrow) && !(rowRow == i && rowButton && rowButton->editField)) {
                const int xT = width - (int)IconSizeGS - gapGS * 4 - (n > visible ? GlobalScale * 4 : 0);
                SetColorID(ColorID::grayUI, 0.7f);
                if (trash) {
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)xT, 0, 0);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::trash)]->uvs);
                    w3dEngine::PopMatrix();
                }
                if (arrow) {   // (la flecha hacia abajo: cambiar lo de la fila; a la izquierda del tacho)
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)(trash ? xT - (int)IconSizeGS - gapGS * 2 : xT), 0, 0);
                    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::arrow)]->uvs);
                    w3dEngine::PopMatrix();
                }
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
            }
            if (mode != 14 && rowDestination >= 0 && (rowDestination == i || (rowDestination == n && i == n - 1))) {
                static Card* lineD = NULL;
                if (!lineD) lineD = new Card(NULL, width, GlobalScale * 2);
                lineD->Resize(width - bordersGS, GlobalScale * 2);
                w3dEngine::PushMatrix();
                w3dEngine::Translatef(0, (GLfloat)(rowDestination == i ? -gapGS : RowHeightGS), 0);
                SetColorID(ColorID::accent, 1.0f);
                lineD->RenderObject(false);
                w3dEngine::PopMatrix();
                SetColorID(selectIndex == i ? ColorID::accent : ColorID::grayUI, 1.0f);
            }

            if (selectIndex == i || hoverRow == i){
                SetColorID(ColorID::grayUI, 1.0f);
            }
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
        }
    }
    // (antes se avanzaba lo que ocupaban las filas CON contenido: con renglones en blanco -la lista agrandada, las capas
    //  vacias- los textos de todo lo que venia abajo quedaban corridos para arriba, encima de la caja)
    w3dEngine::PopMatrix();
    w3dEngine::Translatef(0, (GLfloat)(listBox->height + gapGS + Handle()), 0);
}

void PropListMeshParts::button_left(){};
void PropListMeshParts::button_right(){};

bool PropList::Cancel(){
    selectIndex = originalIndex;
    editing = false;
    return editing;
};

bool PropList::EditPropertie(){
    editing = !editing;
    if (editing){
        originalIndex = selectIndex;
    }
    return editing;
};

void PropListMeshParts::button_up(){
    int n = ListCount(); if (n == 0) return;
    int i = selectIndex - 1; if (i < 0) i = n - 1;
    ListSelect(i); FitWindow(); // setea el activo (segun modo) + sigue a la seleccion
};

void PropListMeshParts::button_down(){
    int n = ListCount(); if (n == 0) return;
    int i = selectIndex + 1; if (i >= n) i = 0;
    ListSelect(i); FitWindow();
};

int PropListMeshParts::Resize(int w){
    width = w - bordersGS - SideWidth();
    // LISTA VACIA (ej: modificadores sin ninguno): NO ocupa fila (0) y no se dibuja -> queda solo el boton "Add",
    // sin el recuadro aplastado y vacio que parecia un error. Igual que un PropText 'oculto'.
    if (hidden || (ListCount() == 0 && !showEmpty)){ listBox->Resize(w - bordersGS, 0); return 0; }
    int newHeight = bordersGS;
    // los modos uvmaps/colors/anim siguen el ACTIVO de su fuente (selectIndex refleja uvActiveMap/activeColor/etc)
    if (mesh && mode == 1) selectIndex = mesh->uvActiveMap;
    else if (mesh && mode == 2) selectIndex = mesh->activeColor;
    else if (mesh && mode == 4) selectIndex = mesh->activeGroup;
    else if (mesh && mode == 9) selectIndex = mesh->uvActiveGroup;
    else if (arm && mode == 5) selectIndex = arm->activeAnim;
    else if (mesh && mode == 7 && ActiveAnimVertexOf) selectIndex = ActiveAnimVertexOf(mesh);
    else if (arm && mode == 6) selectIndex = arm->activeBone;
    else if (mesh && mode == 8) selectIndex = mesh->Arm2DActiveBone();
    else if (mesh && mode == 10) selectIndex = mesh->armature2dActive;
    // constraints: la lista sigue al ACTIVO del objeto. Va aca (y no solo en el helper del
    // panel, como los modificadores) porque el stack es POR OBJETO: cambiar de objeto
    // seleccionado tiene que mover el resaltado sin que haga falta tocar ningun boton.
    else if (obj && mode == 11) selectIndex = obj->activeConstraint;
    else if (mode == 13) selectIndex = MixActiveRow ? MixActiveRow() : -1;
    else if (mode == 17) selectIndex = LayerTexActiveRow ? LayerTexActiveRow() : -1;
    else if (mode == 18) selectIndex = LayerMatActiveRow ? LayerMatActiveRow() : -1;
    {
        // VENTANA de RowsHeight() renglones (el resto scrollea; si sobran, quedan en blanco)
        int n = ListCount();
        const int height = RowsHeight();
        int visible = n < height ? n : height;
        (void)height;
        newHeight += RowHeightPx();
        if (scrollRow > n - visible) scrollRow = n - visible;
        if (scrollRow < 0) scrollRow = 0;
    }

    listBox->Resize(width, newHeight);

    return newHeight + gapGS + Handle();   // (+ la franja del asa: la linea para agrandar / achicar)
}