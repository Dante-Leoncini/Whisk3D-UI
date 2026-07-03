#ifndef FONT_H
#define FONT_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <map>
#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
    // stlport v4 no tiene <cstdint>
    typedef unsigned short uint16_t_w3d;
    #ifndef uint16_t
        typedef unsigned short uint16_t;
    #endif
#else
    #include <cstdint>
    #include <GL/gl.h>
#endif

#include "objects/Textures.h"

class Font {
    public:
        struct Glyph {
            GLfloat uvs[8];
            GLfloat uvsTri[12]; // strip expandido a 2 triangulos (camino unico de la UI, 4 OS)
        };

        Font(int texW, int texH, GLuint textureID);

        // Obtener UV por codepoint
        const GLfloat* getUV(uint16_t codepoint) const;

        // Malla compartida (quad)
        const GLshort* getMesh() const;
        const GLfloat* getUVTri(uint16_t codepoint) const;
        const GLfloat* getMeshTri() const { return meshTri; }

        GLuint getTexture() const { return FontTexture; }

        void SetScale(GLshort scale);

    private:
        void addGlyph(uint16_t codepoint, int x, int y, int w, int h);

        GLuint FontTexture;
        int texWidth;
        int texHeight;

        std::map<uint16_t, Glyph> atlas;

        GLshort mesh[8];      // (default en el constructor: C++03)
        GLfloat meshTri[12];  // version triangulos float (camino unico de la UI, 4 OS)

        Glyph fallback; // '?' o vacio
};


// ================= UTF-8 helper =================

uint16_t UTF8_Char(const char* s, size_t& i);

// ================= GLOBAL =================

extern Font* WhiskFont;

#endif
