#ifndef W3D_TEXT_ATLAS_H
#define W3D_TEXT_ATLAS_H
// ============================================================================
//  W3dTextAtlas.h — WhiskUI: texto por ATLAS de fuente (horneado de un TTF con
//  bake_atlas.py). Header-only, C++03 / Symbian-safe.
//
//  Dibuja usando el render del Core (w3dEngine) con el atlas RGB (gris = cobertura,
//  SIN canal alpha) y mezcla SCREEN (ONE, ONE_MINUS_SRC_COLOR): el RGB gris hace de
//  alpha/cobertura, el color lo pone el vertex color. Ideal para texto claro.
//
//  A diferencia del bitmap fijo (W3dFont), aca cada glifo se escala/posiciona LIBRE
//  -> letras de TAMAÑO VARIABLE, superpuestas, animadas. NO rasteriza TTF en runtime
//  (eso seria stb_truetype); carga el atlas ya horneado (perfecto para N95).
//
//  Coordenadas: asume un ortho Y-DOWN (0 = arriba). El 'px' es el alto de fuente en
//  las unidades del ortho. El pen (x,y) es la cima del ascender del renglon.
// ============================================================================

#include "w3dGraphics.h"
#include "w3dTexture.h"   // UploadRGBA (antes dependia del orden de includes del caller)
#include "stb/stb_image.h"
#include <map>
#include <string.h>

namespace w3dui {
namespace gfx = w3dEngine;

struct W3dAtlasGlyph {
    float u0, v0, u1, v1;   // UV en el atlas
    short w, h;             // tamanio en px (a fontPx)
    short xoff, yoff;       // bearing desde el pen (yoff hacia ABAJO desde la cima del ascender)
    short advance;          // avance del pen
};

struct W3dTextAtlas {
    unsigned tex; int atlasW, atlasH, fontPx, lineH, ascent;
    int mezcla;   // modo de mezcla (default: premultiplicado -> texto de cualquier color + sombras)
    bool pixelPerfect; // fuente PIXEL (NEAREST): dibujar SOLO a multiplos enteros y pen entero
    std::map<unsigned, W3dAtlasGlyph> glyphs;
    W3dTextAtlas() : tex(0), atlasW(0), atlasH(0), fontPx(1), lineH(0), ascent(0),
                     mezcla(gfx::MezclaPremult), pixelPerfect(false) {}

    // lectura little-endian de un buffer
    static unsigned rdU16(const unsigned char* p){ return (unsigned)p[0] | ((unsigned)p[1]<<8); }
    static int      rdI16(const unsigned char* p){ int v=(int)p[0]|((int)p[1]<<8); return v>=32768? v-65536 : v; }
    static unsigned rdU32(const unsigned char* p){ return (unsigned)p[0]|((unsigned)p[1]<<8)|((unsigned)p[2]<<16)|((unsigned)p[3]<<24); }

    // proximo codepoint UTF-8 (1-2 bytes = latin1; 3-4 bytes se saltean). Avanza s.
    static unsigned NextCp(const char*& s) {
        unsigned char c = (unsigned char)*s++;
        if (c < 0x80) return c;
        if ((c & 0xE0) == 0xC0 && ((unsigned char)*s & 0xC0) == 0x80)
            return ((unsigned)(c & 0x1F) << 6) | ((unsigned char)*s++ & 0x3F);
        while (((unsigned char)*s & 0xC0) == 0x80) s++;
        return c;
    }

