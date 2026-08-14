#ifndef PROPSELECTTEXT_H
#define PROPSELECTTEXT_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

class PropSelectText : public PropertieBase {
    public:
        PropSelectText(const std::string& Name);

        PropertyType GetType() W3D_OVERRIDE;

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