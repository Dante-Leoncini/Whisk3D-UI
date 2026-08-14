#ifndef PROPCOLOR_H
#define PROPCOLOR_H

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
        int AnimProp() override { return animProp; }
        int AnimComp() override { return animComp; }
        void SetGlColor();

        PropertyType GetType() override;
        // fila de PALETA del editor 2D: el indice de la entrada (-1 = fila normal).
        // El click handler lo usa para la crucecita de borrar.
        virtual int PaletaIdx() const { return -1; }

        void button_up() override;
        void button_down() override;
        void button_left() override;
        void button_right() override;
        bool Cancel() override;

        bool EditPropertie() override;
        void RenderPropertiBox(Card* propertiBox) override;
        void RenderPropertiBoxBorder(Card* propertiBox) override;
        void RenderPropertiValue(Card* propertiBox) override;
        void RenderPropertiLabel(Card* propertiBox) override;
        int Resize(int w) override;
};

#endif