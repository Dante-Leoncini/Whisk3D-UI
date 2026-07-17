#include "bitmapText.h"
#include "w3dGraphics.h"
#include <stdio.h>
#include <vector>

// BATCH: se hornea el offset X de cada glifo en las posiciones y se emite UN SOLO DrawTrianglesArray por string, en vez
// de 1 draw-call + Translatef POR CARACTER. Es el multiplicador principal del conteo de draw-calls de toda la UI (cada
// etiqueta/valor pasa de ~8-12 draws a 1). Mismo camino GL que ya usa el N95 (TRIANGLES, float, client-array): no toca
// el backend. Los buffers son static (reusados entre llamadas: sin alloc por frame).
void RenderBitmapText(const std::string& text, textAlign align, int maxPixels){
    if (!WhiskFont) return;

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
    if (drawChars <= 0) return;

    // 2) si es right, saltar los que no entran (empezar por el 1er glifo visible)
    size_t start = 0;
    if (align == textAlign::right && totalChars > drawChars) {
        int skip = totalChars - drawChars;
        size_t j = 0;
        while (skip--) {
            UTF8_Char(text.c_str(), j);
        }
        start = j;
    }

    // 3) offset X inicial segun alineacion (antes era un Translatef; ahora se hornea en las posiciones)
    float x0 = 0.0f;
    if (align == textAlign::right)       x0 = (float)(-drawChars * cw);
    else if (align == textAlign::center) x0 = (float)((maxPixels - drawChars * cw) / 2);

    // 4) armar UN buffer de posiciones + UVs para todos los glifos (offset x = drawn*cw horneado)
    static std::vector<GLfloat> vbuf, ubuf; // reusados (sin alloc por frame tras el warmup)
    vbuf.clear(); ubuf.clear();
    const GLfloat* quad = WhiskFont->getMeshTri(); // 6 verts * (x,y) = 12 floats, glifo base en el origen
    i = start;
    int drawn = 0;
    while (i < text.size() && drawn < drawChars) {
        uint16_t cp = UTF8_Char(text.c_str(), i);
        if (cp != 0x20) {
            // ACENTUADA: se dibuja la LETRA BASE y encima el acento, los dos del mismo atlas (ver font.h). Entra en
            // el MISMO buffer -> sigue siendo un solo draw-call para el string entero.
            uint16_t base = cp;
            Font::Acento ac = Font::Descomponer(cp, base);
            const GLfloat* uv = WhiskFont->getUVTri(base); // 6 verts * (u,v) = 12 floats
            float dx = x0 + (float)(drawn * cw);
            const float dy = (float)WhiskFont->BajadaDe(base); // los derivados por rotacion no caen a la misma altura
            for (int v = 0; v < 6; v++) {
                vbuf.push_back(quad[v*2] + dx); vbuf.push_back(quad[v*2+1] + dy);
                ubuf.push_back(uv[v*2]);        ubuf.push_back(uv[v*2+1]);
            }
            if (ac != Font::AcentoNo) {
                // la altura del acento depende de la LETRA que lleva abajo, no del acento
                const bool mayus = (base >= 0x41 && base <= 0x5A);
                const GLfloat* ma = WhiskFont->getMeshAcentoTri(mayus);
                const GLfloat* ua = WhiskFont->getUVAcentoTri(ac);
                for (int v = 0; v < 6; v++) {
                    vbuf.push_back(ma[v*2] + dx); vbuf.push_back(ma[v*2+1]);
                    ubuf.push_back(ua[v*2]);      ubuf.push_back(ua[v*2+1]);
                }
            }
        }
        drawn++;
    }
    if (vbuf.empty()) return; // string todo espacios

    // 5) UN draw-call para el string entero
    w3dEngine::BindTexture(WhiskFont->getTexture());
    w3dEngine::VertexPointer2f(0, &vbuf[0]);
    w3dEngine::TexCoordPointer2f(0, &ubuf[0]);
    w3dEngine::DrawTrianglesArray((int)(vbuf.size() / 2));
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