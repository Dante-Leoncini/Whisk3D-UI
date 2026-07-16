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

#include "WhiskUI/UI.h"
#include "WhiskUI/icons.h"
#include "WhiskUI/colores.h"
#include "WhiskUI/card.h"
#include "WhiskUI/bitmapText.h"
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

        virtual void RenderPropertiBox(Card* propertiBox);
        virtual void RenderPropertiBoxBorder(Card* propertiBox);
        virtual void RenderPropertiValue(Card* propertiBox);
        virtual void RenderPropertiLabel(Card* propertiBox);
        virtual int Resize(int w);
};

#endif