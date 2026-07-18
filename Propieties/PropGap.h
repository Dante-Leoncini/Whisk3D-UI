#ifndef PROPGAP_H
#define PROPGAP_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

class PropGap : public PropertieBase {
    public:
        PropGap(const std::string& Name);

        PropertyType GetType() override;
        bool Seleccionable() { return false; }

        void RenderPropertiBox(Card* propertiBox) override;
        void RenderPropertiValue(Card* propertiBox) override;
        void RenderPropertiLabel(Card* propertiBox) override;
        int Resize(int w) override;
};

#endif