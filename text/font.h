#ifndef FONT_H
#define FONT_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <map>
#include <string>
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

        // ---- ACENTOS COMPUESTOS ----
        // El atlas NO tiene letras acentuadas: se dibuja la LETRA BASE y encima el acento, recortado de un glifo
        // que ya existe. El agudo sale de la ',' ESPEJADA en horizontal (la coma cae a la izquierda, el agudo sube
        // a la derecha) y la tilde de la '~' de la 'n~'. A 2px de alto no se lee mas que la inclinacion.
        // Es el mismo criterio que la '\', que se dibuja como '/' espejado: reusar atlas en vez de gastarlo.
        enum Acento { AcentoNo = 0, AcentoAgudo = 1, AcentoTilde = 2 };

        // Descompone un codepoint acentuado. Devuelve el acento y deja en 'base' la letra sin acentuar.
        // AcentoNo = no es una letra acentuada (o no la sabemos componer): dibujar normal.
        static Acento Descomponer(uint16_t codepoint, uint16_t& base);

        // Cuanto hay que BAJAR este glifo respecto de la celda (px * escala). Los derivados por rotacion no caen
        // a la misma altura que su original: el '?' invertido cuelga hacia abajo. 0 = va donde va cualquiera.
        int BajadaDe(uint16_t codepoint) const;

        // UV del ACENTO (6 verts = 12 floats), ya recortado a la franja de pixeles que tiene la tinta.
        const GLfloat* getUVAcentoTri(Acento a) const;
        // ...y su malla. La MAYUSCULA es mas alta que la minuscula, asi que su acento va MAS ARRIBA: no es el
        // mismo quad. 'mayus' lo elige.
        const GLfloat* getMeshAcentoTri(bool mayus) const { return mayus ? meshAcentoMay : meshAcento; }

        // ---- EL ATLAS EXTENDIDO (font-ext.png, generado por tools/fuente_ext.py) ----
        // Lo que no entra en font.png: las marcas de los acentos que no se recortan de ahi (grave, circunflejo,
        // dieresis, punto, breve, cedilla, ogonek), ss / i sin punto / l con barra / oe / ae, unos signos y el
        // CIRILICO. Se carga RECIEN la primera vez que un texto lo necesita (en el N95 la RAM vale): en ingles o
        // en espaniol puede no cargarse nunca.
        void SetAtlasExtendido(const std::string& ruta);
        GLuint getTextureExt() const { return extTex; }
        // Un caracter listo para dibujar: su UV (6 verts = 12 floats) y de que atlas sale ('ext' = el extendido).
        // Resuelve las derivadas de font.png ('\\', el '?' invertido), las cirilicas iguales a una latina (que
        // usan la de font.png) y lo que esta en el extendido. false = no lo tiene: devuelve el '?' de font.png.
        bool Glifo(uint16_t cp, const GLfloat*& uv, bool& ext);
        // Letra COMPUESTA (letra base + marca: a con acento, c cedilla, i con dieresis, la breve de la i corta
        // rusa...): false si 'cp' no lo es. 'base' = la letra sin la marca. La marca: si es el agudo o la tilde
        // sale de font.png, con su propia malla (una franja de 2px arriba); si no, es la celda entera del
        // extendido y se dibuja con la malla del glifo ('malla' = NULL).
        bool Compuesta(uint16_t cp, uint16_t& base, const GLfloat*& uvMarca, const GLfloat*& malla, bool& extMarca);
        // Si se puede dibujar (no sale el '?'). Para las pruebas de las traducciones.
        bool Soporta(uint16_t cp);

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

        GLshort escala;       // la ultima que paso SetScale (la usan los offsets en px)
        GLshort mesh[8];      // (default en el constructor: C++03)
        GLfloat meshTri[12];  // version triangulos float (camino unico de la UI, 4 OS)
        GLfloat meshAcento[12];    // la franja donde va el acento de una MINUSCULA
        GLfloat meshAcentoMay[12]; // idem para una MAYUSCULA (mas arriba: la letra es mas alta)
        GLfloat uvAgudo[12];    // la ',' espejada y recortada
        GLfloat uvTilde[12];    // la '~' de la 'n~', recortada

        Glyph fallback; // '?' o vacio
        // el atlas extendido: la ruta, la textura (0 = todavia no), y los UV de cada celda (12 floats por celda)
        std::string extRuta;
        GLuint extTex;
        bool extIntentado;
        GLfloat* extUV;
        bool CargarExt();
        const GLfloat* UVCeldaExt(int celda);
};


// ================= UTF-8 helper =================

uint16_t UTF8_Char(const char* s, size_t& i);
// Cuantas LETRAS (no bytes) tiene un texto UTF-8. El ancho en pantalla es letras * CharacterWidthGS: con .size()
// una letra acentuada o cirilica (2 bytes) contaba doble y los botones y menus salian anchos de mas.
int UTF8_Largo(const std::string& s);

// ================= GLOBAL =================

extern Font* WhiskFont;

#endif
