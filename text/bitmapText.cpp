#include "bitmapText.h"
#include "w3dGraphics.h"
#include <stdio.h>
#include <vector>

// BATCH: se hornea el offset X de cada glifo en las posiciones y se emite UN SOLO DrawTrianglesArray por string, en vez
// de 1 draw-call + Translatef POR CARACTER. Es el multiplicador principal del conteo de draw-calls de toda la UI (cada
// etiqueta/valor pasa de ~8-12 draws a 1). Mismo camino GL que ya usa el N95 (TRIANGLES, float, client-array): no toca
// el backend. Los buffers son static (reusados entre llamadas: sin alloc por frame).
void RenderBitmapEtiqueta(const std::string& text, int maxPixels){
    const int cw = CharacterWidthGS;
    const int maxChars = (cw > 0) ? maxPixels / cw : 0;
    int total = 0;
    for (size_t i = 0; i < text.size(); ) { UTF8_Char(text.c_str(), i); total++; }
    if (maxChars < 4 || total <= maxChars) { RenderBitmapText(text, textAlign::right, maxPixels); return; }
    size_t j = 0;
    for (int k = 0; k < maxChars - 2; k++) UTF8_Char(text.c_str(), j);
    RenderBitmapText(text.substr(0, j) + "..", textAlign::right, maxPixels);
}

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
// ============================================================================
//  CAJA DE TEXTO: el dibujo comun de TODOS los campos editables (renombrar en el outliner, el nombre en Properties,
//  los numeros tipeados, el hex del color, los botones input...). Mismo comportamiento en las 5 plataformas:
//   - TODO SELECCIONADO (al empezar a renombrar): un fondo de color atras del texto -> se entiende que la primera
//     tecla REEMPLAZA lo seleccionado. Las flechas sacan la seleccion y dejan el cursor.
//   - si no: el CURSOR, un palito que parpadea, en la posicion del caret.
// ============================================================================
#include "WhiskUI/widgets/TextField.h"
#include "WhiskUI/theme/colores.h"
extern unsigned int w3dGetTicks();

static const unsigned int kParpadeoMs = 530;   // el palito: medio segundo prendido, medio apagado

// un rectangulo liso (sin textura) en el origen actual, del color actual
static void RectCampo(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    const GLfloat v[12] = { (GLfloat)x, (GLfloat)y, (GLfloat)(x + w), (GLfloat)y, (GLfloat)x, (GLfloat)(y + h),
                            (GLfloat)(x + w), (GLfloat)y, (GLfloat)(x + w), (GLfloat)(y + h), (GLfloat)x, (GLfloat)(y + h) };
    static const GLfloat uv[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    w3dEngine::Disable(w3dEngine::Texture2D);
    w3dEngine::VertexPointer2f(0, v);
    w3dEngine::TexCoordPointer2f(0, uv);
    w3dEngine::DrawTrianglesArray(6);
    w3dEngine::Enable(w3dEngine::Texture2D);
}
// cuantos caracteres UTF-8 hay en los primeros 'bytes' de s
static int CharsUTF8(const std::string& s, size_t bytes) {
    if (bytes > s.size()) bytes = s.size();
    int n = 0;
    for (size_t i = 0; i < bytes; i++) if (((unsigned char)s[i] & 0xC0) != 0x80) n++;
    return n;
}
static bool CursorPrendido() { return ((w3dGetTicks() / kParpadeoMs) & 1u) == 0u; }

void RenderTextField(const TextField& f, bool foco, int align, int maxPixels) {
    const textAlign al((textAlign::Enum)align);
    if (!foco) { RenderBitmapText(f.text, al, maxPixels); return; }
    const int cw = CharacterWidthGS;
    const int maxChars = cw > 0 ? maxPixels / cw : 0;
    if (maxChars <= 0) return;
    const int total = CharsUTF8(f.text, f.text.size());
    const int vis = total > maxChars ? maxChars : total;
    int x0 = 0;
    if (al == textAlign::right)       x0 = -vis * cw;
    else if (al == textAlign::center) x0 = (maxPixels - vis * cw) / 2;
    const int g = GlobalScale > 0 ? GlobalScale : 1;
    if (f.selectAll && total > 0) {
        // el FONDO de la seleccion atras del texto (y el texto en blanco encima, que se lea)
        w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::accentDark)]);
        RectCampo(x0 - g, -g, vis * cw + 2 * g, LetterHeightGS + 2 * g);
        w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]);
        RenderBitmapText(f.text, al, maxPixels);
        return;
    }
    RenderBitmapText(f.text, al, maxPixels);
    if (!CursorPrendido()) return;
    int c = CharsUTF8(f.text, f.caret < 0 ? 0 : (size_t)f.caret);
    if (c > vis) c = vis;
    RectCampo(x0 + c * cw - (c > 0 ? g / 2 : 0), -g, g, LetterHeightGS + 2 * g);   // el palito (del color del texto)
}

// el palito parpadea: con un campo enfocado, cada medio segundo hay que redibujar (el loop lo pregunta por cuadro)
bool TextFieldParpadeoTick() {
    static int fase = -1;
    if (!g_textFieldActivo || g_textFieldActivo->selectAll) { fase = -1; return false; }
    const int ahora = CursorPrendido() ? 1 : 0;
    if (ahora == fase) return false;
    fase = ahora;
    return true;
}
