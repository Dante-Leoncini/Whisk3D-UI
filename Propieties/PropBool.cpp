#include "PropBool.h"
#include "w3dGraphics.h"

// el glyph sale de card.h (CheckboxTilde): un solo lugar decide como se ve un checkbox
std::string unicodeCheck = CheckboxTilde();

PropBool::PropBool(const std::string& Name):
    PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    value = NULL;
    onChange = NULL;
};

// El checkbox va SIEMPRE pegado a la DERECHA (pantallas chicas: el N95 recortaba la etiqueta y
// dejaba espacio muerto). La etiqueta ocupa TODO el ancho a su izquierda (menos un gap).
// dxCheck = cuanto correr desde el origen (colEtiqueta) hasta el borde IZQUIERDO del checkbox.
static int CheckBoxAncho() { return RenglonHeightGS + GlobalScale * 2; }

void PropBool::RenderPropertiBox(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColEtiqueta - propertiBox->width - bordersGS);
        w3dEngine::Translatef(dx, 0, 0);
        propertiBox->Render(false);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
}

void PropBool::RenderPropertiBoxBorder(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColEtiqueta - propertiBox->width - bordersGS);
        w3dEngine::Translatef(dx, -RenglonHeightGS - gapGS, 0);
        propertiBox->RenderBorder(false);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
};

void PropBool::RenderPropertiValue(Card* propertiBox){
    if (value){
        int cb = CheckBoxAncho();
        // el value-origin esta en colEtiqueta+borderGS (NO colEtiqueta) -> el ✔ se centra en el MISMO
        // borde izq que el box (width-PropColEtiqueta-cb-bordersGS), corrido el borderGS del origen.
        float dx = (float)(width - PropColEtiqueta - cb - bordersGS - borderGS);
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapText(unicodeCheck, textAlign::center, cb);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
}

void PropBool::RenderPropertiLabel(Card* propertiBox){
    if (value){
        int cb = CheckBoxAncho();
        // la etiqueta termina (derecha-alineada) justo antes del checkbox y ocupa todo a su izquierda
        float dx = (float)(width - PropColEtiqueta - cb - bordersGS - gapGS);
        int wLabel = width - cb - bordersGS - gapGS * 2;
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapText(name, textAlign::right, wLabel);
        w3dEngine::Translatef(-dx, RenglonHeightGS + gapGS, 0);
    }
};

bool PropBool::EditPropertie(){
    if (value){
        *value = !*value;
        if (onChange) onChange(); // ej: redo-panel -> Regenerar con/sin smooth
    }
    return false;
};

PropertyType PropBool::GetType(){
    return PropertyType::Bool;
}

int PropBool::Resize(int w){
    if (!value) return 0;
    width = w;
    return RenglonHeightGS + gapGS;
}