#ifndef PROPBOOL_H
#define PROPBOOL_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"

extern std::string unicodeCheck;

class PropBool : public PropertieBase {
    public:
        PropBool(const std::string& Name);

        bool* value;
        bool Seleccionable() { return value != NULL; }
        void (*onChange)(); // se llama al togglear (lo usa el redo-panel: Regenerar)
        int animProp, animComp;
        int AnimProp() W3D_OVERRIDE { return animProp; }
        int AnimComp() W3D_OVERRIDE { return animComp; }  // canal animable (rombo a la izq); -1 = no animable

        PropertyType GetType() W3D_OVERRIDE;

        bool EditPropertie() W3D_OVERRIDE;

        void RenderPropertiBox(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiBoxBorder(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiValue(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiLabel(Card* propertiBox) W3D_OVERRIDE;
        int Resize(int w) W3D_OVERRIDE;
};

#endif