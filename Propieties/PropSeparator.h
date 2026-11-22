#ifndef PROPSEPARATOR_H
#define PROPSEPARATOR_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"
#include "w3dGraphics.h" // w3dEngine + ColorList/ColorID

// fila SEPARADORA: una linea horizontal fina (gris) a lo ancho de la tarjeta. No seleccionable.
// Header-only (no agrega .cpp al MMP de Symbian). Se usa p/ separar visualmente secciones de un grupo.
class PropSeparator : public PropertieBase {
    public:
        bool hidden; // con oculto = true no dibuja la linea ni ocupa fila (ej: material por defecto)

        PropSeparator() : PropertieBase(""), hidden(false) {}

        PropertyType GetType() { return PropertyType::Gap; } // como un gap: no editable/seleccionable
        bool Selectable() { return false; }
        // distancia: gap (arriba) + LINEA de 2px + gap (abajo)
        int Resize(int w) { width = w; return hidden ? 0 : (gapGS * 2 + 2); }

        void RenderPropertiBox(Card* propertiBox) {
            if (hidden) return;
            // linea OSCURA fina (negro, IDENTICA al separador de titulo de los submenus: 2px FIJOS con
            // triangulos, sin textura) a lo ancho de UN BOTON (width - bordersGS), al margen izquierdo.
            const float* ac = ColorList[static_cast<int>(ColorID::grayLine)];   // (el mismo gris que las de los menus)
            float x0 = (float)-PropColLabel, x1 = x0 + (float)(width - bordersGS);
            float y0 = (float)gapGS, y1 = y0 + 2.0f;
            GLfloat sep[18] = { x0,y0,0, x1,y0,0, x1,y1,0,  x0,y0,0, x1,y1,0, x0,y1,0 };
            w3dEngine::Color4f(ac[0], ac[1], ac[2], 1.0f);
            w3dEngine::Disable(w3dEngine::Texture2D);
            w3dEngine::DisableArray(w3dEngine::TexCoordArray);
            w3dEngine::VertexPointer3f(0, sep);
            w3dEngine::DrawTrianglesArray(6);
            w3dEngine::EnableArray(w3dEngine::TexCoordArray);
            w3dEngine::Enable(w3dEngine::Texture2D);
            w3dEngine::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
            w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); // gap arriba + line(2) + gap abajo
        }
        void RenderPropertiValue(Card* propertiBox) { if (hidden) return; w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); }
        void RenderPropertiLabel(Card* propertiBox) { if (hidden) return; w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); }
};

#endif // PROPSEPARATOR_H
