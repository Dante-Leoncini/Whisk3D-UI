#!/usr/bin/env python3
# ============================================================================
#  bake_atlas.py — WhiskUI: hornea un ATLAS de fuente desde un TTF.
#
#  Genera un atlas RGB (glifo en escala de grises = cobertura; sin canal alpha)
#  + un archivo de metricas binario (.w3dfnt) que WhiskUI (W3dTextAtlas.h) carga
#  y usa para dibujar texto con letras de TAMAÑO VARIABLE (cada glifo se escala
#  y posiciona libre). Pensado para N95: se hornea en PC y se envia el atlas
#  (sin rasterizar TTF en runtime). El path C++ en runtime seria stb_truetype.
#
#  Uso:  bake_atlas.py <fuente.ttf> <size:128|256|512|1024> <fontPx> <salida_sin_ext>
#  Sale:  <salida>.png  (RGB, gris)   +   <salida>.w3dfnt  (metricas)
# ============================================================================
import sys, struct
from PIL import Image, ImageDraw, ImageFont

def main():
    if len(sys.argv) < 5:
        print("uso: bake_atlas.py <fuente.ttf> <atlasSize> <fontPx> <salida_sin_ext>"); return 1
    ttf, atlas, fpx, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    font = ImageFont.truetype(ttf, fpx)
    ascent, descent = font.getmetrics()
    lineH = ascent + descent

    # charset: ASCII imprimible (32..126) + acentos/simbolos del español
    chars = [chr(c) for c in range(32, 127)] + list("áéíóúÁÉÍÓÚñÑüÜ¿¡")

    # rasterizo cada glifo (gris) y saco metricas (bearing + advance)
    glyphs = []  # (codepoint, img|None, xoff, yoff, w, h, advance)
    pad = fpx
    for ch in chars:
        adv = int(round(font.getlength(ch)))
        big = Image.new('L', (fpx * 3 + pad * 2, fpx * 3 + pad * 2), 0)
        ImageDraw.Draw(big).text((pad, pad), ch, fill=255, font=font, anchor='la')
        bb = big.getbbox()
        if bb is None:  # espacio u otro sin pixeles
            glyphs.append((ord(ch), None, 0, 0, 0, 0, adv)); continue
        x0, y0, x1, y1 = bb
        w, h = x1 - x0, y1 - y0
        glyphs.append((ord(ch), big.crop(bb), x0 - pad, y0 - pad, w, h, adv))

    # packer por estantes (shelf): ordeno por alto desc y voy llenando filas
    order = sorted([g for g in glyphs if g[1] is not None], key=lambda g: -g[5])
    GAP = 1
    px, py, rowH = GAP, GAP, 0
    placed = {}  # codepoint -> (x,y,w,h)
    sheet = Image.new('L', (atlas, atlas), 0)
    for cp, img, xo, yo, w, h, adv in order:
        if px + w + GAP > atlas:
            px = GAP; py += rowH + GAP; rowH = 0
        if py + h + GAP > atlas:
            print("ERROR: no entran los glifos en %dx%d a %dpx. Subi el atlas o baja fontPx." % (atlas, atlas, fpx)); return 2
        sheet.paste(img, (px, py))
        placed[cp] = (px, py, w, h)
        px += w + GAP
        if h > rowH: rowH = h

    # atlas RGB (gris replicado a los 3 canales; SIN alpha)
    rgb = Image.merge('RGB', (sheet, sheet, sheet))
    rgb.save(out + ".png")

    # metricas binarias (little-endian): magic, atlas, fontPx, lineH, ascent, count, glifos
    with open(out + ".w3dfnt", "wb") as f:
        f.write(b"W3DF")
        f.write(struct.pack("<HHHhhH", atlas, atlas, fpx, lineH, ascent, len(glyphs)))
        for cp, img, xo, yo, w, h, adv in glyphs:
            x, y = placed.get(cp, (0, 0, 0, 0))[:2] if cp in placed else (0, 0)
            f.write(struct.pack("<IHHHHhhh", cp, x, y, w, h, xo, yo, adv))
    print("LISTO: %s.png (%dx%d RGB) + %s.w3dfnt (%d glifos, fontPx=%d, lineH=%d)" %
          (out, atlas, atlas, out, len(glyphs), fpx, lineH))
    return 0

sys.exit(main())
