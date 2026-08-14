#include "PropertieBase.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"
#include "WhiskUI/draw/icons.h"   // IconType::keyframe (rombo)

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

int g_propKeyBtnCol = 0;
void* g_kfFocoProp = 0;
int  (*W3dKeyframeEstado)(int prop, int comp) = 0;
void (*W3dKeyframeToggle)(int prop, int comp) = 0;

// el rombo de keyframe: verde LLENO (2 = hay key), verde tenue (1 = animado sin key
// aqui) o gris normal (0 = sin animacion). Se dibuja en el origen actual.
void W3dRenderRomboKey(int estado){
    const float* c = (estado == 2) ? ListaColores[static_cast<int>(ColorID::accent)]
                   : (estado == 1) ? NULL   // tenue: se arma abajo
                                   : ListaColores[static_cast<int>(ColorID::grisUI)];
    static float tenue[4];
    if (estado == 1){ const float* a = ListaColores[static_cast<int>(ColorID::accent)];
                      tenue[0]=a[0]*0.5f; tenue[1]=a[1]*0.5f; tenue[2]=a[2]*0.5f; tenue[3]=1.0f; c = tenue; }
    w3dEngine::Color4fv(c);
    W3dDrawStrip4(IconMesh, IconsUV[static_cast<size_t>(IconType::keyframe)]->uvs);
    w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]);
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