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
            !properties[selectIndex]->Seleccionable())
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
            !properties[selectIndex]->Seleccionable())
            continue;

        // encontrado válido
        return false;
    }
}

void GroupPropertie::selectLastIndexProperty(){
    selectIndex = properties.size() - 1;
}

GroupPropertie::GroupPropertie(const std::string& Name): name(Name), icono(-1) {
    // C++03: defaults (name viene del init-list)
    open = true;
    visible = true;
    editando = false;
    anchoValores = 0.5f;
    colEtiqueta = 150;
    width = 300;
    height = 300;
    maxPixelsTitle = 1920;
    selectIndex = -2;

    card = new Card(NULL, 300, 300);
    propertiBox = new Card(NULL, 300, RenglonHeightGS);
    checkBox = new Card(NULL, RenglonHeightGS, RenglonHeightGS);
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
    height = RenglonHeightGS + gapGS + borderGS;
    int widthProperties = width - bordersGS;

    if (open){
        for (size_t i = 0; i < properties.size(); ++i){
            height += properties[i]->Resize(width);
        }
    }
    card->Resize(Width, height);
    int heightProperties = height - bordersGS;
    maxPixelsTitle = widthProperties - IconSizeGS - gapGS;

    // la columna de valores mide anchoValores; el resto es etiqueta
    int anchoBox = (int)(widthProperties * anchoValores);
    colEtiqueta = widthProperties - anchoBox;
    propertiBox->Resize(anchoBox, RenglonHeightGS+GlobalScale*2);
    checkBox->Resize(RenglonHeightGS+GlobalScale*2, RenglonHeightGS+GlobalScale*2);
}

bool GroupPropertie::Cancel(){
    if (selectIndex < 0){
        editando = false;
        return false;
    }

    editando = properties[selectIndex]->Cancel();
    return editando;
}

bool GroupPropertie::EnterPropertieSelect(){
    if (selectIndex < 0){
        open = !open;
        editando = false;
        return false;
    }

    editando = properties[selectIndex]->EditPropertie();
    return editando;
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

void GroupPropertie::RenderPropertiBox(){
    PropColEtiqueta = colEtiqueta; // para las filas de ancho completo
    w3dEngine::Translatef((GLfloat)colEtiqueta, -GlobalScale, 0);
    for (size_t i = 0; i < properties.size(); ++i){
        Card* cardToUse = propertiBox;

        if (properties[i]->GetType() == PropertyType::Bool){
            cardToUse = checkBox;
            // el relleno sale de CheckboxColorCaja (card.h), el MISMO que usan los menus: un solo lugar decide
            // como se ve un checkbox encendido/apagado en todo el editor
            bool on = (static_cast<PropBool*>(properties[i])->value && *static_cast<PropBool*>(properties[i])->value);
            if (on){
                const float* c = CheckboxColorCaja(true);
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

        // hover: se resalta el BORDE del checkbox (no el relleno)
        if (this == PropHoverGroup && (int)i == PropHoverFila &&
            selectIndex != (int)i &&
            properties[i]->GetType() == PropertyType::Bool){
            SetColorID(ColorID::blanco, 1.0f);
            properties[i]->RenderPropertiBoxBorder(cardToUse);
            SetColorID(ColorID::background, 1.0f);
        }

        //dibujado del borde
        if (selectIndex == i){
            if (editando || properties[i]->editando){
                SetColorID(ColorID::accent, 1.0f);
            }
            else {
                SetColorID(ColorID::blanco, 1.0f);
            }
            properties[i]->RenderPropertiBoxBorder(cardToUse);
            SetColorID(ColorID::background, 1.0f);
        }
    }
    w3dEngine::Translatef((GLfloat)-colEtiqueta, GlobalScale, 0);
};

void GroupPropertie::RenderPropertiValue(){
    PropColEtiqueta = colEtiqueta;
    w3dEngine::Translatef((GLfloat)(colEtiqueta+borderGS), 0, 0);
    for (size_t i = 0; i < properties.size(); ++i){
        //si es checkbox
        if (properties[i]->GetType() == PropertyType::Bool){
            SetColorID(ColorID::background, 1.0f);
        }
        else if (selectIndex == i){
            if (editando){
                SetColorID(ColorID::accent, 1.0f);
            }
            else {
                SetColorID(ColorID::blanco, 1.0f);
            }
        }
        else if (this == PropHoverGroup && (int)i == PropHoverFila){
            SetColorID(ColorID::blanco, 1.0f);
        }
        else {
            SetColorID(ColorID::grisUI, 1.0f);
        }
        properties[i]->RenderPropertiValue(propertiBox);
    }
    w3dEngine::Translatef((GLfloat)(-colEtiqueta-borderGS), GlobalScale, 0);
};

void GroupPropertie::RenderPropertiLabel(){
    PropColEtiqueta = colEtiqueta;
    w3dEngine::Translatef((GLfloat)(colEtiqueta - gapGS), 0, 0);
    for (size_t i = 0; i < properties.size(); ++i){
        if (selectIndex == i){
            SetColorID(ColorID::accent, 1.0f);
        }
        else if (this == PropHoverGroup && (int)i == PropHoverFila){
            // hover: el texto se resalta en blanco
            SetColorID(ColorID::blanco, 1.0f);
        }
        else {
            SetColorID(ColorID::grisUI, 1.0f);
        }
        properties[i]->RenderPropertiLabel(propertiBox);
    }
    w3dEngine::Translatef((GLfloat)(-colEtiqueta + gapGS), GlobalScale, 0);
};

void GroupPropertie::Render(){
    if (!visible) return;

    SetColorID(ColorID::gris, 1.0f);

    card->RenderObject(false);

    w3dEngine::Translatef(borderGS, borderGS, 0);

    if (selectIndex == -1){
        SetColorID(ColorID::accent, 1.0f);
    }
    else {
        SetColorID(ColorID::grisUI, 1.0f);
    }

    CardTitulo(
        open
            ? IconsUV[static_cast<size_t>(IconType::arrow)]->uvs
            : IconsUV[static_cast<size_t>(IconType::arrowRight)]->uvs,
        name, maxPixelsTitle,
        (icono >= 0) ? IconsUV[static_cast<size_t>(icono)]->uvs : NULL
    );

    w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);

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

GroupPropertie* PropHoverGroup = NULL;
int PropHoverFila = -1;
int PropColEtiqueta = 150;

std::vector<GroupPropertie*> GroupProperties;