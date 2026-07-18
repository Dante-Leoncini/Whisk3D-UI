#ifndef GROUPPROPERTIE_H
#define GROUPPROPERTIE_H

#include <vector>
#include <string>

#include "WhiskUI/widgets/card.h"
#ifndef W3D_SYMBIAN
#include "variables.h"
#endif
#include "PropertieBase.h"
#include "PropList.h"
#include "PropBool.h"
#include "PropColor.h"

#include "WhiskUI/theme/colores.h"

class GroupPropertie {
    public:
        // ancho de la columna de valores (fraccion del ancho del grupo);
        // Transform usa 0.5, Mesh Parts 0.30 (mas lugar para etiquetas)
        float anchoValores;
        int colEtiqueta; // donde empieza la columna de valores
        std::string name;
        int icono;       // IconType de la tarjeta, o -1 = sin icono (default). Va entre la flecha y el titulo.
        Card* card;
        Card* propertiBox;
        Card* checkBox;
        bool open;
        bool visible;
        bool editando;

        int width;
        int height;
        int maxPixelsTitle;
        std::vector<PropertieBase*> properties;

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
        void RenderPropertiBox();
        void RenderPropertiValue();
        void RenderPropertiLabel();
        void Resize(int Width, int Height);

        GroupPropertie(const std::string& Name);
        ~GroupPropertie();
};

extern std::vector<GroupPropertie*> GroupProperties;

// la fila bajo el mouse (hover): texto blanco / borde del checkbox
extern GroupPropertie* PropHoverGroup;
extern int PropHoverFila;

// donde ARRANCA la columna de valores del grupo que se esta dibujando
// (las filas la usan para volver al borde izquierdo)
extern int PropColEtiqueta;

#endif