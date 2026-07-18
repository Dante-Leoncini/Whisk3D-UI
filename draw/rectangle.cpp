#include "rectangle.h"
#include "w3dGraphics.h"

// Constructor
Rec2D::Rec2D(Object2D* parent) : Object2D(parent) {
    SetSize(0, 0, 10, 10); // default (era inicializador de clase: C++03)
}

// Configura tamaño y recalcula vértices
void Rec2D::SetSize(GLshort x, GLshort y, GLshort w, GLshort h) {
    width = w;
    height = h;

    vertices[0] = x;
    vertices[1] = y;

    vertices[2] = x + w;
    vertices[3] = y;

    vertices[4] = x;
    vertices[5] = y + h;

    vertices[6] = x + w;
    vertices[7] = y + h;

    // strip expandido a 2 triangulos float: camino UNICO de la UI por la abstraccion (4 OS)
    verticesTri[0]  = (GLfloat)vertices[0]; verticesTri[1]  = (GLfloat)vertices[1];
    verticesTri[2]  = (GLfloat)vertices[2]; verticesTri[3]  = (GLfloat)vertices[3];
    verticesTri[4]  = (GLfloat)vertices[4]; verticesTri[5]  = (GLfloat)vertices[5];
    verticesTri[6]  = (GLfloat)vertices[4]; verticesTri[7]  = (GLfloat)vertices[5];
    verticesTri[8]  = (GLfloat)vertices[2]; verticesTri[9]  = (GLfloat)vertices[3];
    verticesTri[10] = (GLfloat)vertices[6]; verticesTri[11] = (GLfloat)vertices[7];
}

// Cambia el color del rectángulo
void Rec2D::SetColor(GLubyte red, GLubyte green, GLubyte blue) {
    color[0] = red;
    color[1] = green;
    color[2] = blue;
}

// Renderiza el rectángulo
void Rec2D::RenderObject(bool usarColorPropio) {
    if (usarColorPropio)
        w3dEngine::Color4ub(color[0], color[1], color[2], opacity);

    w3dEngine::VertexPointer2f(0, verticesTri);
    w3dEngine::TexCoordPointer2f(8, verticesTri); // dummy valido
    w3dEngine::DrawTrianglesArray(6);
}