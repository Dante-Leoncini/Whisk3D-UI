#ifndef PROPERTIEBASE_H
#define PROPERTIEBASE_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <string>
#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "WhiskUI/core/UI.h"
#include "WhiskUI/draw/icons.h"
#include "WhiskUI/theme/colors.h"
#include "WhiskUI/widgets/card.h"
#include "WhiskUI/text/bitmapText.h"
#include "objects/Objects.h"
#include "w3dGraphics.h"   // AlteredColor (w3dEngine::Color4f)

// dialecto C++03 compartido
struct PropertyType {
    enum Enum {
        Base,
        Gap,
        Float,
        Int,
        Bool,
        List,
        SelectText,
        Color,
        Button,
        Text,      // caja de texto editable (PropText)
        ButtonRow  // varios botones en UNA fila (auto-ancho 50/50, 33/33/33, con gap)
    };
    Enum v;
    PropertyType(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

// donde ARRANCA la columna de valores del grupo en dibujo (las filas
// de ancho completo la usan para volver al borde izquierdo)
extern int PropColLabel;
extern int g_propKeyBtnCol;   // ancho de la columna del boton keyframe (0 si no hay); lo setea el grupo
extern void* g_kfFocusProp;   // la PropertieBase cuyo keyframe esta enfocado por TECLADO (borde blanco); NULL = ninguno

// HOOKS del boton-rombo de keyframe (canal de animacion de una propiedad). El editor
// los cablea (usan ActiveObj + CurrentFrame + las curvas). NULL sin editor.
//   state(prop,comp) -> 0 sin animacion / 1 animado sin key aqui / 2 hay key aqui
//                        3 CAMBIO SIN GUARDAR (verde): el valor de ahora no es el que dice la curva en este frame;
//                          un click lo guarda como key en el frame actual
//   toggle(prop,comp) -> pone/quita el key del canal en el frame actual
extern int  (*W3dKeyframeState)(int prop, int comp);
extern void (*W3dKeyframeToggle)(int prop, int comp);
// dibuja el rombo (IconType::keyframe) coloreado por el estado, en el origen actual
void W3dRenderDiamondKey(int state);

// Titulo de una tarjeta: [flecha abrir/cerrar] [icono opcional] texto.
// iconExtra = NULL -> no se dibuja y el texto no reserva su lugar (la tarjeta queda igual que siempre).
void CardTitle(GLfloat* icon, const std::string& text, int maxPixels = 1920, GLfloat* iconExtra = NULL);
// el AMARILLO de lo alterado (un override: ver PropertieBase::alterado)
inline void AlteredColor() { w3dEngine::Color4f(1.0f, 0.86f, 0.18f, 1.0f); }

class PropertieBase {
    public:
        std::string name;
        int width;

        PropertieBase(const std::string& Name);
        virtual ~PropertieBase();

        virtual PropertyType GetType();

        bool editing;
        // ALTERADO: el valor no es el ORIGINAL (un objeto que genera un proxy o una instancia, cambiado en ESTE proyecto: un
        // override). El numero y la casilla se ven en AMARILLO. Lo pone Properties en cada cuadro
        bool altered;

        virtual void button_up();
        virtual void button_down();
        virtual void button_left();
        virtual void button_right();
        virtual bool Cancel();

        virtual bool EditPropertie();
        // false = la navegacion con flechas la saltea (gaps, filas sin
        // valor enlazado: antes se "seleccionaban" filas que no se ven)
        virtual bool Selectable() { return true; }
        // canal de animacion de esta propiedad (el boton de KEYFRAME a la derecha): animProp = el canal del Core
        // (AnimPosition, AnimColor, AnimLightMisc...), animComp = AnimX/Y/Z; -1 = no animable. Una fila que no se ve
        // (sin valor enlazado, oculta) no tiene rombo: antes se dibujaba igual, encima de la fila de arriba
        int animProp, animComp;
        int AnimProp() { return Selectable() ? animProp : -1; }
        int AnimComp() { return animComp; }

        virtual void RenderPropertiBox(Card* propertiBox);
        virtual void RenderPropertiBoxBorder(Card* propertiBox);
        virtual void RenderPropertiValue(Card* propertiBox);
        virtual void RenderPropertiLabel(Card* propertiBox);
        virtual int Resize(int w);
};

#endif