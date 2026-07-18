#include "WhiskUI/draw/glesdraw.h"
#include "w3dGraphics.h" // la abstraccion del engine (sin OpenGL directo)

// El dibujo 2D de la UI ya NO llama OpenGL directo: va por w3dEngine (gfx::), asi la UI
// es portable a cualquier backend grafico. UN solo camino para PC y Symbian: convierte
// los verts a FLOAT en un pool rotativo (el driver del N95 no quiere GL_SHORT) y dibuja
// con TRIANGLES/LINES por DrawArrays (glDrawElements GL_LINES no anda en el N95).
namespace gfx = w3dEngine;

static const int KPoolSlots = 256;
static GLfloat gPoolV[KPoolSlots][128]; // hasta 64 vertices xy float
static GLfloat gPoolU[KPoolSlots][128];
static int gPoolIdx = 0;

int W3dPantallaAlto = 0; // alto de pantalla (flip de Y; lo setea main.cpp / el container)

void W3dDrawStrip4(const GLshort* aVerts8, const GLfloat* aUVs8) {
    GLfloat* v = gPoolV[gPoolIdx];
    GLfloat* u = gPoolU[gPoolIdx];
    gPoolIdx = (gPoolIdx + 1) % KPoolSlots;
    // strip 0,1,2,3 -> triangulos 0,1,2 + 2,1,3
    static const int KOrden[6] = { 0, 1, 2, 2, 1, 3 };
    for (int i = 0; i < 6; i++) {
        v[i*2 + 0] = (GLfloat)aVerts8[KOrden[i]*2 + 0];
        v[i*2 + 1] = (GLfloat)aVerts8[KOrden[i]*2 + 1];
        u[i*2 + 0] = aUVs8[KOrden[i]*2 + 0];
        u[i*2 + 1] = aUVs8[KOrden[i]*2 + 1];
    }
    gfx::VertexPointer2f(0, v);
    gfx::TexCoordPointer2f(0, u);
    gfx::DrawTrianglesArray(6);
}

void W3dDrawElemsB(const GLshort* aVerts, const GLfloat* aUVs,
                   const GLubyte* aIdx, int aCount) {
    if (aCount > 64) aCount = 64;
    GLfloat* v = gPoolV[gPoolIdx];
    GLfloat* u = gPoolU[gPoolIdx];
    gPoolIdx = (gPoolIdx + 1) % KPoolSlots;
    for (int i = 0; i < aCount; i++) {
        int vi = aIdx[i];
        v[i*2 + 0] = (GLfloat)aVerts[vi*2 + 0];
        v[i*2 + 1] = (GLfloat)aVerts[vi*2 + 1];
        u[i*2 + 0] = aUVs[vi*2 + 0];
        u[i*2 + 1] = aUVs[vi*2 + 1];
    }
    gfx::VertexPointer2f(0, v);
    gfx::TexCoordPointer2f(0, u);
    gfx::DrawTrianglesArray(aCount);
}

void W3dDrawLinesS(int aLineWidth, int aCantidad,
                   const GLshort* aVerts3, const GLushort* aIdx) {
    GLfloat* v = gPoolV[gPoolIdx];
    gPoolIdx = (gPoolIdx + 1) % KPoolSlots;
    int n = aCantidad * 2;
    if (n > 42) n = 42; // 42 vertices x 3 = 126 floats (slot de 128)
    for (int i = 0; i < n; i++) {
        int vi = aIdx[i];
        v[i*3 + 0] = (GLfloat)aVerts3[vi*3 + 0];
        v[i*3 + 1] = (GLfloat)aVerts3[vi*3 + 1];
        v[i*3 + 2] = (GLfloat)aVerts3[vi*3 + 2];
    }
    gfx::LineWidth((float)aLineWidth);
    gfx::VertexPointer3f(0, v);
    gfx::TexCoordPointer2f(12, v); // texcoord dummy valido (lee xy del propio vertice)
    gfx::DrawLines(n);
}

void W3dDrawLinesF(const GLfloat* aVerts3, const GLushort* aIdx, int aIdxCount) {
    // en TANDAS de 40 vertices (pares): el piso manda 96 indices
    int hechos = 0;
    while (hechos < aIdxCount) {
        int n = aIdxCount - hechos;
        if (n > 40) n = 40;
        GLfloat* v = gPoolV[gPoolIdx];
        gPoolIdx = (gPoolIdx + 1) % KPoolSlots;
        for (int i = 0; i < n; i++) {
            int vi = aIdx[hechos + i];
            v[i*3 + 0] = aVerts3[vi*3 + 0];
            v[i*3 + 1] = aVerts3[vi*3 + 1];
            v[i*3 + 2] = aVerts3[vi*3 + 2];
        }
        gfx::VertexPointer3f(0, v);
        gfx::TexCoordPointer2f(12, v); // texcoord dummy valido
        gfx::DrawLines(n);
        hechos += n;
    }
}
// w3dSetColor: es del engine (w3dBase.h), vive en w3dGraphics.cpp (PC + Symbian)
