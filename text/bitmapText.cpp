#include "bitmapText.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"   // W3dPantallaAlto: el campo enfocado, en la pantalla (el mouse)
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

    // 4) armar los buffers de posiciones + UVs (offset x = drawn*cw horneado). DOS tandas: lo de font.png (casi todo)
    //    y lo del atlas EXTENDIDO (marcas de acentos que no estan en font.png, el cirilico...: ver font.h). Si el
    //    string no tiene nada del extendido, es un solo draw-call, como siempre.
    static std::vector<GLfloat> vbuf, ubuf, vext, uext; // reusados (sin alloc por frame tras el warmup)
    vbuf.clear(); ubuf.clear(); vext.clear(); uext.clear();
    const GLfloat* quad = WhiskFont->getMeshTri(); // 6 verts * (x,y) = 12 floats, glifo base en el origen
    i = start;
    int drawn = 0;
    while (i < text.size() && drawn < drawChars) {
        uint16_t cp = UTF8_Char(text.c_str(), i);
        if (cp != 0x20) {
            // COMPUESTA (letra con acento, cedilla, dieresis...): se dibuja la LETRA BASE y la marca encima (o abajo)
            uint16_t base = cp;
            const GLfloat* um = 0; const GLfloat* mm = 0; bool extMarca = false;
            const bool compuesta = WhiskFont->Compuesta(cp, base, um, mm, extMarca);
            // (si la marca no se pudo cargar, Compuesta deja la letra base: mejor sin acento que un '?')
            const GLfloat* uv = 0; bool ext = false;
            WhiskFont->Glifo(base, uv, ext);               // 6 verts * (u,v) = 12 floats (el '?' si no lo tiene)
            float dx = x0 + (float)(drawn * cw);
            const float dy = (float)WhiskFont->BajadaDe(base); // los derivados por rotacion no caen a la misma altura
            std::vector<GLfloat>& vb = ext ? vext : vbuf;
            std::vector<GLfloat>& ub = ext ? uext : ubuf;
            for (int v = 0; v < 6; v++) {
                vb.push_back(quad[v*2] + dx); vb.push_back(quad[v*2+1] + dy);
                ub.push_back(uv[v*2]);        ub.push_back(uv[v*2+1]);
            }
            if (compuesta && um) {
                // agudo / tilde: su franja de 2px (la altura depende de la letra de abajo, ya elegida);
                // las del extendido: la celda entera, con la malla del glifo (la marca ya esta en su fila)
                const GLfloat* ma = mm ? mm : quad;
                std::vector<GLfloat>& vm = extMarca ? vext : vbuf;
                std::vector<GLfloat>& uvm = extMarca ? uext : ubuf;
                for (int v = 0; v < 6; v++) {
                    vm.push_back(ma[v*2] + dx); vm.push_back(ma[v*2+1]);
                    uvm.push_back(um[v*2]);     uvm.push_back(um[v*2+1]);
                }
            }
        }
        drawn++;
    }
    if (vbuf.empty() && vext.empty()) return; // string todo espacios

    // 5) UN draw-call por atlas. Despues del extendido se vuelve a dejar font.png: el que dibuja a continuacion
    //    (iconos del mismo atlas) puede contar con que sigue puesto.
    if (!vbuf.empty()) {
        w3dEngine::BindTexture(WhiskFont->getTexture());
        w3dEngine::VertexPointer2f(0, &vbuf[0]);
        w3dEngine::TexCoordPointer2f(0, &ubuf[0]);
        w3dEngine::DrawTrianglesArray((int)(vbuf.size() / 2));
    }
    if (!vext.empty() && WhiskFont->getTextureExt()) {
        w3dEngine::BindTexture(WhiskFont->getTextureExt());
        w3dEngine::TexFilter(false);   // pixel perfect (cacheado por textura: si ya esta, no manda nada)
        w3dEngine::VertexPointer2f(0, &vext[0]);
        w3dEngine::TexCoordPointer2f(0, &uext[0]);
        w3dEngine::DrawTrianglesArray((int)(vext.size() / 2));
    }
    w3dEngine::BindTexture(WhiskFont->getTexture());
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

