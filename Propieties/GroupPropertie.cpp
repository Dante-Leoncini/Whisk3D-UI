#include "GroupPropertie.h"
#include "w3dGraphics.h"

bool GroupPropertie::NextSelect(){
    if (!open){
        selectIndex = -2;
        return true;
    }

    const int size = static_cast<int>(properties.size());

    while (true){
        selectIndex++;

        // llegamos al final
        if (selectIndex >= size){
            selectIndex = -2;
            return true;
        }

        //el idice es negativo
        if (selectIndex < 0){
            selectIndex = 0;
        }

        // saltar gaps y filas que no se dibujan (sin valor enlazado)
        if (properties[selectIndex]->GetType() == PropertyType::Gap ||
            !properties[selectIndex]->Selectable())
            continue;

        // encontrado válido
        return false;
    }
}

bool GroupPropertie::PrevSelect(){
    if (!open){
        selectIndex = -2;
        return true;
    }

    const int size = static_cast<int>(properties.size());

    while (true){
        selectIndex--;

        // llegamos al inicio
        if (selectIndex < -1){
            return true;
        }
        if (selectIndex < 0){
            return false;
        }

        // por seguridad (simetría con NextSelect)
        if (selectIndex >= size){
            selectIndex = size - 1;
        }

        // saltar gaps y filas que no se dibujan (sin valor enlazado)
        if (properties[selectIndex]->GetType() == PropertyType::Gap ||
            !properties[selectIndex]->Selectable())
            continue;

        // encontrado válido
        return false;
    }
}

void GroupPropertie::selectLastIndexProperty(){
    selectIndex = properties.size() - 1;
}

GroupPropertie::GroupPropertie(const std::string& Name): name(Name), icon(-1) {
    arrow = true;
    // C++03: defaults (name viene del init-list)
    open = true;
    visible = true;
    editing = false;
    valueWidth = 0.5f;
    reserveKeyBtn = false; keyBtnCol = 0;
    colLabel = 150;
    width = 300;
    height = 300;
    maxPixelsTitle = 1920;
    selectIndex = -2;

    card = new Card(NULL, 300, 300);
    propertiBox = new Card(NULL, 300, RowHeightGS);
    checkBox = new Card(NULL, RowHeightGS, RowHeightGS);
}

GroupPropertie::~GroupPropertie(){
    // los grupos de Properties viven toda la app; el redo-panel (RedoMeshPanel)
    // si crea/destruye su grupo, asi que liberamos lo que el grupo posee.
    for (size_t i = 0; i < properties.size(); ++i) delete properties[i];
    properties.clear();
    delete card;
    delete propertiBox;
    delete checkBox;
}

void GroupPropertie::Resize(int Width, int Height){
    width = Width;
    height = RowHeightGS + gapGS + borderGS;
    int widthProperties = width - bordersGS;

    // la columna de valores mide valueWidth; el resto es etiqueta. Se calcula ANTES de
    // resizear las hijas (y se publica en PropColLabel) para que las filas que ocupan
    // solo la columna de valores (los desplegables) le acierten al MISMO ancho que el box.
    int widthBox = (int)(widthProperties * valueWidth);
    // columna del boton de KEYFRAME (a la derecha de todo): un cuadrado + gap
    keyBtnCol = reserveKeyBtn ? (RowHeightGS + GlobalScale*2 + gapGS) : 0;
    widthBox -= keyBtnCol;
    colLabel = widthProperties - widthBox - keyBtnCol;
    PropColLabel    = colLabel;
    g_propKeyBtnCol = keyBtnCol;   // las filas con rombo (ej: un desplegable animable) se achican para dejarle lugar

    rowH.assign(properties.size(), 0);
    if (open){
        for (size_t i = 0; i < properties.size(); ++i){
            rowH[i] = properties[i]->Resize(width);
            height += rowH[i];
        }
    }
    card->Resize(Width, height);
    int heightProperties = height - bordersGS;
    maxPixelsTitle = widthProperties - IconSizeGS - gapGS;

    propertiBox->Resize(widthBox, RowHeightGS+GlobalScale*2);
    checkBox->Resize(RowHeightGS+GlobalScale*2, RowHeightGS+GlobalScale*2);
}

bool GroupPropertie::Cancel(){
    if (selectIndex < 0){
        editing = false;
        return false;
    }

    editing = properties[selectIndex]->Cancel();
    return editing;
}

bool GroupPropertie::EnterPropertieSelect(){
    if (selectIndex < 0){
        open = !open;
        editing = false;
        return false;
    }

    editing = properties[selectIndex]->EditPropertie();
    return editing;
}

void GroupPropertie::button_up(){
    if (selectIndex < 0){
        return;
    }
    properties[selectIndex]->button_up();
};

void GroupPropertie::button_down(){
    if (selectIndex < 0){
        return;
    }
    properties[selectIndex]->button_down();
};

void GroupPropertie::button_left(){
    if (selectIndex < 0){
        return;
    }
    properties[selectIndex]->button_left();
};

void GroupPropertie::button_right(){
    if (selectIndex < 0){
        return;
    }
    properties[selectIndex]->button_right();
};

