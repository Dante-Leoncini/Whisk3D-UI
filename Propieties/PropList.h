#ifndef PROPLIST_H
#define PROPLIST_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"
#include "objects/Mesh.h"

class Armature; // objects/Armature.h (lista de clips de animacion, modo 5)

// modo 13: el ARBOL del MIX de animaciones (objetos -> capas). La lista no conoce el modelo: lo pregunta a
// estos ganchos, que instala el panel de propiedades (Properties.cpp). NULL = lista vacia.
extern int  (*MixFilasCount)();
extern std::string (*MixFilaTexto)(int fila);
extern int  (*MixFilaNivel)(int fila);        // sangria (0 = objeto/escenas, 1 = capa)
extern int  (*MixFilaOjo)(int fila);          // -1 sin ojo, 0 oculta, 1 visible
extern void (*MixFilaSeleccionar)(int fila);
extern int  (*MixFilaActiva)();
extern int  (*MixFilaPlegada)(int fila);      // -1 no se pliega (capa), 0 desplegada, 1 plegada
extern int  (*MixFilaIcono)(int fila);        // IconType de la fila

class PropList : public PropertieBase {
    public:
        Card* listBox;
        int selectIndex;
        int originalIndex;

        PropList(const std::string& Name);
        ~PropList(); // libera listBox (la lista es duena de su caja)

        bool EditPropertie() W3D_OVERRIDE;
        bool Cancel() W3D_OVERRIDE;

        PropertyType GetType() W3D_OVERRIDE;
};

class PropListMeshParts : public PropList {
    public:
        PropListMeshParts(const std::string& Name);

        Mesh* mesh;
        Armature* arm;  // fuente cuando modo == 5 (clips de animacion del esqueleto)
        // fuente cuando modo == 11 (stack de constraints). Va APARTE de 'mesh' y no es un
        // Mesh* casteado: los constraints los tiene CUALQUIER objeto (una luz, una camara,
        // una coleccion), y meter un Object* que no es malla por el campo 'mesh' seria un
        // downcast invalido -> la lista leeria basura del otro lado del objeto.
        Object* obj;
        int filasMax;   // alto de la VENTANA (ajustable arrastrando)
        int scrollFila; // primera fila visible
        int modo;       // 0 = mesh parts; 1 = UV maps; 2 = colores; 3 = modificadores; 4 = vertex groups; 5 = animaciones (armature); 6 = huesos (armature); 7 = animaciones del OBJETO (vertex anims del mesh); 8 = huesos 2D del armature 2D ACTIVO; 10 = armatures 2D del mesh (Mesh::armatures2d); 11 = stack de constraints (Object::constraints, va por 'obj'); 12 = scripts lua del objeto (Object::scriptDatos, va por 'obj'; el ORDEN es el de ejecucion); 13 = el arbol del MIX de animaciones (ganchos Mix*)
        void AjustarVentana(); // mantiene la seleccion a la vista
        // segun 'modo' lee la lista correcta (la MISMA lista para parts/uvmaps/colors):
        int         ListaCount() const;       // cantidad de items
        std::string ListaNombre(int i) const; // nombre del item i
        void        ListaSeleccionar(int i);  // setea el ACTIVO (selectIndex + activo de la malla) + re-bake

        void button_up() W3D_OVERRIDE;
        void button_down() W3D_OVERRIDE;
        void button_left() W3D_OVERRIDE;
        void button_right() W3D_OVERRIDE;

        void RenderPropertiBox(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiBoxBorder(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiValue(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiLabel(Card* propertiBox) W3D_OVERRIDE;
        int Resize(int width) W3D_OVERRIDE;
};

#endif