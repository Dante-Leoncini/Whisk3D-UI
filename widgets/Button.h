#ifndef BUTTON_H
#define BUTTON_H

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
#include "WhiskUI/widgets/card.h"
#include "WhiskUI/draw/icons.h"
#include "WhiskUI/theme/colores.h"
#include "WhiskUI/text/bitmapText.h"
#include "WhiskUI/widgets/TextField.h" // editField: el boton se vuelve un input de texto (rename in-place)

// Boton compartido (4 OS): tarjeta del color de las cards con borde
// oscuro (el borde es lo que lo hace "boton"), texto y un icono opcional
// a la izquierda. Hover: borde blanco y fondo un toque mas claro.
class Button {
    public:
        std::string text;
        int icon;        // IconType o -1 (sin icono)
        bool adaptar;    // true: el ancho sigue al contenido
        bool desplegable; // true: abre un menu (logica; NO dibuja flecha por si solo)
        bool altoRenglon; // true: alto de FILA de propiedades (igual al box de los inputs)
        bool caretMenu;   // true: dibuja la flechita abajo-derecha (afordancia de dropdown). OPT-IN: apagado por
                          // defecto (en la barra del viewport se come mucho espacio); se prende donde no queda claro
        bool centrado;    // true: icono+texto centrados (OK/Cancel/etc)
        bool cuadrado;    // true: fuerza ancho = alto (botones de icono cuadrados, ej: transporte del timeline)
        bool visible;     // false: no se dibuja ni ocupa lugar en la barra
                          // (ej: el menu "Object" sin nada seleccionado)
        bool hover;
        bool focoMenu;   // menu enfocado con el teclado: borde+texto VERDE
        int width, height;
        int sx, sy;      // rect ABSOLUTO del ultimo render (hover/click)
        const float* tinte; // color de fondo opcional (NULL = gris normal);
                            // p.ej. accent para un boton "de accion" verde
        const float* colorTexto; // color del texto+icono (NULL = grisUI/hover)
        const float* colorBorde; // color del borde (NULL = fondo/hover); para
                                 // un boton "desactivado" se pone el del fondo
        int iconFlip;       // 0 nada, 1 flip horizontal, 2 flip vertical
        TextField* editField; // != NULL: el boton se dibuja como INPUT de texto (rename in-place). El
                              // texto/caret salen de aca; al confirmar/cancelar se vuelve a poner NULL.
        int rol;            // ROL estable del boton en la barra (se busca por rol, NO por indice:
                            // reordenar la barra no rompe el dispatch). -1 = sin rol.
        Card* card;

        Button(const std::string& Text, int Icon = -1, bool Adaptar = true);
        ~Button();

        void Resize(int maxW); // nunca mas ancho que el contenedor
        void Render();         // dibuja en el cursor actual (translate)
        bool Contains(int mx, int my) const; // contra el rect absoluto
};

#endif
