#ifndef PROPLIST_H
#define PROPLIST_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"
#include "objects/Mesh.h"

class Button;
class Armature; // objects/Armature.h (lista de clips de animacion, modo 5)

// modo 13: el ARBOL del MIX de animaciones (objetos -> capas). La lista no conoce el modelo: lo pregunta a
// estos ganchos, que instala el panel de propiedades (Properties.cpp). NULL = lista vacia.
extern int  (*MixRowsCount)();
extern std::string (*MixRowText)(int row);
extern int  (*MixRowLevel)(int row);        // sangria (0 = objeto/escenas, 1 = capa)
extern int  (*MixRowEye)(int row);          // -1 sin ojo, 0 oculta, 1 visible
extern void (*MixRowSelect)(int row);
// modo 14: los PASOS de una rutina ('obj'). La lista no conoce la Rutina (es del editor): los llena PropsRoutine.cpp
extern int (*RoutineRowsCount)(Object* o);
extern std::string (*RoutineRowText)(Object* o, int row);
extern void (*RoutineRowSelect)(Object* o, int row);
extern int (*RoutineRowIcon)(Object* o, int row);   // el icono del grupo del paso (-1 = sin icono)
// lo que se ve a la DERECHA de la fila: 0 nada, 1 checkbox apagado, 2 prendido, 3 un cuadro del color 'rgba'
extern int (*RoutineRowValue)(Object* o, int row, float* rgba);
// arrastrando un paso: la fila donde caeria (se dibuja una linea arriba de ella; -1 = no se arrastra)
extern int g_routineRowDestination;
extern int  (*MixActiveRow)();
// modo 15: la pila de POSTPROCESADO (render/PostProcess.h, del editor): la llenan estos ganchos (Properties.cpp)
extern int (*PostRowsCount)();
extern std::string (*PostRowText)(int row);
extern void (*PostRowSelect)(int row);
// modo 16: la pila de MODIFICADORES DE LA CURVA activa del timeline (Animation.h): la llenan estos ganchos (Properties.cpp)
extern int (*CurveModRowsCount)();
extern std::string (*CurveModRowText)(int row);
extern void (*CurveModRowSelect)(int row);
// modo 17 (las CAPAS DE TEXTURA del material activo): los instala Properties.cpp
extern int (*LayerTexRowsCount)();
extern std::string (*LayerTexRowText)(int row);
extern void (*LayerTexRowSelect)(int row);
extern int (*LayerTexActiveRow)();
// modo 18 (las CAPAS DE MATERIAL del mesh part activo): los instala Properties.cpp
extern int (*LayerMatRowsCount)();
extern std::string (*LayerMatRowText)(int row);
extern void (*LayerMatRowSelect)(int row);
extern int (*LayerMatActiveRow)();
extern int  (*MixCollapsedRow)(int row);      // -1 no se pliega (capa), 0 desplegada, 1 plegada
extern int  (*MixRowIcon)(int row);           // IconType de la fila

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
        virtual Button* RenameField() { return 0; }   // el boton que se vuelve el campo de renombrar una fila (o NULL)
};

class PropListMeshParts : public PropList {
    public:
        PropListMeshParts(const std::string& Name);
        ~PropListMeshParts();   // (sus botones: el de renombrar en linea y los tres del costado)
        Button* RenameField() W3D_OVERRIDE { return rowButton; }

