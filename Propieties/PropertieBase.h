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
#include "WhiskUI/theme/colores.h"
#include "WhiskUI/widgets/card.h"
#include "WhiskUI/text/bitmapText.h"
#include "objects/Objects.h"

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
extern int PropColEtiqueta;
extern int g_propKeyBtnCol;   // ancho de la columna del boton keyframe (0 si no hay); lo setea el grupo
extern void* g_kfFocoProp;    // la PropertieBase cuyo keyframe esta enfocado por TECLADO (borde blanco); NULL = ninguno

// HOOKS del boton-rombo de keyframe (canal de animacion de una propiedad). El editor
// los cablea (usan ObjActivo + CurrentFrame + las curvas). NULL sin editor.
//   estado(prop,comp) -> 0 sin animacion / 1 animado sin key aqui / 2 hay key aqui
//   toggle(prop,comp) -> pone/quita el key del canal en el frame actual
extern int  (*W3dKeyframeEstado)(int prop, int comp);
extern void (*W3dKeyframeToggle)(int prop, int comp);
// dibuja el rombo (IconType::keyframe) coloreado por el estado, en el origen actual
void W3dRenderRomboKey(int estado);

// Titulo de una tarjeta: [flecha abrir/cerrar] [icono opcional] texto.
// iconExtra = NULL -> no se dibuja y el texto no reserva su lugar (la tarjeta queda igual que siempre).
void CardTitulo(GLfloat* icon, const std::string& texto, int maxPixels = 1920, GLfloat* iconExtra = NULL);

class PropertieBase {
    public:
        std::string name;
        int width;

        PropertieBase(const std::string& Name);
        virtual ~PropertieBase();

        virtual PropertyType GetType();

        bool editando;

        virtual void button_up();
        virtual void button_down();
        virtual void button_left();
        virtual void button_right();
        virtual bool Cancel();

        virtual bool EditPropertie();
        // false = la navegacion con flechas la saltea (gaps, filas sin
        // valor enlazado: antes se "seleccionaban" filas que no se ven)
        virtual bool Seleccionable() { return true; }
        // canal de animacion de esta propiedad (para el boton de KEYFRAME a la
        // derecha). -1 = no animable. Lo overridean PropFloat/PropBool.
        virtual int AnimProp() { return -1; }
        virtual int AnimComp() { return 0; }

        virtual void RenderPropertiBox(Card* propertiBox);
        virtual void RenderPropertiBoxBorder(Card* propertiBox);
        virtual void RenderPropertiValue(Card* propertiBox);
        virtual void RenderPropertiLabel(Card* propertiBox);
        virtual int Resize(int w);
};

#endif