int g_propClipTop = -1000000000, g_propClipBottom = 1000000000;
// la Y (local a la tarjeta) de la primera fila: el borde y el renglon del titulo (ver Render)
static int RowsTop() { return borderGS + RowHeightGS + gapGS; }
bool GroupPropertie::RowOutside(size_t i, int y) const {
    if (rowH.size() != properties.size() || i >= rowH.size()) return false;   // (sin medir: RenderRow, una lista cambiada)
    const int h = rowH[i];
    if (h <= 0) return false;   // (no ocupa lugar: su Render no dibuja nada)
    // (con una fila de margen: el borde de la caja y el rombo del keyframe asoman un pixel de la fila)
    return y + h < g_propClipTop - (int)RowHeightGS || y > g_propClipBottom + (int)RowHeightGS;
}

void GroupPropertie::RenderPropertiBox(){
    PropColLabel    = colLabel; // para las filas de ancho completo
    g_propKeyBtnCol = keyBtnCol;
    w3dEngine::Translatef((GLfloat)colLabel, -GlobalScale, 0);
    int y = RowsTop();
    for (size_t i = 0; i < properties.size(); y += RowAdvance(i), ++i){
        if (RowOutside(i, y)) { w3dEngine::Translatef(0, (GLfloat)rowH[i], 0); continue; }
        Card* cardToUse = propertiBox;

        if (properties[i]->GetType() == PropertyType::Bool){
            cardToUse = checkBox;
            // el relleno sale de CheckboxBoxColor (card.h), el MISMO que usan los menus: un solo lugar decide
            // como se ve un checkbox encendido/apagado en todo el editor
            bool on = (static_cast<PropBool*>(properties[i])->value && *static_cast<PropBool*>(properties[i])->value);
            if (on){
                const float* c = CheckboxBoxColor(true);
                w3dEngine::Color4f(c[0], c[1], c[2], 1.0f);
            }
        }
        else if (properties[i]->GetType() == PropertyType::Color){
            cardToUse = checkBox; // swatch CUADRADO (mismo tamaño que el checkbox), pegado a la derecha
            static_cast<PropColor*>(properties[i])->SetGlColor();
        }

        properties[i]->RenderPropertiBox(cardToUse);

        if (cardToUse == checkBox){
            SetColorID(ColorID::background, 1.0f);
        }

        // BOTON de keyframe COMPLETO (fondo + borde + rombo) a la DERECHA de todo.
        // OJO: RenderPropertiBox ya avanzo la Y una fila -> back-translate para
        // volver a la fila de ESTA propiedad. El rombo se centra en el cuadrado.
        if (reserveKeyBtn && properties[i]->AnimProp() >= 0){
            int est = W3dKeyframeState ? W3dKeyframeState(properties[i]->AnimProp(), properties[i]->AnimComp()) : 0;
            int btnSide = checkBox->width;                 // el cuadrado (= el del checkbox)
            bool focus = (g_kfFocusProp == (void*)properties[i]);   // enfocado por TECLADO
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(propertiBox->width + gapGS), (GLfloat)(-(RowHeightGS + gapGS)), 0);
            // SIN recuadro por defecto (solo el icono). El BORDE BLANCO se dibuja
            // unicamente cuando se navega con las flechas (Symbian/Nokia): asi sabes
            // que estas pisando el keyframe y con OK/Enter lo pones/sacas.
            if (focus){ SetColorID(ColorID::white, 1.0f); checkBox->RenderBorder(false); }
            w3dEngine::Translatef((GLfloat)((btnSide - IconSizeGS)/2), (GLfloat)((btnSide - IconSizeGS)/2), 0);
            W3dRenderDiamondKey(est);
            w3dEngine::PopMatrix();
            SetColorID(ColorID::background, 1.0f);
        }

        // hover: se resalta el BORDE del checkbox (no el relleno)
        if (this == PropHoverGroup && (int)i == PropHoverRow &&
            selectIndex != (int)i &&
            properties[i]->GetType() == PropertyType::Bool){
            SetColorID(ColorID::white, 1.0f);
            properties[i]->RenderPropertiBoxBorder(cardToUse);
            SetColorID(ColorID::background, 1.0f);
        }

        //dibujado del borde (NO en el valor si el foco esta en el KEYFRAME de esta fila: estamos en el rombo)
        if (selectIndex == i && g_kfFocusProp != (void*)properties[i]){
            if (editing || properties[i]->editing){
                SetColorID(ColorID::accent, 1.0f);
            }
            else {
                SetColorID(ColorID::white, 1.0f);
            }
            properties[i]->RenderPropertiBoxBorder(cardToUse);
            SetColorID(ColorID::background, 1.0f);
        }
    }
    w3dEngine::Translatef((GLfloat)-colLabel, GlobalScale, 0);
};

