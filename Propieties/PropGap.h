#ifndef PROPGAP_H
#define PROPGAP_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

class PropGap : public PropertieBase {
    public:
        PropGap(const std::string& Name);

        PropertyType GetType() W3D_OVERRIDE;
        bool Seleccionable() { return false; }

        void RenderPropertiBox(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiValue(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiLabel(Card* propertiBox) W3D_OVERRIDE;
        int Resize(int w) W3D_OVERRIDE;
};

#endif