// DONDE SE DIBUJO EL CAMPO ENFOCADO, en la pantalla (pixeles de la ventana, Y para abajo, como el mouse): el texto
// empieza en x0 y cada letra mide cw; la caja que se puede tocar va de (rx0, ry0) a (rx1, ry1); vis = las letras que se ven
struct GeomCampo { const TextField* f; float x0, cw, rx0, ry0, rx1, ry1; int vis; };
static GeomCampo gGeomCampo = { NULL, 0, 1, 0, 0, 0, 0, 0 };
static bool gArrastrandoCampo = false;
// un punto del origen actual (la modelview y la proyeccion de la UI, el viewport de su panel) -> la pantalla
static void APantalla(const float* M, const int* vp, float lx, float ly, float& sx, float& sy) {
    const float cx = M[0] * lx + M[4] * ly + M[12], cy = M[1] * lx + M[5] * ly + M[13], cw = M[3] * lx + M[7] * ly + M[15];
    const float w = (cw != 0.0f) ? cw : 1.0f;
    sx = (float)vp[0] + (cx / w + 1.0f) * 0.5f * (float)vp[2];
    sy = (float)W3dPantallaAlto - ((float)vp[1] + (cy / w + 1.0f) * 0.5f * (float)vp[3]);
}
static void AnotarGeometria(const TextField& f, int x0, int vis, int cw, int align, int maxPixels) {
    float mv[16], pr[16], M[16];
    w3dEngine::GetMatrix(w3dEngine::ModelView, mv); w3dEngine::GetMatrix(w3dEngine::Projection, pr);
    for (int c = 0; c < 4; c++) for (int r = 0; r < 4; r++) {
        float v = 0.0f; for (int k = 0; k < 4; k++) v += pr[k * 4 + r] * mv[c * 4 + k];
        M[c * 4 + r] = v;
    }
    int vp[4] = { 0, 0, 1, 1 }; w3dEngine::GetViewport(vp);
    const int lh = LetterHeightGS > 0 ? LetterHeightGS : 8;
    // la caja: el ancho del campo y media letra de mas arriba y abajo (el dedo y el mouse no tienen que ser exactos)
    const float bx0 = (align == 1) ? (float)-maxPixels : 0.0f, bx1 = (align == 1) ? 0.0f : (float)maxPixels;
    float ax, ay, bx, by, tx, ty, ux, uy;
    APantalla(M, vp, bx0 - (float)cw, -(float)lh * 0.5f, ax, ay);
    APantalla(M, vp, bx1 + (float)cw, (float)lh * 1.5f, bx, by);
    APantalla(M, vp, (float)x0, 0.0f, tx, ty);
    APantalla(M, vp, (float)(x0 + cw), 0.0f, ux, uy);
    gGeomCampo.f = &f; gGeomCampo.x0 = tx; gGeomCampo.cw = (ux - tx) != 0.0f ? (ux - tx) : 1.0f; gGeomCampo.vis = vis;
    gGeomCampo.rx0 = ax < bx ? ax : bx; gGeomCampo.rx1 = ax < bx ? bx : ax;
    gGeomCampo.ry0 = ay < by ? ay : by; gGeomCampo.ry1 = ay < by ? by : ay;
}
// el byte del texto donde empieza la letra numero 'n' (UTF-8)
static int ByteDeLetra(const std::string& s, int n) {
    int i = 0, letras = 0;
    while (i < (int)s.size() && letras < n) { i++; while (i < (int)s.size() && ((unsigned char)s[i] & 0xC0) == 0x80) i++; letras++; }
    return i;
}

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
    if (&f == g_textFieldActivo) AnotarGeometria(f, x0, vis, cw, align, maxPixels);
    if (f.selectAll && total > 0) {
        // el FONDO de la seleccion atras del texto (y el texto en blanco encima, que se lea)
        w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::accentDark)]);
        RectCampo(x0 - g, -g, vis * cw + 2 * g, LetterHeightGS + 2 * g);
        w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]);
        RenderBitmapText(f.text, al, maxPixels);
        return;
    }
    RenderBitmapText(f.text, al, maxPixels);
    // UNA PARTE seleccionada (Shift + flechas, arrastrar el mouse): su fondo y esas letras en blanco encima
    if (f.HaySeleccion()) {
        int a, b; f.Seleccion(a, b);
        int ca = CharsUTF8(f.text, (size_t)a), cb = CharsUTF8(f.text, (size_t)b);
        if (ca > vis) ca = vis; if (cb > vis) cb = vis;
        if (cb > ca) {
            w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::accentDark)]);
            RectCampo(x0 + ca * cw, -g, (cb - ca) * cw, LetterHeightGS + 2 * g);
            w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]);
            const int ba = ByteDeLetra(f.text, ca), bb = ByteDeLetra(f.text, cb);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((float)(x0 + ca * cw), 0.0f, 0.0f);
            RenderBitmapText(f.text.substr((size_t)ba, (size_t)(bb - ba)), textAlign(textAlign::left), (cb - ca) * cw + cw);
            w3dEngine::PopMatrix();
        }
    }
    if (!CursorPrendido()) return;
    int c = CharsUTF8(f.text, f.caret < 0 ? 0 : (size_t)f.caret);
    if (c > vis) c = vis;
    RectCampo(x0 + c * cw - (c > 0 ? g / 2 : 0), -g, g, LetterHeightGS + 2 * g);   // el palito (del color del texto)
}

