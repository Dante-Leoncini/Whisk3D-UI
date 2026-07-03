#ifndef ICONS_H
#define ICONS_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <cstddef>
#include <vector>
#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "UI.h" // para RenglonHeight

// Tamaños iniciales
extern GLshort IconMesh[8];
extern GLshort IconLineMesh[8];
extern int IconSize;
extern int IconSizeGS;

const size_t ICON_TOTAL = 40; // (constexpr es C++11)

// dialecto C++03 compartido
struct IconType {
    enum Enum {
    camera,
    light,
    mesh,
    visible,
    hidden,
    archive,
    line,
    arrow,
    arrowRight,
    mirror,
    array,
    gamepad,
    instance,
    constraint,
    curve,
    textura,
    empty,
    lista,
    cuadricula,
    carpeta,
    foto,
    object,    // cubo (pestania "Objeto" y Add > Cube)
    material,  // pestania de mesh parts
    plane,     // Add > Plane
    circle,    // Add > Circle
    cono,      // Add > Cone
    normalVertex, // overlay normales: vertex normal
    normalCustom, // overlay normales: custom/split normal
    normalFace,   // overlay normales: face normal
    cilindro,     // Add > Cylinder
    selVertex,    // Edit Mode: seleccion de vertices (default)
    selEdge,      // Edit Mode: seleccion de bordes
    selFace,      // Edit Mode: seleccion de caras
    pivotMedian,     // Transform Pivot: punto medio
    pivotIndividual, // Transform Pivot: origenes individuales
    pivotActive,     // Transform Pivot: elemento activo
    pivotCursor,     // Transform Pivot: cursor 3D
    notifError,      // notificacion de ERROR (se pinta rojo)
    notifOk,         // notificacion OK/exito (se pinta verde)
    modificador      // pestania "Modificadores" en Properties
    };
    Enum v;
    IconType(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

struct IconRect {
    int x, y, w, h;
};

class IconUV {
    public:
        GLfloat uvs[8];
#ifdef W3D_SYMBIAN
        GLfloat uvsTri[12]; // strip expandido (driver del N95)
#endif
};

// ===================================================
//  Vector global de íconos UV
// ===================================================
extern std::vector<IconUV*> IconsUV;

// ===================================================
// Funciones
// ===================================================
void SetIconScale(int scale);
void CrearIconos(int texW, int texH);

#endif