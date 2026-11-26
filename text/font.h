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
        enum Accent { AccentNo = 0, AccentSharp = 1, AccentTilde = 2 };

        // Descompone un codepoint acentuado. Devuelve el acento y deja en 'base' la letra sin acentuar.
        // AccentNo = no es una letra acentuada (o no la sabemos componer): dibujar normal.
        static Accent Decompose(uint16_t codepoint, uint16_t& base);

        // Cuanto hay que BAJAR este glifo respecto de la celda (px * escala). Los derivados por rotacion no caen
        // a la misma altura que su original: el '?' invertido cuelga hacia abajo. 0 = va donde va cualquiera.
        int DescentOf(uint16_t codepoint) const;

        // UV del ACENTO (6 verts = 12 floats), ya recortado a la franja de pixeles que tiene la tinta.
        const GLfloat* getUVAccentTri(Accent a) const;
        // ...y su malla. La MAYUSCULA es mas alta que la minuscula, asi que su acento va MAS ARRIBA: no es el
        // mismo quad. 'mayus' lo elige.
        const GLfloat* getMeshAccentTri(bool caps) const { return caps ? meshAccentMay : meshAccent; }

        // ---- EL ATLAS EXTENDIDO (font-ext.png, generado por tools/fuente_ext.py) ----
        // Lo que no entra en font.png: las marcas de los acentos que no se recortan de ahi (grave, circunflejo,
        // dieresis, punto, breve, cedilla, ogonek), ss / i sin punto / l con barra / oe / ae, unos signos y el
        // CIRILICO. Se carga RECIEN la primera vez que un texto lo necesita (en el N95 la RAM vale): en ingles o
        // en espaniol puede no cargarse nunca.
        void SetAtlasExtended(const std::string& path);
        GLuint getTextureExt() const { return extTex; }
        // Un caracter listo para dibujar: su UV (6 verts = 12 floats) y de que atlas sale ('ext' = el extendido).
        // Resuelve las derivadas de font.png ('\\', el '?' invertido), las cirilicas iguales a una latina (que
        // usan la de font.png) y lo que esta en el extendido. false = no lo tiene: devuelve el '?' de font.png.
        bool FindGlyph(uint16_t cp, const GLfloat*& uv, bool& ext);
        // EL CAMINO RAPIDO del texto (casi todo es ASCII): el UV de font.png de un ASCII (cp < 128), o NULL si ese
        // caracter no esta en font.png (sale del extendido o es el '?': FindGlyph). Un ASCII nunca es compuesto ni baja
        // de la celda, asi que con esto alcanza: sin buscar en el mapa ni en las tablas por cada letra de cada cuadro.
        const GLfloat* AsciiUV(uint16_t cp) const { return asciiUV[cp]; }
        // Letra COMPUESTA (letra base + marca: a con acento, c cedilla, i con dieresis, la breve de la i corta
        // rusa...): false si 'cp' no lo es. 'base' = la letra sin la marca. La marca: si es el agudo o la tilde
        // sale de font.png, con su propia malla (una franja de 2px arriba); si no, es la celda entera del
        // extendido y se dibuja con la malla del glifo ('malla' = NULL).
        bool Composite(uint16_t cp, uint16_t& base, const GLfloat*& uvMark, const GLfloat*& tmpMesh, bool& extMark);
        // Si se puede dibujar (no sale el '?'). Para las pruebas de las traducciones.
        bool Supports(uint16_t cp);

        // Obtener UV por codepoint
        const GLfloat* getUV(uint16_t codepoint) const;

        // Malla compartida (quad)
        const GLshort* getMesh() const;
        const GLfloat* getUVTri(uint16_t codepoint) const;
        const GLfloat* getMeshTri() const { return meshTri; }

        GLuint getTexture() const { return FontTexture; }

        void SetScale(GLshort newScale);

    private:
        void addGlyph(uint16_t codepoint, int x, int y, int w, int h);

        GLuint FontTexture;
        int texWidth;
        int texHeight;

        std::map<uint16_t, Glyph> atlas;
        const GLfloat* asciiUV[128];   // AsciiUV: se arma una vez, al terminar de cargar el atlas (el constructor)

        GLshort scale;        // la ultima que paso SetScale (la usan los offsets en px)
        GLshort mesh[8];      // (default en el constructor: C++03)
        GLfloat meshTri[12];  // version triangulos float (camino unico de la UI, 4 OS)
        GLfloat meshAccent[12];    // la franja donde va el acento de una MINUSCULA
        GLfloat meshAccentMay[12]; // idem para una MAYUSCULA (mas arriba: la letra es mas alta)
        GLfloat uvSharp[12];    // la ',' espejada y recortada
        GLfloat uvCheck[12];    // la '~' de la 'n~', recortada

        Glyph fallback; // '?' o vacio
        // el atlas extendido: la ruta, la textura (0 = todavia no), y los UV de cada celda (12 floats por celda)
        std::string extPath;
        GLuint extTex;
        bool extTried;
        GLfloat* extUV;
        bool LoadExt();
        const GLfloat* UVCellExt(int cell);
};


// ================= UTF-8 helper =================

uint16_t UTF8_Char(const char* s, size_t& i);
// Cuantas LETRAS (no bytes) tiene un texto UTF-8. El ancho en pantalla es letras * CharacterWidthGS: con .size()
// una letra acentuada o cirilica (2 bytes) contaba doble y los botones y menus salian anchos de mas.
int UTF8_Length(const std::string& s);

// ================= GLOBAL =================

extern Font* WhiskFont;

#endif
