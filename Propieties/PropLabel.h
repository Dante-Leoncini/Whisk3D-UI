#ifndef PROPLABEL_H
#define PROPLABEL_H

#include "PropertieBase.h"
#include <vector>

// fila de TEXTO informativo (no seleccionable, no editable); se usa para
// el aviso del material por defecto. Con oculto = true no ocupa fila.
// wrap = true: texto MULTILINEA con salto de linea en los ESPACIOS (word-wrap) que se adapta al ancho
// disponible (Resize lo recalcula al achicar el area) -> ocupa tantas filas como haga falta para leerlo entero.
class PropLabel : public PropertieBase {
    public:
        bool oculto;
        bool wrap;                       // word-wrap multilinea (por espacios)
        std::vector<std::string> lineas; // lineas ya cortadas (las recalcula Resize en modo wrap)

        PropLabel(const std::string& Name, bool Wrap = false);

        bool Seleccionable() { return false; }
        int Resize(int w);

        void RenderPropertiBox(Card* propertiBox);
        void RenderPropertiValue(Card* propertiBox);
        void RenderPropertiLabel(Card* propertiBox);
};

#endif
