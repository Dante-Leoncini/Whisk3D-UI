#include "PropertieBase.h"
#include "w3dGraphics.h"
#include "WhiskUI/glesdraw.h"

void CardTitulo(GLfloat* icon, const std::string& texto, int maxPixels, GLfloat* iconExtra){
    //icono de la coleccion
    W3dDrawStrip4(IconMesh, icon); // (shim: strip+short no dibujan en N95)

    w3dEngine::PushMatrix();
    w3dEngine::Translatef(IconSizeGS + gapGS, 0, 0);
    if (iconExtra){                       // icono propio de la tarjeta (ej: el rombo de "Keyframe")
        W3dDrawStrip4(IconMesh, iconExtra);
        w3dEngine::Translatef(IconSizeGS + gapGS, 0, 0);
        maxPixels -= IconSizeGS + gapGS;  // el texto tiene menos lugar: se trunca antes
    }
    RenderBitmapText(texto, textAlign::left, maxPixels);
    w3dEngine::PopMatrix();
}

PropertieBase::PropertieBase(const std::string& Name): name(Name){
    // C++03: defaults (name viene del init-list; "?" era solo el default)
    width = 300;
    editando = false;
}

PropertieBase::~PropertieBase() {}

PropertyType PropertieBase::GetType(){
    return PropertyType::Base;
}

void PropertieBase::button_up(){};
void PropertieBase::button_down(){};
void PropertieBase::button_left(){};
void PropertieBase::button_right(){};
bool PropertieBase::Cancel(){return false;};

void PropertieBase::RenderPropertiBox(Card* propertiBox){};
bool PropertieBase::EditPropertie(){return false;};
void PropertieBase::RenderPropertiBoxBorder(Card* propertiBox){};
void PropertieBase::RenderPropertiValue(Card* propertiBox){};
void PropertieBase::RenderPropertiLabel(Card* propertiBox){};
int PropertieBase::Resize(int w){return 0;};