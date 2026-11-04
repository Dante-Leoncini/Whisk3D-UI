#ifndef PROPBUTTON_H
#define PROPBUTTON_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/Button.h"

// fila-boton para los grupos de propiedades (ej: "New Material")
class PropButton : public PropertieBase {
    public:
        Button* button;
        bool conLabel; // true (los desplegables): el NOMBRE a la izquierda, del ancho de su texto, y el boton en TODO
                       // el resto del renglon. false: boton a lo ancho.
        int colBoton;  // (conLabel) donde empieza el boton, desde el borde izquierdo del renglon (lo calcula Resize)
        int ColBoton() const { return conLabel ? colBoton : 0; }
        bool oculto; // true: no ocupa fila (material por defecto)
        bool gris;   // true: se dibuja atenuado y NO ejecuta su accion (ej: Render Animation sin animaciones)
        void (*action)(); // que hace el boton al activarse (o NULL)
        // canal ANIMABLE (rombo de keyframe a la derecha, como PropFloat/PropBool). -1 = no animable. Ej: el
        // selector de la "Camara activa" (AnimCamActiva): el rombo guarda el corte en el frame actual.
        int animProp, animComp;
        // (con label) BOTONES CHICOS de solo icono A LA DERECHA del boton: los de un MAPA del material (numero / textura
        // y su capa UV). 'slotsExtras' reserva el lugar de varios aunque la fila tenga menos: asi el ULTIMO (la UV) queda
        // alineado en todas las filas. Cada uno con su accion; el click lo despacha Properties (ExtraEnX)
        Button* extras[2];
        void (*accionExtra[2])();
        int nExtras;
        int slotsExtras;   // los lugares reservados a la derecha (0 = los que tiene)
        int colExtras;     // donde empieza la zona de los extras (desde el borde izquierdo del renglon)
        void AgregarExtra(int icono, void (*accion)(), const char* ayuda);
        int ExtraX(int i) const;      // donde va el extra i (desde el borde izquierdo del renglon)
        int ExtraEnX(int lx) const;   // el extra en esa x (desde el borde izquierdo del renglon) o -1
        // (con label) un CUADRADITO antes del boton grande (el color base de un material, al lado de su textura)
        Button* prefijo;
        void (*accionPrefijo)();
        int colPrefijo;
        void AgregarPrefijo(void (*accion)(), const char* ayuda);
        int labelChars;    // > 0: la columna del nombre mide esto (las filas de los mapas quedan alineadas)
        int AnimProp() { return oculto ? -1 : animProp; }
        int AnimComp() { return animComp; }

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
