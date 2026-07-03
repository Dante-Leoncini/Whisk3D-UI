#ifndef TAB_H
#define TAB_H

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
#include "WhiskUI/card.h"
#include "WhiskUI/rectangle.h"
#include "WhiskUI/icons.h"
#include "WhiskUI/colores.h"
#include "WhiskUI/bitmapText.h"

// Pestania compartida (4 OS): como un boton pero SIN el borde inferior,
// asi se funde con el contenido de abajo. Puede tener texto o icono.
// Se usa en el selector de color (RGB/HSV/Hex) y en la barra de
// propiedades (pestanias con distintas cosas).
class Tab {
    public:
        std::string text;
        int icon;       // IconType o -1
        bool activa;    // la pestania seleccionada (texto/icono blanco)
        bool foco;      // enfocada con las flechas (texto/icono VERDE, sin mouse)
        bool visible;   // false: no se dibuja ni ocupa lugar (ej: Mesh sin mesh)
        bool hover;
        int width, height;
        int sx, sy;     // rect ABSOLUTO del ultimo render (hover/click)
        Card* card;
        Rec2D* rect;    // tapa el borde inferior

        Tab(const std::string& Text, int Icon = -1);
        ~Tab();

        void Resize(int maxW);
        void Render(); // dibuja en el cursor actual (translate)
        bool Contains(int mx, int my) const;
};

#endif
