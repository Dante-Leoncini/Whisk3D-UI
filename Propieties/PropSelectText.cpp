#include "PropSelectText.h"
#include "w3dGraphics.h"

PropSelectText::PropSelectText(const std::string& Name):
    PropertieBase(Name){
};

void PropSelectText::RenderPropertiBox(Card* propertiBox){
    propertiBox->Render(false);
    w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
}

void PropSelectText::RenderPropertiBoxBorder(Card* propertiBox){
    w3dEngine::Translatef(0, -RowHeightGS - gapGS, 0);
    propertiBox->RenderBorder(false);
    w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
};

void PropSelectText::RenderPropertiValue(Card* propertiBox){
    //RenderBitmapFloat(*value, textAlign::left, propertiBox->width -bordersGS, unit);
    w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
}

void PropSelectText::RenderPropertiLabel(Card* propertiBox){
    RenderBitmapLabel(name, PropColLabel - bordersGS);
    w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
};

void PropSelectText::button_up(){
};

void PropSelectText::button_down(){
};

void PropSelectText::button_left(){
};

void PropSelectText::button_right(){
};

bool PropSelectText::Cancel(){
    //std::cout << "se cancelo originalValue: "<< originalValue << "\n";
    //*value = originalValue;
    //std::cout << "quedo asi: "<< *value << "\n";
    editing = false;
    return editing;
};

bool PropSelectText::EditPropertie(){
    editing = !editing;
    if (editing){
        //originalValue = *value;
        //std::cout << "se guardo originalValue: "<< originalValue << "\n";
    }
    return editing;
};

PropertyType PropSelectText::GetType(){
    return PropertyType::SelectText;
}

int PropSelectText::Resize(int w){
    width = w;
    return RowHeightGS + gapGS;
}