        Mesh* mesh;
        Armature* arm;  // fuente cuando modo == 5 (clips de animacion del esqueleto)
        // fuente cuando modo == 11 (stack de constraints). Va APARTE de 'mesh' y no es un
        // Mesh* casteado: los constraints los tiene CUALQUIER objeto (una luz, una camara,
        // una coleccion), y meter un Object* que no es malla por el campo 'mesh' seria un
        // downcast invalido -> la lista leeria basura del otro lado del objeto.
        Object* obj;
        int rowsMax;   // alto de la VENTANA (ajustable arrastrando)
        int scrollRow; // primera fila visible
        int mode;      // 0 = mesh parts; 1 = UV maps; 2 = colores; 3 = modificadores; 4 = vertex groups; 5 = animaciones (armature); 6 = huesos (armature); 7 = animaciones del OBJETO (vertex anims del mesh); 8 = huesos 2D del armature 2D ACTIVO; 10 = armatures 2D del mesh (Mesh::armatures2d); 11 = stack de constraints (Object::constraints, va por 'obj'); 12 = scripts lua del objeto (Object::scriptData, va por 'obj'; el ORDEN es el de ejecucion); 13 = el arbol del MIX de animaciones (ganchos Mix*); 19 = stack de constraints del hueso ACTIVO del armature (va por 'arm', Pose Mode)
        // LO QUE SE HACE CON UNA FILA (lo pone el panel al armar la lista; NULL = esa lista no lo tiene). Actuan sobre
        // la fila ACTIVA: el que las llama la elige antes. Con ellas la lista muestra el TACHO a la derecha de cada fila
        // (si hay mas de 'minItems'), el menu contextual (click derecho / mantener y soltar), renombra con doble click
        // y se reordena arrastrando una fila
        void (*opDelete)(); void (*opRename)(); void (*opUpload)(); void (*opLower)();
        bool (*opReadOnly)();   // true = ahora no se puede tocar (una libreria): sin tacho ni arrastre
        void (*opChange)();     // != NULL: cada fila lleva una FLECHA (a la izquierda del tacho) que abre un menu para
                                   // cambiar lo que tiene (las capas de material: elegir otro material)
        int minItems;              // con esta cantidad (o menos) no se borra: el tacho no esta (mesh parts: 1)
        struct Extra { const char* text; void (*action)(); int icon; bool onlyEdit; };
        Extra extras[4]; int nExtras;   // opciones de mas para el menu (Asignar / Seleccionar...; onlyEdit: en Edicion)
        void AddExtra(const char* text, void (*action)(), int icon, bool onlyEdit);
        bool WithTrash() const;    // las filas llevan el tacho
        bool Reordenable() const;  // se puede arrastrar una fila
        Button* rowButton;         // el campo de RENOMBRAR en linea (se dibuja en la fila rowRow mientras edita)
        int rowRow;
        int rowDestination;           // arrastrando una fila: la linea de donde cae (-1 = no se arrastra)
        static int Handle();          // el alto de la franja de abajo (la linea para agrandar / achicar la lista)
        // EL ALTO: sin tocar, las filas que hay (hasta rowsMax). Si el usuario lo cambio (heightFixed) o la lista tiene
        // un minimo (minRows), mide eso aunque sobren renglones en blanco
        bool heightFixed; int minRows;
        int heightPx;              // (heightFixed) el alto de las filas en PIXELES: el asa lo mueve de a pixel, no de a renglon
        int handleState;           // la linea de abajo: 0 normal (el color de la caja), 1 el puntero encima (blanca), 2 agarrada (verde)
        int RowHeightPx() const;   // el alto del area de las filas (sin los bordes de la caja)
        // LA BARRITA DE SCROLL de la lista (con mas filas que renglones): 0 normal, 1 el puntero encima (mas ancha y
        // blanca), 2 agarrada (verde). ScrollGeo: el largo del riel, el del agarre y donde esta (pixeles, desde arriba)
        int sbState;
        int hoverRow;             // la fila con el puntero encima (su texto va en blanco); -1 = ninguna
        bool ScrollGeo(int* rail, int* grab, int* pos) const;
        void ScrollFromY(int y);  // el agarre con su centro en 'y' (pixeles desde arriba del riel)
        bool showEmpty;           // se dibuja aunque no tenga nada (las capas de textura: sus 3 renglones)
        bool hidden;              // no se dibuja ni ocupa lugar (las capas de textura con el material por defecto)
        int RowsHeight() const;   // los renglones que mide la caja
        // TRES BOTONES A LA DERECHA de la caja, a lo alto (subir / bajar / opciones de la fila elegida)
        bool buttonsSide; Button* sideBtn[3];
        int SideWidth() const;     // lo que ocupa esa columna (0 sin botones)
        bool SideEnabled(int k) const;   // se puede usar ahora (subir: no es la primera; bajar: no es la ultima)
        void FitWindow(); // mantiene la seleccion a la vista
        // segun 'modo' lee la lista correcta (la MISMA lista para parts/uvmaps/colors):
        int         ListCount() const;     // cantidad de items
        std::string ListName(int i) const; // nombre del item i
        void        ListSelect(int i);     // setea el ACTIVO (selectIndex + activo de la malla) + re-bake

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