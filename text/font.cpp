#include "font.h"

// ================= UTF-8 helpers =================

uint16_t UTF8_Char(const char* s, size_t& i) {
    unsigned char c = (unsigned char)s[i];

    // ASCII
    if (c < 0x80) {
        i += 1;
        return c;
    }

    // UTF-8 2 bytes
    if ((c & 0xE0) == 0xC0) {
        uint16_t cp =
            ((c & 0x1F) << 6) |
            ((unsigned char)s[i + 1] & 0x3F);
        i += 2;
        return cp;
    }

    // UTF-8 3 bytes
    if ((c & 0xF0) == 0xE0) {
        uint16_t cp =
            ((c & 0x0F) << 12) |
            (((unsigned char)s[i + 1] & 0x3F) << 6) |
            ((unsigned char)s[i + 2] & 0x3F);
        i += 3;
        return cp;
    }

    // Fallback
    i += 1;
    return '?';
}

// ---------------------------------------------------------------------------------------------------------------
//  ACENTOS COMPUESTOS. Ver font.h. Estos numeros son "de que pixel del atlas se recorta" y son lo unico a tocar si
//  el acento se ve corrido o comido: no hace falta que el recorte sea exacto, pero de MAS se ven cosas de al lado.
// ---------------------------------------------------------------------------------------------------------------
static const int ACENTO_ALTO   = 2;    // el acento no pasa de 2px de alto
static const int ACENTO_DER_PX = 2;    // corrido a la derecha: la tinta de la coma no esta centrada en su celda,
                                       // asi que pegado al borde izquierdo el acento caia fuera de la letra
// A que altura de la celda arranca el acento. La MAYUSCULA es mas alta que la minuscula -> su acento va mas arriba,
// o se le monta encima. Son dos numeros porque son dos casos distintos, no uno mal medido.
static const int ACENTO_Y_MINUS = 1;   // sobre una minuscula (a, e, o...)
static const int ACENTO_Y_MAYUS = 0;   // sobre una MAYUSCULA (A, E, O...): pegado arriba de todo

// El '?' invertido se dibuja rotando el '?', pero NO va a la misma altura: el normal arranca en la linea de
// mayusculas y el invertido cuelga hacia abajo. Se lo baja estos px.
static const int INVQ_BAJA_PX  = 2;

// la ',' vive en (19,63) 5x11: su tinta esta ABAJO de la celda
static const int COMA_X = 19, COMA_Y = 63, COMA_W = 5;
static const int COMA_INK_Y = 8;       // primera fila con tinta de la coma (dentro de su celda de 11). La coma
                                       // CUELGA de la linea base: su tinta esta abajo de todo. Con 7 se agarraba
                                       // una fila en blanco y se veia un solo pixel de acento.

// la 'n~' vive en (79,31) 5x11: su tilde esta ARRIBA de la celda
static const int ENIE_X = 79, ENIE_Y = 31, ENIE_W = 5;
static const int ENIE_INK_Y = 0;       // primera fila con tinta de la tilde

Font::Acento Font::Descomponer(uint16_t cp, uint16_t& base){
    switch (cp){
        case 0x00E1: base = 'a'; return AcentoAgudo;  // a con acento
        case 0x00E9: base = 'e'; return AcentoAgudo;  // e
        case 0x00ED: base = 'i'; return AcentoAgudo;  // i
        case 0x00F3: base = 'o'; return AcentoAgudo;  // o
        case 0x00FA: base = 'u'; return AcentoAgudo;  // u
        case 0x00C1: base = 'A'; return AcentoAgudo;
        case 0x00C9: base = 'E'; return AcentoAgudo;
        case 0x00CD: base = 'I'; return AcentoAgudo;
        case 0x00D3: base = 'O'; return AcentoAgudo;
        case 0x00DA: base = 'U'; return AcentoAgudo;
        case 0x00E3: base = 'a'; return AcentoTilde;  // a con tilde (pt)
        case 0x00F5: base = 'o'; return AcentoTilde;  // o con tilde (pt)
        default: break;
    }
    base = cp;
    return AcentoNo;
}

int Font::BajadaDe(uint16_t codepoint) const {
    if (codepoint == 0x00BF) return INVQ_BAJA_PX * escala;
    return 0;
}

const GLfloat* Font::getUVAcentoTri(Acento a) const {
    return (a == AcentoTilde) ? uvTilde : uvAgudo;
}