// ---- EL MOUSE sobre el campo enfocado (ver TextField.h) ----
bool TextFieldPuntoEnActivo(int mx, int my) {
    if (!g_textFieldActivo || gGeomCampo.f != g_textFieldActivo) return false;
    return (float)mx >= gGeomCampo.rx0 && (float)mx <= gGeomCampo.rx1 && (float)my >= gGeomCampo.ry0 && (float)my <= gGeomCampo.ry1;
}
// la letra mas cercana a esa x -> su byte
static int PosicionEn(int mx) {
    TextField* f = g_textFieldActivo;
    float n = ((float)mx - gGeomCampo.x0) / gGeomCampo.cw;
    int letra = (int)(n + 0.5f);
    if (letra < 0) letra = 0; if (letra > gGeomCampo.vis) letra = gGeomCampo.vis;
    return ByteDeLetra(f->text, letra);
}
bool TextFieldMouseAbajo(int mx, int my, bool shift, bool doble) {
    if (!TextFieldPuntoEnActivo(mx, my)) return false;
    TextField* f = g_textFieldActivo;
    if (doble) { f->SelectAll(); gArrastrandoCampo = false; return true; }
    const int p = PosicionEn(mx);
    if (shift) f->MoverA(p, true);
    else { f->Deseleccionar(); f->caret = p; f->ancla = p; }
    gArrastrandoCampo = true;
    return true;
}
bool TextFieldMouseArrastrar(int mx) {
    if (!gArrastrandoCampo || !g_textFieldActivo || gGeomCampo.f != g_textFieldActivo) return false;
    TextField* f = g_textFieldActivo;
    if (f->ancla < 0) f->ancla = f->caret;
    f->selectAll = false;
    f->caret = PosicionEn(mx);
    return true;
}
void TextFieldMouseArriba() { gArrastrandoCampo = false; }

// el campo enfocado (NULL = ninguno): estado de la biblioteca, lo lee el palito y lo pone quien enfoca
TextField* g_textFieldActivo = NULL;

// el palito parpadea: con un campo enfocado, cada medio segundo hay que redibujar (el loop lo pregunta por cuadro)
bool TextFieldParpadeoTick() {
    static int fase = -1;
    if (!g_textFieldActivo || g_textFieldActivo->selectAll) { fase = -1; return false; }
    const int ahora = CursorPrendido() ? 1 : 0;
    if (ahora == fase) return false;
    fase = ahora;
    return true;
}
