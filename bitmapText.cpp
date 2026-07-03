#include "bitmapText.h"
#include "w3dGraphics.h"
#include <stdio.h>

void RenderBitmapText(const std::string& text, textAlign align, int maxPixels){
    if (!WhiskFont) return;
    // triangulos float por la abstraccion (la fase 2D del N95 no dibuja STRIP ni SHORT)
    w3dEngine::BindTexture(WhiskFont->getTexture());
    w3dEngine::VertexPointer2f(0, WhiskFont->getMeshTri());

    const int cw = CharacterWidthGS;
    int maxChars = maxPixels / cw;
    if (maxChars <= 0) return;

    // 1) contar caracteres UTF-8
    size_t i = 0;
    int totalChars = 0;
    while (i < text.size()) {
        UTF8_Char(text.c_str(), i);
        totalChars++;
    }

    int drawChars = totalChars > maxChars ? maxChars : totalChars;

    // 2) si es right, saltar los que no entran
    size_t start = 0;
    if (align == textAlign::right && totalChars > drawChars) {
        int skip = totalChars - drawChars;
        size_t j = 0;
        while (skip--) {
            UTF8_Char(text.c_str(), j);
        }
        start = j;
    }

    w3dEngine::PushMatrix();

    // 3) mover cursor inicial SOLO UNA VEZ
    if (align == textAlign::right) {
        w3dEngine::Translatef(-drawChars * cw, 0, 0);
    } else if (align == textAlign::center) {
        w3dEngine::Translatef((GLfloat)((maxPixels - drawChars * cw) / 2), 0, 0);
    }

    // 4) dibujar NORMAL, siempre hacia adelante
    i = start;
    int drawn = 0;
    while (i < text.size() && drawn < drawChars) {
        uint16_t cp = UTF8_Char(text.c_str(), i);

        if (cp != 0x20) {
            w3dEngine::TexCoordPointer2f(0, WhiskFont->getUVTri(cp));
            w3dEngine::DrawTrianglesArray(6);
        }

        w3dEngine::Translatef(cw, 0, 0);
        drawn++;
    }

    w3dEngine::PopMatrix();
}

void RenderBitmapFloat(float value, textAlign align, int maxPixels, const std::string& unit){
    // formateo SIN %f: el printf de punto flotante no anda en estlib
    // (Symbian) y ostringstream tampoco; enteros funcionan en los 4 OS
    char buf[32];
    int neg = (value < 0.0f) ? 1 : 0;
    float av = neg ? -value : value;
    int ent = (int)av;
    int frac = (int)((av - (float)ent) * 10000.0f + 0.5f);
    if (frac >= 10000) { ent++; frac -= 10000; }
    sprintf(buf, "%s%d.%04d", neg ? "-" : "", ent, frac);
    RenderBitmapText(std::string(buf) + unit, align, maxPixels);
}