void Font::SetScale(GLshort scale){
    escala = scale;
    mesh[0] = 0;          mesh[1] = 0;
    mesh[2] = 5 * scale;  mesh[3] = 0;
    mesh[4] = 0;          mesh[5] = 11 * scale;
    mesh[6] = 5 * scale;  mesh[7] = 11 * scale;
    // triangulos float: 0,1,2 + 2,1,3 (orden del strip) — camino unico de la UI en los 4 OS
    meshTri[0]  = (GLfloat)mesh[0]; meshTri[1]  = (GLfloat)mesh[1];
    meshTri[2]  = (GLfloat)mesh[2]; meshTri[3]  = (GLfloat)mesh[3];
    meshTri[4]  = (GLfloat)mesh[4]; meshTri[5]  = (GLfloat)mesh[5];
    meshTri[6]  = (GLfloat)mesh[4]; meshTri[7]  = (GLfloat)mesh[5];
    meshTri[8]  = (GLfloat)mesh[2]; meshTri[9]  = (GLfloat)mesh[3];
    meshTri[10] = (GLfloat)mesh[6]; meshTri[11] = (GLfloat)mesh[7];

    // el ACENTO: mismo ancho que un caracter, ACENTO_ALTO px de alto (y=0 es arriba de la celda). Dos quads: la
    // mayuscula es mas alta y su acento va mas arriba.
    const GLfloat ax0 = (GLfloat)(ACENTO_DER_PX * scale), ax1 = (GLfloat)((ACENTO_DER_PX + 5) * scale);
    for (int caso = 0; caso < 2; caso++){
        GLfloat* m = caso ? meshAcentoMay : meshAcento;
        const int top = caso ? ACENTO_Y_MAYUS : ACENTO_Y_MINUS;
        const GLfloat ay0 = (GLfloat)(top * scale);
        const GLfloat ay1 = (GLfloat)((top + ACENTO_ALTO) * scale);
        m[0]  = ax0; m[1]  = ay0;
        m[2]  = ax1; m[3]  = ay0;
        m[4]  = ax0; m[5]  = ay1;
        m[6]  = ax0; m[7]  = ay1;
        m[8]  = ax1; m[9]  = ay0;
        m[10] = ax1; m[11] = ay1;
    }
}

// =================================================

