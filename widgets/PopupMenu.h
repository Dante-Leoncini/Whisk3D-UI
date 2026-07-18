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

#include "WhiskUI/core/UI.h"
#include "WhiskUI/widgets/card.h"
#include "WhiskUI/draw/icons.h"
#include "WhiskUI/theme/colores.h"
#include "WhiskUI/text/bitmapText.h"

class PopupMenu;

// una opcion del menu: icono opcional + gap + texto; puede abrir otro
// menu desplegable (submenu)
// ---------------------------------------------------------------------------------------------------------------
//  MENU DECLARATIVO. Un menu se describe como una TABLA de estos: el texto y la accion del item JUNTOS, en una sola
//  linea. Se acabo el id magico apuntando a un switch lejano (y el bug de "me olvide de conectarlo": aca no se puede
//  agregar un item sin su accion). PopupMenu::Construir(tabla) arma el menu; el id de cada item es su fila.
// ---------------------------------------------------------------------------------------------------------------
class PopupMenu;
struct MenuDef {
    const char* text;       // la CLAVE es el texto. Se traduce con el hook de abajo al construir.
    void      (*accion)();  // que hace el item. NULL = es submenu (ver 'submenu') o item padre sin accion.
    const char* atajo;      // hint del atajo ("Shift D"); NULL = sin atajo
    int         icon;       // 0 = SIN icono. Los reales via ICONO(x) (= IconType+1: deja el 0 libre como "ninguno").
    PopupMenu** submenu;    // NULL = item comun; si no, apunta al PUNTERO del submenu (que ya tiene que existir)
    bool*       checkbox;   // NULL = comun; si no, es un checkbox sobre *checkbox
};
#define ICONO(x) ((int)(x) + 1)   // para MenuDef.icon; sin ICONO(...) el campo queda 0 = sin icono

// Traduccion opcional de los textos de menu. El toolkit NO conoce el sistema de idiomas del editor: si la app
// instala este hook (con su T()), las tablas salen traducidas; si queda NULL, el texto va tal cual.
extern const char* (*W3dMenuTraducir)(const char*);

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
        void (*accion)();   // accion PROPIA del item (estilo declarativo, ver MenuDef). NULL = usa el id + el
                            // action(id) del menu (camino viejo). El que dispara PREFIERE esta si esta.
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
        int scroll;         // primera fila VISIBLE (>0 cuando el menu es mas alto que la pantalla -> scrollbar)
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
        // arma el menu desde una TABLA declarativa (id de cada item = su fila). n = cantidad de filas.
        // Reemplaza a la retahila de ->Agregar(...) + el switch de acciones: la accion vive en la propia fila.
        void Construir(const MenuDef* defs, int n);
        // dispara el item 'id': su accion PROPIA si tiene (declarativo), si no el action(id) del menu (viejo).
        void Ejecutar(int id);
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
        // SCROLL (menus mas altos que la pantalla, ej: armature con 129 clips):
        void Wheel(int dir);        // rueda: +1 arriba / -1 abajo, una fila (delega al submenu abierto)
        int  FilasVisibles() const; // cuantas filas de items entran en la ventana visible
        int  MaxScroll() const;     // scroll maximo (0 = entra todo, sin scrollbar)
        void AsegurarVisible();     // ajusta scroll para que selectIndex quede visible (nav por teclado)

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
