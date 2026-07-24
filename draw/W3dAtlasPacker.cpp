// ============================================================================
//  W3dAtlasPacker.cpp — ver W3dAtlasPacker.h.
// ============================================================================
#include "W3dAtlasPacker.h"
#include <string.h>

// copia un bloque RGBA adentro del canvas (con recorte defensivo)
static void Blit(unsigned char* canvas, int cw, int ch,
                 const unsigned char* src, int sw, int sh, int dx, int dy) {
    for (int y = 0; y < sh; y++) {
        if (dy + y >= ch) break;
        int n = (dx + sw <= cw) ? sw : cw - dx;
        if (n <= 0) break;
        memcpy(canvas + ((dy + y) * cw + dx) * 4, src + y * sw * 4, n * 4);
    }
}

// (x,y,w,h) pisa algun item FIJO? devuelve su borde derecho para saltearlo
static bool PisaFijo(const W3dAtlasItem* items, int n, int gutter,
                     int x, int y, int w, int h, int* outSalto) {
    for (int i = 0; i < n; i++) {
        if (!items[i].fijo) continue;
        if (x < items[i].x + items[i].w + gutter && x + w > items[i].x &&
            y < items[i].y + items[i].h + gutter && y + h > items[i].y) {
            *outSalto = items[i].x + items[i].w + gutter;
            return true;
        }
    }
    return false;
}

bool W3dAtlasPack(W3dAtlasItem* items, int n, int gutter,
                  std::vector<unsigned char>& outCanvas, int* outW, int* outH) {
    static const int candW[6] = { 128, 256, 256, 512, 512, 1024 };
    static const int candH[6] = { 128, 128, 256, 256, 512, 512 };

    for (int c = 0; c < 6; c++) {
        int W = candW[c], H = candH[c];
        // los fijos tienen que entrar tal cual
        bool ok = true;
        for (int i = 0; i < n && ok; i++)
            if (items[i].fijo && (items[i].x + items[i].w > W || items[i].y + items[i].h > H))
                ok = false;
        if (!ok) continue;

        // los sueltos: filas de izquierda a derecha, salteando los fijos
        int x = 0, y = 0, filaAlto = 0;
        for (int i = 0; i < n && ok; i++) {
            if (items[i].fijo) continue;
            int w = items[i].w + gutter, h = items[i].h + gutter;
            for (;;) {
                if (x + w > W) {                    // fin de fila: bajar
                    y += filaAlto ? filaAlto : h;
                    filaAlto = 0;
                    x = 0;
                    if (y + h > H) { ok = false; break; }
                    continue;
                }
                int salto = 0;
                if (PisaFijo(items, n, gutter, x, y, w, h, &salto)) { x = salto; continue; }
                items[i].x = x; items[i].y = y;
                x += w;
                if (h > filaAlto) filaAlto = h;
                break;
            }
        }
        if (!ok) continue;

        // entro: componer el canvas
        outCanvas.assign((size_t)W * H * 4, 0);
        for (int i = 0; i < n; i++)
            if (items[i].rgba)
                Blit(&outCanvas[0], W, H, items[i].rgba, items[i].w, items[i].h,
                     items[i].x, items[i].y);
        *outW = W; *outH = H;
        return true;
    }
    return false;
}
