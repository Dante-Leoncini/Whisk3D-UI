#ifndef POPUPMENU_H
#define POPUPMENU_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <string>
#include <vector>
#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "WhiskUI/UI.h"
#include "WhiskUI/card.h"
#include "WhiskUI/icons.h"
#include "WhiskUI/colores.h"
#include "WhiskUI/bitmapText.h"

class PopupMenu;

// una opcion del menu: icono opcional + gap + texto; puede abrir otro
// menu desplegable (submenu)
class MenuItem {
    public:
        std::string text;
        int icon;           // IconType o -1 (sin icono)
        int id;             // lo que devuelve al seleccionarse
        PopupMenu* submenu; // NULL = opcion comun
        bool* checkbox;     // NULL = opcion comun; si no, muestra/togglea este
                            // bool (un check a la derecha) y NO cierra el menu
        bool* gris;         // NULL = nunca; si no, se ve GRIS (deshabilitada)
                            // cuando *gris es false (ej: depende de "overlays")
        std::string atajo;  // hint del atajo a la derecha, gris tenue ("Shift D",
                            // "Ctrl I", "X"...); "" = sin atajo. Solo visual.
        bool verde;         // true = opcion ACTIVA: icono + texto en VERDE accent
                            // (ej: el modo de pivot actual en el menu de pivot)
        float* valorFloat;  // NULL = opcion comun; si no, es un SLIDER (ej: size)
        float vmin, vmax;   // rango del slider (click en la barra setea el valor)

        MenuItem(const std::string& Text, int Id, int Icon = -1,
                 PopupMenu* Submenu = NULL, bool* Checkbox = NULL);
};

// menu desplegable compartido (4 OS). Highlight con hover/flechas,
// enter u OK selecciona, click afuera o alejarse mucho lo cierra.
class PopupMenu {
    public:
        std::vector<MenuItem*> items;
        std::string titulo; // cabecera opcional ("Set Parent To", etc)
        int x, y;           // posicion ABSOLUTA en pantalla
        int width, height;
        int reservaDerecha; // px reservados a la derecha (tildes/flecha submenu/slider): el label se
                            // trunca ANTES de esta columna (sino, al clampear el ancho a la pantalla,
                            // el texto tapaba las tildes y no se veia si estaban marcadas)
        int selectIndex;    // opcion resaltada (-1 = ninguna)
        bool abierto;
        PopupMenu* submenuAbierto;
        Card* card;
        Card* highlight;
        Card* sliderCard; // barra del slider (items float)

        // que hace el menu al elegirse una opcion (recibe el id); si es
        // NULL la seleccion no hace nada todavia
        void (*action)(int id);

        PopupMenu();
        ~PopupMenu();

        MenuItem* Agregar(const std::string& text, int id, int icon = -1,
                          PopupMenu* submenu = NULL);
        // opcion con checkbox a la derecha: muestra/togglea *estado (no cierra)
        MenuItem* AgregarCheck(const std::string& text, int id, bool* estado,
                               int icon = -1);
        // opcion SLIDER: barra a la derecha; click en ella setea *valor (no cierra)
        MenuItem* AgregarFloat(const std::string& text, int id, float* valor,
                               float vmin, float vmax, int icon = -1);
        void Limpiar(); // borra todas las opciones (menus dinamicos)
        void Resize();      // ancho/alto segun el contenido
        void Abrir(int sx, int sy, int pantallaW, int pantallaH);
        void Cerrar();
        bool Contains(int mx, int my) const;

        void Render();      // en ortho 2D de PANTALLA completa
        // abre/cierra el submenu segun la fila resaltada (selectIndex): SOLO por
        // hover de mouse (las flechas ya NO lo auto-abren; se abre con der/enter/ok)
        void SincronizarSubmenu();
        bool MouseMove(int mx, int my); // hover + auto-cierre por lejania
        int Click(int mx, int my);  // id elegido, -1 nada, -2 abrio submenu
        int Enter();                // idem Click sobre la opcion resaltada
        void button_up();
        void button_down();

        // navegacion por teclado (Symbian/keypad y PC):
        MenuItem* ItemActual();          // opcion resaltada o NULL
        void AjustarSlider(int dir);     // +1/-1: mueve el valor del slider resaltado
        bool AbrirSubmenuActual();       // abre el submenu de la opcion resaltada y
                                         // pre-selecciona su 1ra fila; false si no hay
};

extern PopupMenu* MenuAbierto; // el desplegable activo (NULL = ninguno)

// tamano de la pantalla para que Abrir() pueda clampear (lo setea cada
// plataforma al hacer resize)
extern int MenuPantallaW;
extern int MenuPantallaH;

#endif
