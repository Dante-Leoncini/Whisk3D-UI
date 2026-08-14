#ifndef PROPCOLOR_H
#define PROPCOLOR_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

class Object;   // (editor) el objeto dueno del color: contexto de PALETA del picker

class PropColor : public PropertieBase {
    public:
        PropColor(const std::string& Name);

        GLfloat* value;   // apunta a float[4] (init en ctor)
        bool Seleccionable() { return value != NULL; }
        GLfloat originalValue[4];
        // contexto de PALETA para el ColorPicker (pestania "Pal"): el campo
        // indice pal* del elemento (-1 = color propio) y el objeto dueno.
        // NULL = campo sin soporte de paleta (ej. materiales): sin pestania.
        int* palRef;
        Object* palObj;
        // canal de animacion (rombo de keyframe): animProp = AnimColor (color de luz/material);
        // el rombo keyea las 3 curvas RGB juntas (AnimX/Y/Z). -1 = no animable (sin rombo).
        int animProp, animComp;
        int AnimProp() W3D_OVERRIDE { return animProp; }
        int AnimComp() W3D_OVERRIDE { return animComp; }
        void SetGlColor();

        PropertyType GetType() W3D_OVERRIDE;
        // fila de PALETA del editor 2D: el indice de la entrada (-1 = fila normal).
        // El click handler lo usa para la crucecita de borrar.
        virtual int PaletaIdx() const { return -1; }

        void button_up() W3D_OVERRIDE;
        void button_down() W3D_OVERRIDE;
        void button_left() W3D_OVERRIDE;
        void button_right() W3D_OVERRIDE;
        bool Cancel() W3D_OVERRIDE;

        bool EditPropertie() W3D_OVERRIDE;
        void RenderPropertiBox(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiBoxBorder(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiValue(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiLabel(Card* propertiBox) W3D_OVERRIDE;
        int Resize(int w) W3D_OVERRIDE;
};

#endif