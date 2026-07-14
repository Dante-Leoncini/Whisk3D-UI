#ifndef PROPBUTTON_H
#define PROPBUTTON_H

#include "PropertieBase.h"
#include "WhiskUI/Button.h"

// fila-boton para los grupos de propiedades (ej: "New Material")
class PropButton : public PropertieBase {
    public:
        Button* button;
        bool oculto; // true: no ocupa fila (material por defecto)
        bool gris;   // true: se dibuja atenuado y NO ejecuta su accion (ej: Render Animation sin animaciones)
        void (*action)(); // que hace el boton al activarse (o NULL)

        PropButton(const std::string& Name, int Icon = -1);
        ~PropButton();

        PropertyType GetType();
        int Resize(int w);
        bool EditPropertie(); // la accion del boton (por ahora: nada)
        bool Seleccionable() { return !oculto; }

        void RenderPropertiBox(Card* propertiBox);
        void RenderPropertiBoxBorder(Card* propertiBox);
        void RenderPropertiValue(Card* propertiBox);
        void RenderPropertiLabel(Card* propertiBox);
};

#endif
