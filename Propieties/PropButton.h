#ifndef PROPBUTTON_H
#define PROPBUTTON_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/Button.h"

// fila-boton para los grupos de propiedades (ej: "New Material")
class PropButton : public PropertieBase {
    public:
        Button* button;
        bool conLabel; // true: el NOMBRE a la izquierda (columna de etiquetas) y el boton SOLO
                       // en la columna de valores (para los desplegables). false: boton a lo ancho.
        bool oculto; // true: no ocupa fila (material por defecto)
        bool gris;   // true: se dibuja atenuado y NO ejecuta su accion (ej: Render Animation sin animaciones)
        void (*action)(); // que hace el boton al activarse (o NULL)

        PropButton(const std::string& Name, int Icon = -1);
        ~PropButton();

        PropertyType GetType();
        int Resize(int w);
        int Avance() const; // paso de fila (con label: la grilla de renglones, como los inputs)
        bool EditPropertie(); // la accion del boton (por ahora: nada)
        bool Seleccionable() { return !oculto; }

        void RenderPropertiBox(Card* propertiBox);
        void RenderPropertiBoxBorder(Card* propertiBox);
        void RenderPropertiValue(Card* propertiBox);
        void RenderPropertiLabel(Card* propertiBox);
};

#endif
