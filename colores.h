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
    #include <GL/gl.h>
    #include <cstdint>
#endif

// Paleta de la UI (roles genericos). Dialecto C++03 compartido (RVCT no tiene enum class).
struct ColorID {
    enum Enum {
        background,
        blanco,
        accent,
        accentDark,
        negro,
        gris,
        headerColor,
        negroTransparente,
        grisUI,
        grisLinea, // gris medio (0x494949) para lineas del timeline
        Count
    };
    Enum v;
    ColorID(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

extern GLubyte ListaColoresUbyte[ColorID::Count][4];
extern float ListaColores[ColorID::Count][4];

// Carga la paleta de la UI desde un skin.ini ("nombre r g b a" por linea; ignora
// los nombres que no reconoce). Devuelve false si no pudo abrir el archivo.
bool loadColorsW3d(const char* aPath);

#endif // COLORES_H
