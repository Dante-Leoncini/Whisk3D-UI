/*
 * ==============================================================================
 *  W3dFont.h — fuente bitmap COMPARTIDA de Whisk3D-modelado
 *
 *  Port C++03 de main/UI/font.{h,cpp} de PC. Usa el MISMO asset
 *  (res/Skins/<skin>/font.png, atlas de 128x128, glifos de 5x11 en grilla).
 *  Este modulo solo sabe de UVs y metricas: NO toca GL (cada plataforma
 *  dibuja los quads con su backend). En la Fase 2, font.cpp de PC pasa a
 *  leer la tabla de aca y se borra la copia.
 * ==============================================================================
 */

#ifndef W3D_FONT_H
#define W3D_FONT_H

// metricas del atlas a escala 1 (PC usa GlobalScale=4; el N95, 1)
enum {
    W3dFont_TexSize = 128,
    W3dFont_GlyphW  = 5,
    W3dFont_GlyphH  = 11,
    W3dFont_Advance = 6,   // 5 de glifo + 1 de espacio
    W3dFont_LineH   = 12
};

struct W3dGlyphUV {
    float u0, v0;  // esquina superior izquierda
    float u1, v1;  // esquina inferior derecha
};

// UVs del glifo para un codepoint (latin1/ascii + especiales del atlas).
// Si no existe, devuelve el fallback '?' (y false).
bool W3dFontGetGlyph(unsigned short aCode, W3dGlyphUV& aOut);

// tamano REAL de la textura donde vive la fuente (el atlas dinamico es mas grande
// que los 128 clasicos; font.png queda en su (0,0) asi que las coords no cambian)
void W3dFontSetTexSize(int w, int h);

// posicion EN PIXELES del glifo dentro de font.png (para el que arma su propio
// atlas desde el archivo y no depende del tamano de la textura del editor)
bool W3dFontGetGlyphPx(unsigned short aCode, int* aX, int* aY);

#endif // W3D_FONT_H
