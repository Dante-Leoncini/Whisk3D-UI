/*
 * ==============================================================================
 *  W3dFont.cpp — tabla de glifos identica a main/UI/font.cpp de PC.
 *  Ver W3dFont.h.
 * ==============================================================================
 */

#include "W3dFont.h"

struct TGlyphEntry {
    unsigned short code;
    unsigned char x;
    unsigned char y;
};

// misma tabla que el constructor de Font en main/UI/font.cpp (todos 5x11)
static const TGlyphEntry KGlyphs[] = {
    // mayusculas
    { 'A', 1, 0 },  { 'B', 7, 0 },  { 'C', 13, 0 }, { 'D', 19, 0 },
    { 'E', 25, 0 }, { 'F', 31, 0 }, { 'G', 37, 0 }, { 'H', 43, 0 },
    { 'I', 49, 0 }, { 'J', 55, 0 }, { 'K', 61, 0 }, { 'L', 67, 0 },
    { 'M', 73, 0 }, { 0x00D1, 79, 0 }, { 0x2714, 85, 0 },
    { 'N', 1, 10 }, { 'O', 7, 10 }, { 'P', 13, 10 }, { 'Q', 19, 10 },
    { 'R', 25, 10 }, { 'S', 31, 10 }, { 'T', 37, 10 }, { 'U', 43, 10 },
    { 'V', 49, 10 }, { 'W', 55, 10 }, { 'X', 61, 10 }, { 'Y', 67, 10 },
    { 'Z', 73, 10 }, { '_', 79, 10 },
    // minusculas
    { 'a', 1, 20 }, { 'b', 7, 20 }, { 'c', 13, 20 }, { 'd', 19, 20 },
    { 'e', 25, 20 }, { 'f', 31, 20 }, { 'g', 37, 20 }, { 'h', 43, 20 },
    { 'i', 49, 20 }, { 'j', 55, 20 }, { 'k', 61, 20 }, { 'l', 67, 20 },
    { 'm', 73, 20 },
    { 'n', 1, 31 }, { 0x00F1, 79, 31 }, { 'o', 7, 31 }, { 'p', 13, 31 },
    { 'q', 19, 31 }, { 'r', 25, 31 }, { 's', 31, 31 }, { 't', 37, 31 },
    { 'u', 43, 31 }, { 'v', 49, 31 }, { 'w', 55, 31 }, { 'x', 61, 31 },
    { 'y', 67, 31 }, { 'z', 73, 31 },
    // numeros y simbolos
    { '0', 1, 42 }, { '1', 7, 42 }, { '2', 13, 42 }, { '3', 19, 42 },
    { '4', 25, 42 }, { '5', 31, 42 }, { '6', 37, 42 }, { '7', 43, 42 },
    { '8', 49, 42 }, { '9', 55, 42 },
    { '+', 61, 42 }, { '-', 67, 42 }, { '=', 73, 42 },
    { '(', 1, 52 }, { ')', 7, 52 }, { '[', 13, 52 }, { ']', 19, 52 },
    { '{', 25, 52 }, { '}', 31, 52 }, { '<', 37, 52 }, { '>', 43, 52 },
    { '/', 49, 52 }, { '*', 55, 52 }, { ':', 61, 52 }, { '#', 67, 52 },
    { '%', 73, 52 },
    { '!', 1, 64 }, { '?', 7, 64 }, { '.', 13, 63 }, { ',', 19, 63 },
    { '\'', 25, 64 }, { '"', 31, 64 }, { '@', 37, 64 }, { '&', 43, 64 },
    { '$', 49, 64 }, { 0x00A1, 55, 64 }, { 0x00B0, 61, 64 }, { '|', 67, 64 }
};

static const int KGlyphCount = (int)(sizeof(KGlyphs) / sizeof(KGlyphs[0]));

// el atlas dinamico puede ser mas grande que los 128 clasicos (font.png en el 0,0)
static int gTexW = W3dFont_TexSize;
static int gTexH = W3dFont_TexSize;
void W3dFontSetTexSize(int w, int h) { gTexW = w; gTexH = h; }

static void MakeUV(int aX, int aY, W3dGlyphUV& aOut) {
    aOut.u0 = (float)aX / gTexW;
    aOut.v0 = (float)aY / gTexH;
    aOut.u1 = (float)(aX + W3dFont_GlyphW) / gTexW;
    aOut.v1 = (float)(aY + W3dFont_GlyphH) / gTexH;
}

bool W3dFontGetGlyphPx(unsigned short aCode, int* aX, int* aY) {
    for (int i = 0; i < KGlyphCount; i++) {
        if (KGlyphs[i].code == aCode) { *aX = KGlyphs[i].x; *aY = KGlyphs[i].y; return true; }
    }
    *aX = 7; *aY = 63;   // fallback: '?'
    return false;
}

bool W3dFontGetGlyph(unsigned short aCode, W3dGlyphUV& aOut) {
    for (int i = 0; i < KGlyphCount; i++) {
        if (KGlyphs[i].code == aCode) {
            MakeUV(KGlyphs[i].x, KGlyphs[i].y, aOut);
            return true;
        }
    }
    // fallback: '?' (7,63 como en PC)
    MakeUV(7, 63, aOut);
    return false;
}
