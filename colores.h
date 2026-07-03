#ifndef COLORES_H
#define COLORES_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>     // Para GLubyte, GLfixed si hace falta
    #include <cstdint>
#endif

// ----------------------
//   ENUM de IDs
// ----------------------
// dialecto C++03 compartido (RVCT no tiene enum class)
struct ColorID {
    enum Enum {
    background,
    blanco,
    accent,
    accentDark,
    negro,
    gris,
    naranjaFace,
    headerColor,
    negroTransparente,
    grisUI,
    rojoEje,
    LineaPiso,
    LineaPisoRoja,
    LineaPisoVerde,
    ColorTransformX,
    ColorTransformY,
    ColorTransformZ,
    normalVertex,  // overlay normales: amarillo saturado (promedio por posicion)
    normalCustom,  // overlay normales: magenta (normal guardada por vertice)
    normalFace,    // overlay normales: cian (normal de la cara/triangulo)
    seleccionInactiva // contorno de un mesh SELECCIONADO pero NO activo (verde-rojizo)
    };
    Enum v;
    ColorID(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

// ----------------------
//   Arrays declarados
// ----------------------
extern GLubyte ListaColoresUbyte[21][4];

// carga la paleta desde un skin.ini (portable C++03, lo usan los 4 OS).
// formato: "nombre r g b a" por linea. Devuelve false si no pudo abrir.
bool loadColorsW3d(const char* aPath);
void SincronizarRenderColores(); // UI (ListaColores) -> gRenderColors del core (los 4 OS)

extern float ListaColores[21][4];

#endif // COLORES_H