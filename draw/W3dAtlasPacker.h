#ifndef W3D_ATLAS_PACKER_H
#define W3D_ATLAS_PACKER_H
#include <vector>

// ============================================================================
//  W3dAtlasPacker — junta VARIAS imagenes RGBA en UN solo canvas (atlas):
//  una sola textura = un solo bind para dibujarlo todo. Lo usa el skin del
//  editor (font.png + iconos individuales) y sirve igual para JUEGOS (unir
//  las texturas de un nivel y bajar llamadas de OpenGL).
//
//  C++03 / sin GL: entran pixeles, sale un canvas (el que llama lo sube).
// ============================================================================

struct W3dAtlasItem {
    const unsigned char* rgba;  // pixeles RGBA 8888 de la imagen (no se liberan aca)
    int w, h;                   // su tamano
    bool fijo;                  // true: va SI o SI en la posicion (x,y) pedida
                                // (ej: font.png en el (0,0) para no invalidar coords)
    int x, y;                   // fijo: la posicion pedida. SALIDA: donde quedo.
};

// empaqueta los items en el canvas mas chico que los aguante (128x128 y crece:
// 256x128, 256x256, 512x256... hasta 1024x512). 'gutter' = aire entre items en px
// (2 evita que el filtrado LINEAR sangre al vecino). false si no entraron.
bool W3dAtlasPack(W3dAtlasItem* items, int n, int gutter,
                  std::vector<unsigned char>& outCanvas, int* outW, int* outH);

#endif // W3D_ATLAS_PACKER_H
