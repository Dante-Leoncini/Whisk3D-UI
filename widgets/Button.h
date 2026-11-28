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
#include "WhiskUI/theme/colors.h"
#include "WhiskUI/text/bitmapText.h"
#include "WhiskUI/widgets/TextField.h" // editField: el boton se vuelve un input de texto (rename in-place)

// Boton compartido (4 OS): tarjeta del color de las cards con borde
// oscuro (el borde es lo que lo hace "boton"), texto y un icono opcional
// a la izquierda. Hover: borde blanco y fondo un toque mas claro.
class Button {
    public:
        std::string text;
        int icon;      // IconType o -1 (sin icono)
        bool adapt;    // true: el ancho sigue al contenido
        bool dropdown;  // true: abre un menu (logica; NO dibuja flecha por si solo)
        bool rowHeight; // true: alto de FILA de propiedades (igual al box de los inputs)
        bool caretMenu; // true: dibuja la flechita abajo-derecha (afordancia de dropdown). OPT-IN: apagado por
                          // defecto (en la barra del viewport se come mucho espacio); se prende donde no queda claro
        bool centered;    // true: icono+texto centrados (OK/Cancel/etc)
        bool square;      // true: fuerza ancho = alto (botones de icono cuadrados, ej: transporte del timeline)
        bool visible;     // false: no se dibuja ni ocupa lugar en la barra
                          // (ej: el menu "Object" sin nada seleccionado)
        bool hover;
        bool separator;  // toolbar: una LINEA VERTICAL antes del boton (arranca otro grupo, estilo Inkscape)
        bool joinNext;   // barra: el que sigue va PEGADO, sin hueco (varios botones que se ven como uno alargado:
                         // el selector vertice / arista / cara)
        bool focusMenu;  // menu enfocado con el teclado: borde+texto VERDE
        int width, height;
        int sx, sy;      // rect ABSOLUTO del ultimo render (hover/click)
        const float* tint; // color de fondo opcional (NULL = gris normal);
                            // p.ej. accent para un boton "de accion" verde
        const float* textColor;   // color del texto+icono (NULL = grayUI/hover)
        const float* borderColor; // color del borde (NULL = fondo/hover); para
                                 // un boton "desactivado" se pone el del fondo
        float alpha;        // opacidad del CARD del boton (fondo + borde). 1 = opaco (default,
                            // los paneles no cambian). En la barra del viewport 3D se le pasa el
                            // barAlpha de la barra para que los botones dejen ver la escena
                            // igual que el fondo sobre el que estan.
        int iconFlip;       // 0 nada, 1 flip horizontal, 2 flip vertical
        const float* fill; // != NULL: un CUADRADO de ese color adentro del boton (el color base / la emision de un
                              // material: se abre su selector de color)
        float bar;        // >= 0: el boton es un NUMERO de 0 a 1: una barra de ese largo (accent) y el valor en el medio
                            // (se arrastra de izquierda a derecha; un click lo deja escribir). < 0 = boton comun
        TextField* editField; // != NULL: el boton se dibuja como INPUT de texto (rename in-place). El
                              // texto/caret salen de aca; al confirmar/cancelar se vuelve a poner NULL.
        int role;            // ROL estable del boton en la barra (se busca por rol, NO por indice:
                            // reordenar la barra no rompe el dispatch). -1 = sin rol.
        std::string help;  // el CARTELITO de ayuda (en INGLES, la UI lo pasa por T()): lo que es un boton de solo icono.
                            // Aparece al dejar el mouse encima un momento y se va solo (W3dHelpRender). "" = sin cartel
        Card* card;

        Button(const std::string& Text, int Icon = -1, bool Adapt = true);
        ~Button();

        void Resize(int maxW); // nunca mas ancho que el contenedor
        void Render();         // dibuja en el cursor actual (translate)
        bool Contains(int mx, int my) const; // contra el rect absoluto
};

#endif