void GroupPropertie::RenderPropertiValue(){
    PropColLabel = colLabel; g_propKeyBtnCol = keyBtnCol;
    w3dEngine::Translatef((GLfloat)(colLabel+borderGS), 0, 0);
    int y = RowsTop();
    for (size_t i = 0; i < properties.size(); y += RowAdvance(i), ++i){
        if (RowOutside(i, y)) { w3dEngine::Translatef(0, (GLfloat)rowH[i], 0); continue; }
        //si es checkbox
        if (properties[i]->GetType() == PropertyType::Bool){
            SetColorID(ColorID::background, 1.0f);
        }
        else if (selectIndex == i && g_kfFocusProp != (void*)properties[i]){
            // con el foco en el KEYFRAME de esta fila NO se blanquea el numero (estamos en el rombo, no editando)
            if (editing){
                SetColorID(ColorID::accent, 1.0f);
            }
            else {
                SetColorID(ColorID::white, 1.0f);
            }
        }
        else if (this == PropHoverGroup && (int)i == PropHoverRow){
            SetColorID(ColorID::white, 1.0f);
        }
        else {
            SetColorID(ColorID::grayUI, 1.0f);
        }
        properties[i]->RenderPropertiValue(propertiBox);
    }
    w3dEngine::Translatef((GLfloat)(-colLabel-borderGS), GlobalScale, 0);
};

void GroupPropertie::RenderPropertiLabel(){
    PropColLabel = colLabel;
    w3dEngine::Translatef((GLfloat)(colLabel - gapGS), 0, 0);
    int y = RowsTop();
    for (size_t i = 0; i < properties.size(); y += RowAdvance(i), ++i){
        if (RowOutside(i, y)) { w3dEngine::Translatef(0, (GLfloat)rowH[i], 0); continue; }
        if (selectIndex == i){
            SetColorID(ColorID::accent, 1.0f);
        }
        else if (this == PropHoverGroup && (int)i == PropHoverRow){
            // hover: el texto se resalta en blanco
            SetColorID(ColorID::white, 1.0f);
        }
        else {
            SetColorID(ColorID::grayUI, 1.0f);
        }
        properties[i]->RenderPropertiLabel(propertiBox);
    }
    w3dEngine::Translatef((GLfloat)(-colLabel + gapGS), GlobalScale, 0);
};

void GroupPropertie::Render(){
    if (!visible) return;

    SetColorID(ColorID::gray, 1.0f);

    card->RenderObject(false);

    w3dEngine::Translatef(borderGS, borderGS, 0);

    if (selectIndex == -1){
        SetColorID(ColorID::accent, 1.0f);
    }
    else {
        SetColorID(ColorID::grayUI, 1.0f);
    }

    CardTitle(
        !arrow ? NULL :
        open
            ? IconsUV[static_cast<size_t>(IconType::arrow)]->uvs
            : IconsUV[static_cast<size_t>(IconType::arrowRight)]->uvs,
        name, maxPixelsTitle,
        (icon >= 0) ? IconsUV[static_cast<size_t>(icon)]->uvs : NULL
    );

    w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);

    if (open){
        SetColorID(ColorID::background, 1.0f);

        w3dEngine::PushMatrix();
        RenderPropertiBox();
        w3dEngine::PopMatrix();

        w3dEngine::PushMatrix();
        RenderPropertiValue();
        w3dEngine::PopMatrix();

        RenderPropertiLabel();
    }

    w3dEngine::Translatef(-borderGS, borderGS, 0);
}

int GroupPropertie::IndexOf(const PropertieBase* p) const {
    for (size_t i = 0; i < properties.size(); ++i) if (properties[i] == p) return (int)i;
    return -1;
}

// Las tres pasadas de Render (cajas, valores, nombres) con la lista reducida a ESA fila, corridas hasta donde cae: se ve
// igual que dentro del grupo (la seleccion y el hover de la fila se respetan)
void GroupPropertie::RenderRow(int idx){
    if (!visible || !open || idx < 0 || idx >= (int)properties.size()) return;
    int dy = 0;
    for (int j = 0; j < idx; ++j) dy += properties[(size_t)j]->Resize(width);
    std::vector<PropertieBase*> all; all.swap(properties);
    properties.push_back(all[(size_t)idx]);
    const int selPrev = selectIndex, hoverPrev = PropHoverRow;
    selectIndex = (selPrev == idx) ? 0 : -2;                         // (-1 seria el titulo)
    if (PropHoverGroup == this) PropHoverRow = (hoverPrev == idx) ? 0 : -1;
    w3dEngine::PushMatrix();
    w3dEngine::Translatef((float)borderGS, (float)(borderGS + RowHeightGS + gapGS + dy), 0);
    SetColorID(ColorID::background, 1.0f);
    w3dEngine::PushMatrix(); RenderPropertiBox();   w3dEngine::PopMatrix();
    w3dEngine::PushMatrix(); RenderPropertiValue(); w3dEngine::PopMatrix();
    RenderPropertiLabel();
    w3dEngine::PopMatrix();
    properties.swap(all);
    selectIndex = selPrev; PropHoverRow = hoverPrev;
}

GroupPropertie* PropHoverGroup = NULL;
int PropHoverRow = -1;
int PropColLabel = 150;

std::vector<GroupPropertie*> GroupProperties;