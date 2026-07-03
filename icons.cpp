#include "WhiskUI/icons.h"

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

void CrearIconos(int texW, int texH){
    IconRect lista[ICON_TOTAL] = {
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
        {  95,  1, 10, 10 }  // modificador (pestania Modificadores)
    };

    // limpia por si se llama más de una vez
    IconsUV.clear();

    for (size_t i = 0; i < ICON_TOTAL; ++i) {
        IconUV* NewIcon = new IconUV();

        GLfloat u1 = (GLfloat)lista[i].x / texW;
        GLfloat u2 = (GLfloat)(lista[i].x + lista[i].w) / texW;
        GLfloat v1 = (GLfloat)lista[i].y / texH;
        GLfloat v2 = (GLfloat)(lista[i].y + lista[i].h) / texH;

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