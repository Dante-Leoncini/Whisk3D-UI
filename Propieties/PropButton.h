#ifndef PROPBUTTON_H
#define PROPBUTTON_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/Button.h"

// fila-boton para los grupos de propiedades (ej: "New Material")
class PropButton : public PropertieBase {
    public:
        Button* button;
        bool withLabel; // true (los desplegables): el NOMBRE a la izquierda, del ancho de su texto, y el boton en TODO
                       // el resto del renglon. false: boton a lo ancho.
        int colButton;  // (withLabel) donde empieza el boton, desde el borde izquierdo del renglon (lo calcula Resize)
        bool inColumn; // (withLabel) COMO UN INPUT: el nombre en la columna de etiquetas (alineado igual que el de un
                        // numero o un texto) y el boton en la columna de valores, del MISMO ancho que esos boxes. Para los
                        // desplegables que van entre inputs (Formato entre Nombre y Ancho): alineados con ellos
        int ColButton() const { return withLabel ? colButton : 0; }
        bool hidden; // true: no ocupa fila (material por defecto)
        bool gray;   // true: se dibuja atenuado y NO ejecuta su accion (ej: Render Animation sin animaciones)
        void (*action)(); // que hace el boton al activarse (o NULL)
        // (animable: el selector de la "Camara activa", AnimCamActive: el rombo guarda el corte en el frame actual)
        // (con label) BOTONES CHICOS de solo icono A LA DERECHA del boton: los de un MAPA del material (numero / textura
        // y su capa UV). 'slotsExtras' reserva el lugar de varios aunque la fila tenga menos: asi el ULTIMO (la UV) queda
        // alineado en todas las filas. Cada uno con su accion; el click lo despacha Properties (ExtraInX)
        Button* extras[2];
        void (*actionExtra[2])();
        int nExtras;
        int slotsExtras;   // los lugares reservados a la derecha (0 = los que tiene)
        int colExtras;     // donde empieza la zona de los extras (desde el borde izquierdo del renglon)
        void AddExtra(int icon, void (*newAction)(), const char* help);
        int ExtraX(int i) const;      // donde va el extra i (desde el borde izquierdo del renglon)
        int ExtraInX(int lx) const;   // el extra en esa x (desde el borde izquierdo del renglon) o -1
        // (con label) un CUADRADITO antes del boton grande (el color base de un material, al lado de su textura)
        Button* prefix;
        void (*actionPrefix)();
        int colPrefix;
        void AddPrefix(void (*newAction)(), const char* help);
        int labelChars;    // > 0: la columna del nombre mide esto (las filas de los mapas quedan alineadas)
        // FOCO DE TECLADO dentro de la fila (flechas izq/der del telefono): -2 = el cuadradito (prefijo), -1 = el boton
        // grande, 0..nExtras-1 = un boton chico. Asi sin mouse se llega al color, al numero / textura y a la capa UV
        int focusKb;
        bool KbMove(int dir);        // -1 / +1: al anterior / siguiente VISIBLE. false = no hay mas (la fila no lo usa)

        PropButton(const std::string& Name, int Icon = -1);
        ~PropButton();

        PropertyType GetType();
        int Resize(int w);
        int Advance() const; // paso de fila (con label: la grilla de renglones, como los inputs)
        bool EditPropertie(); // la accion del boton (por ahora: nada)
        bool Selectable() { return !hidden; }

        void RenderPropertiBox(Card* propertiBox);
        void RenderPropertiBoxBorder(Card* propertiBox);
        void RenderPropertiValue(Card* propertiBox);
        void RenderPropertiLabel(Card* propertiBox);
};

#endif
