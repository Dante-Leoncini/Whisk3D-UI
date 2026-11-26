#ifndef GROUPPROPERTIE_H
#define GROUPPROPERTIE_H

#include <vector>
#include <string>

#include "WhiskUI/widgets/card.h"
#include "PropertieBase.h"
#include "PropList.h"
#include "PropBool.h"
#include "PropColor.h"

#include "WhiskUI/theme/colors.h"

class GroupPropertie {
    public:
        // ancho de la columna de valores (fraccion del ancho del grupo);
        // Transform usa 0.5, Mesh Parts 0.30 (mas lugar para etiquetas)
        float valueWidth;
        bool reserveKeyBtn;   // reserva una columna a la derecha para el boton de KEYFRAME (tarjeta Transform)
        int keyBtnCol;        // ancho de esa columna (0 si no se reserva)
        int colLabel; // donde empieza la columna de valores
        std::string name;
        int icon;       // IconType de la tarjeta, o -1 = sin icono (default). Va entre la flecha y el titulo.
        Card* card;
        Card* propertiBox;
        Card* checkBox;
        bool open;
        bool arrow;     // false = sin la flecha de plegar en el titulo (una ventana flotante, no una tarjeta del panel)
        bool visible;
        bool editing;

        int width;
        int height;
        int maxPixelsTitle;
        std::vector<PropertieBase*> properties;
        std::vector<int> rowH;   // el alto de cada fila, medido por Resize (0 = no ocupa lugar): el recorte por fila

        //seleccion
        int selectIndex;
        bool NextSelect();
        bool PrevSelect();
        void selectLastIndexProperty();
        bool EnterPropertieSelect();
        bool Cancel();

        void button_up();
        void button_down();
        void button_left();
        void button_right();

        void Render();
        // UNA sola fila (su caja, su valor y su nombre) en el lugar que ocupa dentro del grupo, sin la tarjeta: la que se
        // esta editando, opaca encima del grupo dibujado translucido (las tarjetas de la ultima operacion al arrastrar)
        void RenderRow(int idx);
        int  IndexOf(const PropertieBase* p) const;   // -1 = no es de este grupo
        void RenderPropertiBox();
        void RenderPropertiValue();
        void RenderPropertiLabel();
        void Resize(int Width, int Height);
        bool RowOutside(size_t i, int y) const;   // la fila i, que arranca en y (local a la tarjeta), no se ve
        int  RowAdvance(size_t i) const { return i < rowH.size() ? rowH[i] : 0; }   // lo que avanza en Y

        GroupPropertie(const std::string& Name);
        ~GroupPropertie();
};

extern std::vector<GroupPropertie*> GroupProperties;

// la fila bajo el mouse (hover): texto blanco / borde del checkbox
extern GroupPropertie* PropHoverGroup;
extern int PropHoverRow;

// donde ARRANCA la columna de valores del grupo que se esta dibujando
// (las filas la usan para volver al borde izquierdo)
extern int PropColLabel;

// EL RECORTE POR FILA: lo que se ve de la tarjeta que se esta dibujando, en Y relativa a su borde de arriba. El panel lo
// pone antes de su Render y lo vuelve a abrir despues (Properties::Render: de una tarjeta que entra a medias, como la de
// Render con el panel maximizado en un telefono, se dibujan solo las filas a la vista; las otras solo avanzan). Por
// defecto no recorta nada (las ventanas flotantes que usan tarjetas)
extern int g_propClipTop, g_propClipBottom;

#endif