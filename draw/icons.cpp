#include "WhiskUI/draw/icons.h"

// Variables globales
GLshort IconMesh[8] = { 0,0, 10,0, 0,10, 10,10 };
GLshort IconLineMesh[8] = { 0,0, (GLshort)(RenglonHeight),0, 0, (GLshort)(RenglonHeight), (GLshort)(RenglonHeight), (GLshort)(RenglonHeight) };

int IconSize = 10;
int IconSizeGS = IconSize;

std::vector<IconUV*> IconsUV;

// ---------------------------------------------------
// Funciones
// ---------------------------------------------------
void SetIconScale(int scale){
    IconSizeGS = IconSize * scale;
    IconMesh[2] = IconMesh[5] = IconMesh[6] = IconMesh[7] = IconSize * scale;
    IconLineMesh[2] = IconLineMesh[5] = IconLineMesh[6] = IconLineMesh[7] = (GLshort)(RenglonHeight * scale);
}

// el nombre de cada icono, EN EL ORDEN del enum (para los pngs individuales del skin)
static const char* KNombres[ICON_TOTAL] = {
    "camera", "light", "mesh", "visible", "hidden", "archive", "line", "arrow",
    "arrowRight", "mirror", "array", "gamepad", "instance", "constraint", "curve",
    "textura", "empty", "lista", "cuadricula", "carpeta", "foto", "object",
    "material", "plane", "circle", "cono", "normalVertex", "normalCustom",
    "normalFace", "cilindro", "selVertex", "selEdge", "selFace", "pivotMedian",
    "pivotIndividual", "pivotActive", "pivotCursor", "notifError", "notifOk",
    "modificador", "armature", "keyframe", "monitor", "snap"
};
const char* IconoNombre(int i) {
    return (i >= 0 && i < (int)ICON_TOTAL) ? KNombres[i] : "";
}

// la tabla LEGACY: donde vive cada icono ADENTRO de font.png (sigue siendo el
// fallback de los iconos sin png individual, y el camino entero sin atlas/)
static const IconRect* TablaLegacy() {
    static IconRect lista[ICON_TOTAL] = {
        {  1, 117, 10, 10 }, // camera
        { 13, 117, 10, 10 }, // light
        { 24, 117, 10, 10 }, // mesh
        { 36, 117, 10, 10 }, // visible
        { 48, 117, 10, 10 }, // hidden
        { 60, 117, 10, 10 }, // archive
        { 72, 117, 10, 10 }, // line
        { 84, 117, 10, 10 }, // arrow
        { 96, 117, 10, 10 }, // arrowRight
        { 1,  93, 10, 10 }, // mirror
        { 25, 93, 10, 10 }, // array
        { 37, 93, 10, 10 },  // gamepad
        { 49, 93, 10, 10 }, // instance
        { 61, 93, 10, 10 }, //constraint
        { 73, 93, 10, 10 }, //curve
        {  1, 105, 10, 10 }, //textura (el arte arranca en 1,105)
        {  1,  81, 10, 10 }, //empty
        { 97, 105, 10, 10 }, // lista
        { 85, 105, 10, 10 }, // cuadricula
        { 85,  81, 10, 10 }, // carpeta (coloreable; modos lista y cuadricula)
        { 61, 105, 10, 10 }, // foto (imagenes: png/jpg/bmp...)
        { 49,  81, 10, 10 }, // object (cubo)
        { 13,  81, 10, 10 }, // material
        { 37,  81, 10, 10 }, // plane
        { 25,  81, 10, 10 }, // circle
        { 73,  81, 10, 10 }, // cono
        { 85,  69, 10, 10 }, // normalVertex
        { 85,  57, 10, 10 }, // normalCustom
        { 97,  57, 10, 10 }, // normalFace
        { 61,  81, 10, 10 }, // cilindro
        { 109, 57, 10, 10 }, // selVertex
        { 109, 45, 10, 10 }, // selEdge
        { 109, 33, 10, 10 }, // selFace
        { 117, 23, 10, 10 }, // pivotMedian
        { 117, 12, 10, 10 }, // pivotIndividual
        { 117,  1, 10, 10 }, // pivotActive
        { 106,  1, 10, 10 }, // pivotCursor
        { 106, 23, 10, 10 }, // notifError (cruz, se pinta rojo)
        { 106, 12, 10, 10 }, // notifOk (check, se pinta verde)
        {  95,  1, 10, 10 }, // modificador (pestania Modificadores)
        {  73, 69, 10, 10 }, // armature (esqueleto) - arte que paso Dante
        {  94, 10, 10, 10 }, // keyframe (rombo) - arte que paso Dante
        {  84, 45, 10, 10 }, // monitor (pantalla): el menu "View" - arte que paso Dante
        {  84, 20, 10, 10 }  // snap (iman) - arte que paso Dante
    };
    return lista;
}

void CrearIconosDesde(const IconRect* lista, int texW, int texH){
    const IconRect* legacy = TablaLegacy();

    // limpia por si se llama más de una vez
    IconsUV.clear();

    for (size_t i = 0; i < ICON_TOTAL; ++i) {
        IconUV* NewIcon = new IconUV();
        // x = -1: el icono no tiene png individual -> su arte de siempre en font.png
        // (que el atlas dinamico deja ENTERO en el (0,0), asi el rect legacy vale igual)
        const IconRect& r = (lista[i].x >= 0) ? lista[i] : legacy[i];

        GLfloat u1 = (GLfloat)r.x / texW;
        GLfloat u2 = (GLfloat)(r.x + r.w) / texW;
        GLfloat v1 = (GLfloat)r.y / texH;
        GLfloat v2 = (GLfloat)(r.y + r.h) / texH;

        NewIcon->uvs[0] = u1; NewIcon->uvs[1] = v1; // top-left
        NewIcon->uvs[2] = u2; NewIcon->uvs[3] = v1; // top-right
        NewIcon->uvs[4] = u1; NewIcon->uvs[5] = v2; // bottom-left
        NewIcon->uvs[6] = u2; NewIcon->uvs[7] = v2; // bottom-right

#ifdef W3D_SYMBIAN
        static const int KOrden[6] = { 0, 1, 2, 2, 1, 3 };
        for (int t = 0; t < 6; t++) {
            NewIcon->uvsTri[t*2 + 0] = NewIcon->uvs[KOrden[t]*2 + 0];
            NewIcon->uvsTri[t*2 + 1] = NewIcon->uvs[KOrden[t]*2 + 1];
        }
#endif

        IconsUV.push_back(NewIcon);
    }
}

void CrearIconos(int texW, int texH){
    CrearIconosDesde(TablaLegacy(), texW, texH);
}