    // carga: bytes del .w3dfnt (metricas) + bytes del .png (atlas RGB gris). true si ok.
    bool Load(const unsigned char* fnt, size_t fntLen, const unsigned char* png, size_t pngLen) {
        if (!fnt || fntLen < 16 || memcmp(fnt, "W3DF", 4) != 0) return false;
        const unsigned char* p = fnt + 4;
        atlasW=(int)rdU16(p); atlasH=(int)rdU16(p+2); fontPx=(int)rdU16(p+4);
        lineH=rdI16(p+6); ascent=rdI16(p+8); int cnt=(int)rdU16(p+10); p += 12;
        if (fontPx < 1) fontPx = 1;
        for (int i=0; i<cnt; i++) {
            if ((size_t)(p - fnt) + 18 > fntLen) break;
            unsigned cp=rdU32(p);
            int x=(int)rdU16(p+4), y=(int)rdU16(p+6), w=(int)rdU16(p+8), h=(int)rdU16(p+10);
            int xo=rdI16(p+12), yo=rdI16(p+14), adv=rdI16(p+16); p += 18;
            W3dAtlasGlyph g;
            g.u0=(float)x/atlasW; g.v0=(float)y/atlasH; g.u1=(float)(x+w)/atlasW; g.v1=(float)(y+h)/atlasH;
            g.w=(short)w; g.h=(short)h; g.xoff=(short)xo; g.yoff=(short)yo; g.advance=(short)adv;
            glyphs[cp] = g;
        }
        // atlas -> textura. El archivo es RGB (gris = cobertura, sin alpha). Copio el gris al canal
        // ALPHA para poder alpha-blendear con PREMULTIPLICADO (texto de cualquier color + sombras).
        if (png && pngLen) {
            int iw=0, ih=0, ch=0; unsigned char* rgba = stbi_load_from_memory(png,(int)pngLen,&iw,&ih,&ch,4);
            if (rgba) {
                for (int i=0; i<iw*ih; i++) rgba[i*4+3] = rgba[i*4+0]; // alpha = cobertura (gris)
                tex=gfx::UploadRGBA(rgba, iw, ih, true); stbi_image_free(rgba);
                gfx::BindTexture(tex); gfx::TexFilter(true); gfx::TexWrap(false);
            }
        }
        return tex != 0 && !glyphs.empty();
    }

    const W3dAtlasGlyph* Glyph(unsigned cp) const {
        std::map<unsigned, W3dAtlasGlyph>::const_iterator it = glyphs.find(cp);
        return it == glyphs.end() ? 0 : &it->second;
    }

    // ancho del texto a tamanio 'px' (alto de fuente en unidades del ortho)
    float TextWidth(const char* s, float px) const {
        if (!s) return 0; float sc = px/(float)fontPx, w = 0;
        while (*s) { const W3dAtlasGlyph* g = Glyph(NextCp(s)); if (g) w += g->advance*sc; }
        return w;
    }
    float LineHeight(float px) const { return lineH * (px/(float)fontPx); }

    // prepara el estado (bindea atlas + mezcla). Llamalo antes de DrawText/DrawGlyph.
    // 'mezcla' (arriba) permite cambiar el modo; por defecto premultiplicado.
    void Begin() const {
        gfx::Disable(gfx::DepthTest); gfx::Disable(gfx::Lighting); gfx::Disable(gfx::CullFace);
        gfx::Enable(gfx::Texture2D); gfx::BindTexture(tex);
        gfx::Enable(gfx::Blend); gfx::SetMezcla(mezcla);
        gfx::EnableArray(gfx::VertexArray); gfx::EnableArray(gfx::TexCoordArray);
        gfx::DisableArray(gfx::ColorArray); gfx::DisableArray(gfx::NormalArray); // usar Color4f constante
    }
    void End() const { gfx::DisableArray(gfx::TexCoordArray); gfx::Disable(gfx::Blend); }

    // dibuja UN glifo (pen en x,y = cima del ascender, ortho Y-DOWN). Devuelve el avance a tamanio px.
    // Para letras de tamanio VARIABLE: llamalo por glifo con distinto px / color y posiciona a mano.
    float DrawGlyph(unsigned cp, float x, float y, float px, float r, float g, float b, float a) const {
        const W3dAtlasGlyph* gl = Glyph(cp); if (!gl) return 0;
        float sc = px/(float)fontPx;
        if (gl->w > 0 && gl->h > 0) {
            float x0=x+gl->xoff*sc, y0=y+gl->yoff*sc, x1=x0+gl->w*sc, y1=y0+gl->h*sc;
            float V[18]  = { x0,y0,0, x1,y0,0, x1,y1,0,  x0,y0,0, x1,y1,0, x0,y1,0 };
            float UV[12] = { gl->u0,gl->v0, gl->u1,gl->v0, gl->u1,gl->v1,  gl->u0,gl->v0, gl->u1,gl->v1, gl->u0,gl->v1 };
            gfx::VertexPointer3f(0, V); gfx::TexCoordPointer2f(0, UV);
            gfx::Color4f(r*a, g*a, b*a, a);   // PREMULTIPLICADO (blend ONE, ONE_MINUS_SRC_ALPHA)
            gfx::DrawTrianglesArray(6);
        }
        return gl->advance * sc;
    }

    // texto uniforme. x,y = pen (y = cima del ascender). Devuelve el ancho dibujado. Llama Begin() antes.
    float DrawText(const char* s, float x, float y, float px, float r, float g, float b, float a) const {
        if (!s) return 0; float x0 = x;
        while (*s) x += DrawGlyph(NextCp(s), x, y, px, r, g, b, a);
        return x - x0;
    }
};

} // namespace w3dui
#endif // W3D_TEXT_ATLAS_H
