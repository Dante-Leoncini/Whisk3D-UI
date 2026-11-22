#include "PropBool.h"
#include "w3dGraphics.h"

// el glyph sale de card.h (CheckboxCheck): un solo lugar decide como se ve un checkbox
std::string unicodeCheck = CheckboxCheck();

PropBool::PropBool(const std::string& Name):
    PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    value = NULL;
    onChange = NULL;
};

// El checkbox va SIEMPRE pegado a la DERECHA (pantallas chicas: el N95 recortaba la etiqueta y
// dejaba espacio muerto). La etiqueta ocupa TODO el ancho a su izquierda (menos un gap).
// dxCheck = cuanto correr desde el origen (colLabel) hasta el borde IZQUIERDO del checkbox.
static int CheckBoxWidth() { return RowHeightGS + GlobalScale * 2; }

void PropBool::RenderPropertiBox(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColLabel - propertiBox->width - bordersGS - g_propKeyBtnCol);
        w3dEngine::Translatef(dx, 0, 0);
        propertiBox->Render(false);
        w3dEngine::Translatef(-dx, RowHeightGS + gapGS, 0);
    }
}

void PropBool::RenderPropertiBoxBorder(Card* propertiBox){
    if (value){
        float dx = (float)(width - PropColLabel - propertiBox->width - bordersGS - g_propKeyBtnCol);
        w3dEngine::Translatef(dx, -RowHeightGS - gapGS, 0);
        if (altered) {
            // (un override de este proyecto: el BORDE de la casilla en amarillo, se vea tildada o no)
            GLubyte r = propertiBox->color[0], g = propertiBox->color[1], b = propertiBox->color[2];
            propertiBox->color[0] = 255; propertiBox->color[1] = 220; propertiBox->color[2] = 46;
            propertiBox->RenderBorder(true);
            propertiBox->color[0] = r; propertiBox->color[1] = g; propertiBox->color[2] = b;
        } else propertiBox->RenderBorder(false);
        w3dEngine::Translatef(-dx, RowHeightGS + gapGS, 0);
    }
};

void PropBool::RenderPropertiValue(Card* propertiBox){
    if (value){
        int cb = CheckBoxWidth();
        // el value-origin esta en colLabel+borderGS (NO colLabel) -> el ✔ se centra en el MISMO
        // borde izq que el box (width-PropColLabel-cb-bordersGS), corrido el borderGS del origen.
        float dx = (float)(width - PropColLabel - cb - bordersGS - borderGS - g_propKeyBtnCol);
        w3dEngine::Translatef(dx, 0, 0);
        // (la tilde va SIEMPRE del color que puso el grupo: apagada es la del fondo y no se ve. Un override se marca
        //  solo con el BORDE amarillo; la tilde amarilla aparecia en una casilla apagada y parecia tildada)
        RenderBitmapText(unicodeCheck, textAlign::center, cb);
        w3dEngine::Translatef(-dx, RowHeightGS + gapGS, 0);
    }
}

void PropBool::RenderPropertiLabel(Card* propertiBox){
    if (value){
        int cb = CheckBoxWidth();
        // la etiqueta termina (derecha-alineada) justo antes del checkbox y ocupa todo a su izquierda
        float dx = (float)(width - PropColLabel - cb - bordersGS - gapGS - g_propKeyBtnCol);
        int wLabel = width - cb - bordersGS - gapGS * 2 - g_propKeyBtnCol;
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapLabel(name, wLabel);
        w3dEngine::Translatef(-dx, RowHeightGS + gapGS, 0);
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
    return RowHeightGS + gapGS;
}