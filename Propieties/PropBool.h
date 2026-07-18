#ifndef PROPBOOL_H
#define PROPBOOL_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

extern std::string unicodeCheck;

class PropBool : public PropertieBase {
    public:
        PropBool(const std::string& Name);

        bool* value;
        bool Seleccionable() { return value != NULL; }
        void (*onChange)(); // se llama al togglear (lo usa el redo-panel: Regenerar)

        PropertyType GetType() override;

        bool EditPropertie() override;

        void RenderPropertiBox(Card* propertiBox) override;
        void RenderPropertiBoxBorder(Card* propertiBox) override;
        void RenderPropertiValue(Card* propertiBox) override;
        void RenderPropertiLabel(Card* propertiBox) override;
        int Resize(int w) override;
};

#endif