Font::Font(int texW, int texH, GLuint textureID)
    : FontTexture(textureID),
      texWidth(texW),
      texHeight(texH)
{
    SetScale(1); // arma el mesh default (era inicializador de clase)

    // UV de los ACENTOS: se recortan de glifos que YA estan en el atlas (ver el bloque de arriba).
    // El AGUDO sale de la ',' ESPEJADA en horizontal: la coma cae hacia la izquierda y el agudo sube hacia la
    // derecha, o sea que es la misma tinta al reves. Mismo truco que la '\' dibujada como '/' espejado.
    {
        const GLfloat u1 = (GLfloat)COMA_X / texWidth;
        const GLfloat u2 = (GLfloat)(COMA_X + COMA_W) / texWidth;
        const GLfloat v1 = (GLfloat)(COMA_Y + COMA_INK_Y) / texHeight;
        const GLfloat v2 = (GLfloat)(COMA_Y + COMA_INK_Y + ACENTO_ALTO) / texHeight;
        // La coma se usa SIN espejar: asi cae hacia el lado del acento agudo. (Espejada quedaba al reves; se vio
        // en pantalla, que es la unica forma de saberlo -- de este tamanio no se deduce.)
        const GLfloat a = u1, b = u2;
        uvAgudo[0]=a; uvAgudo[1]=v1;  uvAgudo[2]=b; uvAgudo[3]=v1;  uvAgudo[4]=a; uvAgudo[5]=v2;
        uvAgudo[6]=a; uvAgudo[7]=v2;  uvAgudo[8]=b; uvAgudo[9]=v1;  uvAgudo[10]=b; uvAgudo[11]=v2;
    }
    // la TILDE sale de la 'n~' (su tilde esta arriba de la celda). Sin espejar: es simetrica.
    {
        const GLfloat u1 = (GLfloat)ENIE_X / texWidth;
        const GLfloat u2 = (GLfloat)(ENIE_X + ENIE_W) / texWidth;
        const GLfloat v1 = (GLfloat)(ENIE_Y + ENIE_INK_Y) / texHeight;
        const GLfloat v2 = (GLfloat)(ENIE_Y + ENIE_INK_Y + ACENTO_ALTO) / texHeight;
        uvTilde[0]=u1; uvTilde[1]=v1;  uvTilde[2]=u2; uvTilde[3]=v1;  uvTilde[4]=u1; uvTilde[5]=v2;
        uvTilde[6]=u1; uvTilde[7]=v2;  uvTilde[8]=u2; uvTilde[9]=v1;  uvTilde[10]=u2; uvTilde[11]=v2;
    }

    // Fallback = signo ?
    addGlyph('?', 7, 63, 5, 11);
    fallback = atlas['?'];

    // ===== MAYÚSCULAS =====
    addGlyph('A', 1, 0, 5, 11);
    addGlyph('B', 7, 0, 5, 11);
    addGlyph('C', 13, 0, 5, 11);
    addGlyph('D', 19, 0, 5, 11);
    addGlyph('E', 25, 0, 5, 11);
    addGlyph('F', 31, 0, 5, 11);
    addGlyph('G', 37, 0, 5, 11);
    addGlyph('H', 43, 0, 5, 11);
    addGlyph('I', 49, 0, 5, 11);
    addGlyph('J', 55, 0, 5, 11);
    addGlyph('K', 61, 0, 5, 11);
    addGlyph('L', 67, 0, 5, 11);
    addGlyph('M', 73, 0, 5, 11);
    addGlyph(0x00D1, 79, 0, 5, 11); // Ñ
    addGlyph(0x2714, 85, 0, 5, 11); // ✔ tilde
    addGlyph('N', 1, 10, 5, 11);
    addGlyph('O', 7, 10, 5, 11);
    addGlyph('P', 13, 10, 5, 11);
    addGlyph('Q', 19, 10, 5, 11);
    addGlyph('R', 25, 10, 5, 11);
    addGlyph('S', 31, 10, 5, 11);
    addGlyph('T', 37, 10, 5, 11);
    addGlyph('U', 43, 10, 5, 11);
    addGlyph('V', 49, 10, 5, 11);
    addGlyph('W', 55, 10, 5, 11);
    addGlyph('X', 61, 10, 5, 11);
    addGlyph('Y', 67, 10, 5, 11);
    addGlyph('Z', 73, 10, 5, 11);
    addGlyph('_', 79, 10, 5, 11);

    // ===== MINÚSCULAS =====
    addGlyph('a', 1, 20, 5, 11);
    addGlyph('b', 7, 20, 5, 11);
    addGlyph('c', 13, 20, 5, 11);
    addGlyph('d', 19, 20, 5, 11);
    addGlyph('e', 25, 20, 5, 11);
    addGlyph('f', 31, 20, 5, 11);
    addGlyph('g', 37, 20, 5, 11);
    addGlyph('h', 43, 20, 5, 11);
    addGlyph('i', 49, 20, 5, 11);
    addGlyph('j', 55, 20, 5, 11);
    addGlyph('k', 61, 20, 5, 11);
    addGlyph('l', 67, 20, 5, 11);
    addGlyph('m', 73, 20, 5, 11);
    addGlyph('n', 1, 31, 5, 11);
    addGlyph(0x00F1, 79, 31, 5, 11); // ñ
    addGlyph('o', 7, 31, 5, 11);
    addGlyph('p', 13, 31, 5, 11);
    addGlyph('q', 19, 31, 5, 11);
    addGlyph('r', 25, 31, 5, 11);
    addGlyph('s', 31, 31, 5, 11);
    addGlyph('t', 37, 31, 5, 11);
    addGlyph('u', 43, 31, 5, 11);
    addGlyph('v', 49, 31, 5, 11);
    addGlyph('w', 55, 31, 5, 11);
    addGlyph('x', 61, 31, 5, 11);
    addGlyph('y', 67, 31, 5, 11);
    addGlyph('z', 73, 31, 5, 11);

    // ===== SÍMBOLOS =====
    addGlyph('0', 1, 42, 5, 11);
    addGlyph('1', 7, 42, 5, 11);
    addGlyph('2', 13, 42, 5, 11);
    addGlyph('3', 19, 42, 5, 11);
    addGlyph('4', 25, 42, 5, 11);
    addGlyph('5', 31, 42, 5, 11);
    addGlyph('6', 37, 42, 5, 11);
    addGlyph('7', 43, 42, 5, 11);
    addGlyph('8', 49, 42, 5, 11);
    addGlyph('9', 55, 42, 5, 11);

    addGlyph('+', 61, 42, 5, 11);
    addGlyph('-', 67, 42, 5, 11);
    addGlyph('=', 73, 42, 5, 11);
    addGlyph('(', 1, 52, 5, 11);
    addGlyph(')', 7, 52, 5, 11);
    addGlyph('[', 13, 52, 5, 11);
    addGlyph(']', 19, 52, 5, 11);
    addGlyph('{', 25, 52, 5, 11);
    addGlyph('}', 31, 52, 5, 11);
    addGlyph('<', 37, 52, 5, 11);
    addGlyph('>', 43, 52, 5, 11);
    addGlyph('/', 49, 52, 5, 11);
    addGlyph('*', 55, 52, 5, 11);
    addGlyph(':', 61, 52, 5, 11);
    addGlyph('#', 67, 52, 5, 11);
    addGlyph('%', 73, 52, 5, 11);
    addGlyph('!', 1, 64, 5, 11);
    addGlyph('?', 7, 64, 5, 11);
    addGlyph('.', 13, 63, 5, 11);
    addGlyph(',', 19, 63, 5, 11);
    addGlyph('\'', 25, 64, 5, 11);
    addGlyph('\"', 31, 64, 5, 11);
    addGlyph('@', 37, 64, 5, 11);
    addGlyph('&', 43, 64, 5, 11);
    addGlyph('$', 49, 64, 5, 11);
    addGlyph(0x00A1, 55, 64, 5, 11); // ¡
    addGlyph(0x00B0, 61, 64, 5, 11); // °
    addGlyph('|', 67, 64, 5, 11);
}

