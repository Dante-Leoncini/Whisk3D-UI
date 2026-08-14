#ifndef PROPSEPARATOR_H
#define PROPSEPARATOR_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"
#include "w3dGraphics.h" // w3dEngine + ListaColores/ColorID

// fila SEPARADORA: una linea horizontal fina (gris) a lo ancho de la tarjeta. No seleccionable.
// Header-only (no agrega .cpp al MMP de Symbian). Se usa p/ separar visualmente secciones de un grupo.
class PropSeparator : public PropertieBase {
    public:
        bool oculto; // con oculto = true no dibuja la linea ni ocupa fila (ej: material por defecto)

        PropSeparator() : PropertieBase(""), oculto(false) {}

        PropertyType GetType() { return PropertyType::Gap; } // como un gap: no editable/seleccionable
        bool Seleccionable() { return false; }
        // distancia: gap (arriba) + LINEA de 2px + gap (abajo)
        int Resize(int w) { width = w; return oculto ? 0 : (gapGS * 2 + 2); }

        void RenderPropertiBox(Card* propertiBox) {
            if (oculto) return;
            // linea OSCURA fina (negro, IDENTICA al separador de titulo de los submenus: 2px FIJOS con
            // triangulos, sin textura) a lo ancho de UN BOTON (width - bordersGS), al margen izquierdo.
            const float* ac = ListaColores[static_cast<int>(ColorID::negro)];
            float x0 = (float)-PropColEtiqueta, x1 = x0 + (float)(width - bordersGS);
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
            w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); // gap arriba + linea(2) + gap abajo
        }
        void RenderPropertiValue(Card* propertiBox) { if (oculto) return; w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); }
        void RenderPropertiLabel(Card* propertiBox) { if (oculto) return; w3dEngine::Translatef(0, (GLfloat)(gapGS * 2 + 2), 0); }
};

#endif // PROPSEPARATOR_H
