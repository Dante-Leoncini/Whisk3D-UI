#include "PropColor.h"
#include "w3dGraphics.h"

PropColor::PropColor(const std::string& Name):
    PropertieBase(Name){
    value = NULL;
};

// el swatch de color = un CUADRADO pegado a la DERECHA (mismo tamaño/posicion que el checkbox);
// la etiqueta ocupa todo el ancho a su izquierda. (Espeja a PropBool.)
void PropColor::RenderPropertiBox(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColEtiqueta - propertiBox->width - bordersGS);
        w3dEngine::Translatef(dx, 0, 0);
        propertiBox->Render(false);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
}

void PropColor::RenderPropertiBoxBorder(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColEtiqueta - propertiBox->width - bordersGS);
        w3dEngine::Translatef(dx, -RenglonHeightGS - gapGS, 0);
        propertiBox->RenderBorder(false);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
};

void PropColor::RenderPropertiValue(Card* propertiBox){
    if (value){
        //RenderBitmapFloat(*value, textAlign::left, propertiBox->width -bordersGS, unit);
        w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
    }
}

void PropColor::RenderPropertiLabel(Card* propertiBox){
    if (value){
        int cb = RenglonHeightGS + GlobalScale * 2; // mismo ancho que el checkbox/swatch
        float dx = (float)(width - PropColEtiqueta - cb - bordersGS - gapGS);
        int wLabel = width - cb - bordersGS - gapGS * 2;
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapText(name, textAlign::right, wLabel);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
};

void PropColor::button_up(){
    *value += 0.01f;
};

void PropColor::button_down(){
    *value -= 0.01f;
};

void PropColor::button_left(){
    *value -= 0.1f;
};

void PropColor::button_right(){
    *value += 0.1f;
};

void PropColor::SetGlColor(){
    if (!value) return;

    w3dEngine::Color4f(value[0], value[1], value[2], 1.0f);
};

bool PropColor::Cancel(){
    if (!value) return false;
    value[0] = originalValue[0];
    value[1] = originalValue[1];
    value[2] = originalValue[2];
    value[3] = originalValue[3];
    editando = false;
    return editando;
};

bool PropColor::EditPropertie(){
    editando = !editando;
    if (editando && value){
        originalValue[0] = value[0];
        originalValue[1] = value[1];
        originalValue[2] = value[2];
        originalValue[3] = value[3];
    }
    return editando;
}

PropertyType PropColor::GetType(){
    return PropertyType::Color;
}

int PropColor::Resize(int w){
    if (!value) return 0; // sin valor no se dibuja: la tarjeta de mesh
    // parts se expandia de mas con el material por defecto
    width = w;
    return RenglonHeightGS + gapGS;
}