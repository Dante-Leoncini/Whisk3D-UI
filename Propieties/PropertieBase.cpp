#include "PropertieBase.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"
#include "WhiskUI/draw/icons.h"   // IconType::keyframe (rombo)

void CardTitle(GLfloat* icon, const std::string& text, int maxPixels, GLfloat* iconExtra){
    if (!icon){   // sin flecha (ventana flotante, no una tarjeta desplegable): el titulo arranca en el borde
        w3dEngine::PushMatrix();
        if (iconExtra){ W3dDrawStrip4(IconMesh, iconExtra); w3dEngine::Translatef(IconSizeGS + gapGS, 0, 0); maxPixels -= IconSizeGS + gapGS; }
        RenderBitmapText(text, textAlign::left, maxPixels);
        w3dEngine::PopMatrix();
        return;
    }
    //icono de la coleccion
    W3dDrawStrip4(IconMesh, icon); // (shim: strip+short no dibujan en N95)

    w3dEngine::PushMatrix();
    w3dEngine::Translatef(IconSizeGS + gapGS, 0, 0);
    if (iconExtra){                       // icono propio de la tarjeta (ej: el rombo de "Keyframe")
        W3dDrawStrip4(IconMesh, iconExtra);
        w3dEngine::Translatef(IconSizeGS + gapGS, 0, 0);
        maxPixels -= IconSizeGS + gapGS;  // el texto tiene menos lugar: se trunca antes
    }
    RenderBitmapText(text, textAlign::left, maxPixels);
    w3dEngine::PopMatrix();
}

int g_propKeyBtnCol = 0;
void* g_kfFocusProp = 0;
int  (*W3dKeyframeState)(int prop, int comp) = 0;
void (*W3dKeyframeToggle)(int prop, int comp) = 0;

// el rombo de keyframe: verde LLENO (2 = hay key), verde tenue (1 = animado sin key
// aqui) o gris normal (0 = sin animacion). Se dibuja en el origen actual.
void W3dRenderDiamondKey(int state){
    const float* c = (state == 2) ? ColorList[static_cast<int>(ColorID::accent)]
                   : (state == 1) ? NULL   // tenue: se arma abajo
                                   : ColorList[static_cast<int>(ColorID::grayUI)];
    static float dim[4];
    if (state == 1){ const float* a = ColorList[static_cast<int>(ColorID::accent)];
                      dim[0]=a[0]*0.5f; dim[1]=a[1]*0.5f; dim[2]=a[2]*0.5f; dim[3]=1.0f; c = dim; }
    static const float green[4] = { 0.35f, 0.85f, 0.35f, 1.0f };   // 3: cambio sin guardar
    if (state == 3) c = green;
    w3dEngine::Color4fv(c);
    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::keyframe)]->uvs);
    w3dEngine::Color4fv(ColorList[static_cast<int>(ColorID::white)]);
}

PropertieBase::PropertieBase(const std::string& Name): name(Name){
    // C++03: defaults (name viene del init-list; "?" era solo el default)
    width = 300;
    editing = false;
    altered = false;
    animProp = -1; animComp = 0;   // sin rombo por defecto
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