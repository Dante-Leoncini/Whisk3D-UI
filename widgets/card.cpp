#include "card.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"
#include "WhiskUI/theme/colors.h"    // ColorList (colores del checkbox)
#include "WhiskUI/text/bitmapText.h" // RenderBitmapText (el tilde)

// UVs e indices
GLubyte CardIndices[] = {
    0,1, 4, 1,4, 5,   1, 2, 5, 5, 2,  6,   2, 3, 6, 6, 3, 7,
    4,5, 8, 8,5, 9,   5, 6, 9, 6, 9, 10,   6, 7,10,10, 7,11,
    8,9,12,12,9,13,   9,10,13,13,10, 14,  10,11,14,14,11,15
};

GLubyte CardBorderIndices[] = {
    0,1, 4, 1,4, 5,   1, 2, 5, 5, 2,  6,   2, 3, 6, 6, 3, 7,
    4,5, 8, 8,5, 9,                        6, 7,10,10, 7,11,
    8,9,12,12,9,13,   9,10,13,13,10, 14,  10,11,14,14,11,15
};

GLfloat CardUV[32] = {
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f
};

GLfloat CardBorderUV[32] = {
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f,
    0.0f,      0.0f
};

GLfloat CardTabUV[32]       = {0};
GLfloat CardTabBorderUV[32] = {0};

void CalcCardUV(int texW, int texH) {
    GLfloat* uv = CardUV;

    // Coordenadas UV en píxeles (borde de 13px, esquinas de 6px, centro de 1px)
    float U[4] = { 122.0f / texW, 124.0f / texW, 125.0f / texW, 127.0f / texW };
    float V[4] = { 103.0f / texH, 105.0f / texH, 106.0f / texH, 108.0f / texH };

    // Generar los 16 pares UV (fila × columna)
    int k = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            uv[k++] = U[x];
            uv[k++] = V[y];
        }
    }

    GLfloat* uvBorder = CardBorderUV;

    // Coordenadas UV en píxeles (borde de 13px, esquinas de 6px, centro de 1px)
    float Ub[4] = { 116.0f / texW, 118.0f / texW, 119.0f / texW, 121.0f / texW };
    float Vb[4] = { 103.0f / texH, 105.0f / texH, 106.0f / texH, 108.0f / texH };

    // Generar los 16 pares UV (fila × columna)
    k = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            uvBorder[k++] = Ub[x];
            uvBorder[k++] = Vb[y];
        }
    }

    // versiones "pestania": iguales pero la fila de ABAJO (vertices 12..15) usa
    // la V de la fila del medio (vertices 8..11). Asi la franja inferior estira
    // la parte RECTA (sin esquina redondeada ni borde inferior) y la pestania
    // se funde plana con la tarjeta de contenido de abajo.
    for (int i = 0; i < 32; i++) {
        CardTabUV[i]       = CardUV[i];
        CardTabBorderUV[i] = CardBorderUV[i];
    }
    for (int c = 0; c < 4; c++) {
        CardTabUV[(12 + c) * 2 + 1]      = CardUV[(8 + c) * 2 + 1];
        CardTabBorderUV[(12 + c) * 2 + 1] = CardBorderUV[(8 + c) * 2 + 1];
    }
}

// Constructor
Card::Card(Object2D* parent, int w, int h) : Object2D(parent) {
    {
        static const GLshort KDef[32] = {
            0,0,   6,0,   12,0,   18,0,
            0,6,   6,6,   12,6,   18,6,
            0,12,  6,12,  12,12,  18,12,
            0,18,  6,18,  12,18,  18,18
        };
        for (int i = 0; i < 32; i++) { mesh[i] = KDef[i]; }
    }

    Resize(w, h);
}

// ------------------ Resize ------------------
void Card::Resize(int w, int h) {
    width = w;
    height = h;
    GLshort U[4] = { 0, (GLshort)(2*GlobalScale), (GLshort)(width - 2*GlobalScale), (GLshort)(width) };
    GLshort V[4] = { 0, (GLshort)(2*GlobalScale), (GLshort)(height - 2*GlobalScale), (GLshort)(height) };

    int k = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            mesh[k++] = U[x];
            mesh[k++] = V[y];
        }
    }
}

// Cambia el color del rectángulo
void Card::SetColor(GLubyte net, GLubyte green, GLubyte blue) {
    color[0] = net;
    color[1] = green;
    color[2] = blue;
}

// Renderiza el rectángulo
void Card::RenderObject(bool useColorOwn) {
    if (useColorOwn)
        w3dEngine::Color4ub(color[0], color[1], color[2], opacity);

    W3dDrawElemsB(mesh, CardUV, CardIndices, 54);
}

// Renderiza el rectángulo
void Card::RenderBorder(bool useColorOwn) {
    if (useColorOwn)
        w3dEngine::Color4ub(color[0], color[1], color[2], opacity);

    W3dDrawElemsB(mesh, CardBorderUV, CardBorderIndices, 48);
}

// versiones "pestania" (bottom recto): el color lo pone el caller
void Card::RenderObjectTab() {
    W3dDrawElemsB(mesh, CardTabUV, CardIndices, 54);
}
void Card::RenderBorderTab() {
    W3dDrawElemsB(mesh, CardTabBorderUV, CardBorderIndices, 48);
}
// ---------------------------------------------------------------------------
//  EL checkbox del editor (ver card.h). Tamaño y colores viven SOLO aca.
// ---------------------------------------------------------------------------
int CheckboxSide(){ return RowHeightGS; }
const float* CheckboxBoxColor(bool on){
    return ColorList[static_cast<int>(on ? ColorID::grayUI : ColorID::background)];
}
const float* CheckboxCheckColor(){ return ColorList[static_cast<int>(ColorID::background)]; }
const char*  CheckboxCheck(){ return "\xE2\x9C\x94"; }

void DrawSampleColor(int side, const float* rgba){
    static Card* box = NULL;
    if (!box) box = new Card(NULL, side, side);
    box->Resize(side, side);
    w3dEngine::Color4f(rgba[0], rgba[1], rgba[2], 1.0f);
    box->RenderObject(false);
}
void DrawCheckbox(int side, bool on){
    static Card* box = NULL;
    if (!box) box = new Card(NULL, side, side);
    box->Resize(side, side);
    const float* c = CheckboxBoxColor(on);
    w3dEngine::Color4f(c[0], c[1], c[2], 1.0f);
    box->RenderObject(false);
    if (on){
        const float* t = CheckboxCheckColor();
        w3dEngine::Color4f(t[0], t[1], t[2], 1.0f);
        RenderBitmapText(CheckboxCheck(), textAlign::center, side);
    }
}
