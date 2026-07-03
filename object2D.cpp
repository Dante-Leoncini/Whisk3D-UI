#include "object2D.h"

// Constructor
Object2D::Object2D(Object2D* parent)
    : Parent(parent), visible(true), x(0), y(0), width(10), height(10),
      scaleX(1), scaleY(1), opacity(255)
{
    color[0] = color[1] = color[2] = 255;
}

// Tipo por defecto
Object2DType Object2D::getType() {
    return Object2DType::empty;
}

// Render del objeto base
void Object2D::RenderObject(bool usarColorPropio) {
    // vacío: los hijos lo implementan
}

// Destructor
Object2D::~Object2D() {
    for (size_t c = 0; c < Childrens.size(); c++) {
        delete Childrens[c];
    }
    Childrens.clear();
}

// Render general (incluye los hijos)
void Object2D::Render(bool usarColorPropio) {
    if (!visible) return;

    RenderObject(usarColorPropio);

    for (size_t c = 0; c < Childrens.size(); c++) {
        Childrens[c]->Render(usarColorPropio);
    }
}