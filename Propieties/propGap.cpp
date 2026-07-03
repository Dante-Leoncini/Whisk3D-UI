#include "PropGap.h"
#include "w3dGraphics.h"

PropGap::PropGap(const std::string& Name):
    PropertieBase(Name){
};

void PropGap::RenderPropertiBox(Card* propertiBox){
    w3dEngine::Translatef(0, gapGS, 0);
}

void PropGap::RenderPropertiValue(Card* propertiBox){
    w3dEngine::Translatef(0, gapGS, 0);
}

void PropGap::RenderPropertiLabel(Card* propertiBox){
    w3dEngine::Translatef(0, gapGS, 0);
}

int PropGap::Resize(int w){
    return gapGS;
}

PropertyType PropGap::GetType(){
    return PropertyType::Gap;
}