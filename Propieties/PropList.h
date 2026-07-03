#ifndef PROPLIST_H
#define PROPLIST_H

#include "PropertieBase.h"
#include "WhiskUI/card.h"
#include "objects/Mesh.h"

class PropList : public PropertieBase {
    public:
        Card* listBox;
        int selectIndex;
        int originalIndex;

        PropList(const std::string& Name);

        bool EditPropertie() override;
        bool Cancel() override;

        PropertyType GetType() override;
};

class PropListMeshParts : public PropList {
    public:
        PropListMeshParts(const std::string& Name);

        Mesh* mesh;
        int filasMax;   // alto de la VENTANA (ajustable arrastrando)
        int scrollFila; // primera fila visible
        int modo;       // 0 = mesh parts (materialsGroup); 1 = UV maps; 2 = capas de color
        void AjustarVentana(); // mantiene la seleccion a la vista
        // segun 'modo' lee la lista correcta (la MISMA lista para parts/uvmaps/colors):
        int         ListaCount() const;       // cantidad de items
        std::string ListaNombre(int i) const; // nombre del item i
        void        ListaSeleccionar(int i);  // setea el ACTIVO (selectIndex + activo de la malla) + re-bake

        void button_up() override;
        void button_down() override;
        void button_left() override;
        void button_right() override;

        void RenderPropertiBox(Card* propertiBox) override;
        void RenderPropertiBoxBorder(Card* propertiBox) override;
        void RenderPropertiValue(Card* propertiBox) override;
        void RenderPropertiLabel(Card* propertiBox) override;
        int Resize(int width) override;
};

#endif