// ================= API =================

const GLfloat* Font::getUV(uint16_t codepoint) const {
    std::map<uint16_t, Glyph>::const_iterator it = atlas.find(codepoint);
    if (it == atlas.end())
        return fallback.uvs;
    return it->second.uvs;
}

// Glifos DERIVADOS de otro: se voltea la celda del original en vez de gastar atlas.
//   '\'  = '/' espejado en horizontal
//   '?' invertida = '?' rotada 180 (o sea: espejada en LOS DOS ejes)
// El original y el derivado ocupan la misma celda, asi que alcanza con dar vuelta las UV.
static const GLfloat* W3dVoltear(const GLfloat* src, bool enU, bool enV, GLfloat* dst){
    GLfloat u1 = src[0], u2 = src[0], v1 = src[1], v2 = src[1];
    for (int k = 0; k < 12; k += 2){
        if (src[k]   < u1) u1 = src[k];    if (src[k]   > u2) u2 = src[k];
        if (src[k+1] < v1) v1 = src[k+1];  if (src[k+1] > v2) v2 = src[k+1];
    }
    for (int k = 0; k < 12; k += 2){
        dst[k]   = enU ? (u1 + u2 - src[k])   : src[k];
        dst[k+1] = enV ? (v1 + v2 - src[k+1]) : src[k+1];
    }
    return dst;
}

const GLfloat* Font::getUVTri(uint16_t codepoint) const {
    // '\' (0x5C): el '/' espejado en horizontal (ahorra atlas, no sale "?")
    if (codepoint == 0x5C) {
        std::map<uint16_t, Glyph>::const_iterator slash = atlas.find('/');
        if (slash != atlas.end()) {
            static GLfloat flipped[12];
            return W3dVoltear(slash->second.uvsTri, true, false, flipped);
        }
    }
    // '?' invertida (0xBF): el '?' ROTADO 180 = espejado en los dos ejes. Sin esto salia el fallback, que es
    // justamente un '?', y "?Borrar Cubo?" quedaba con los dos signos para el mismo lado.
    if (codepoint == 0x00BF) {
        std::map<uint16_t, Glyph>::const_iterator q = atlas.find('?');
        if (q != atlas.end()) {
            static GLfloat rot[12];
            return W3dVoltear(q->second.uvsTri, true, true, rot);
        }
    }
    std::map<uint16_t, Glyph>::const_iterator it = atlas.find(codepoint);
    if (it == atlas.end())
        return fallback.uvsTri;
    return it->second.uvsTri;
}

const GLshort* Font::getMesh() const {
    return mesh;
}

// ================= INTERNAL =================

void Font::addGlyph(uint16_t codepoint, int x, int y, int w, int h) {
    Glyph g;

    GLfloat u1 = (GLfloat)x / texWidth;
    GLfloat v1 = (GLfloat)y / texHeight;
    GLfloat u2 = (GLfloat)(x + w) / texWidth;
    GLfloat v2 = (GLfloat)(y + h) / texHeight;

    g.uvs[0] = u1; g.uvs[1] = v1;
    g.uvs[2] = u2; g.uvs[3] = v1;
    g.uvs[4] = u1; g.uvs[5] = v2;
    g.uvs[6] = u2; g.uvs[7] = v2;

    g.uvsTri[0]  = u1; g.uvsTri[1]  = v1;
    g.uvsTri[2]  = u2; g.uvsTri[3]  = v1;
    g.uvsTri[4]  = u1; g.uvsTri[5]  = v2;
    g.uvsTri[6]  = u1; g.uvsTri[7]  = v2;
    g.uvsTri[8]  = u2; g.uvsTri[9]  = v1;
    g.uvsTri[10] = u2; g.uvsTri[11] = v2;

    atlas[codepoint] = g;
}

// ================= GLOBAL =================

Font* WhiskFont = NULL;