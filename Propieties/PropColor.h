#ifndef PROPCOLOR_H
#define PROPCOLOR_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

class PropColor : public PropertieBase {
    public:
        PropColor(const std::string& Name);

        GLfloat* value;   // apunta a float[4] (init en ctor)
        bool Seleccionable() { return value != NULL; }
        GLfloat originalValue[4];
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