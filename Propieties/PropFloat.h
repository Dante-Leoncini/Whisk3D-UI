#ifndef PROPFLOAT_H
#define PROPFLOAT_H
#include "crossplatform.h" // W3D_OVERRIDE (C++03/RVCT)

#include "PropertieBase.h"
#include "WhiskUI/widgets/card.h"
#include "WhiskUI/widgets/TextField.h" // edicion por TEXTO del valor (click / OK -> tipear + enter)

class PropFloat : public PropertieBase {
    public:
        PropFloat(const std::string& Name, const std::string& Unit = "");

        float* value;
        TextField field;            // edicion por texto: al hacer click/OK se enfoca con TODO seleccionado (tipear
                                    // reemplaza), se escribe el numero y Enter lo aplica (mas rapido que arrastrar).
        void IniciarEdicionTexto(); // enfoca el campo con el valor actual formateado + SelectAll
        bool Seleccionable() { return value != NULL; }
        float originalValue;
        std::string unit;

        // pasos de edicion: flechas finas (arriba/abajo), gruesas (izq/der) y
        // cuanto cambia por pixel al ARRASTRAR con el mouse. Rango opcional.
        float stepFino;
        float stepGrueso;
        float dragStep;
        float minVal, maxVal;
        bool tieneRango;            // clampea *value a [minVal, maxVal]
        // modos de presentacion (los usa el redo-panel de primitivas; default off
        // para no tocar el editor de Propiedades):
        bool entero;                // muestra/escalona como entero (redondeado)
        bool centrado;              // centra el valor en su columna
        bool flechas;               // dibuja < > clickeables a los lados
        // ACELERACION del izq/der: arranca en 1 y acelera de a poco.
        // OFF por defecto -> los contadores click-a-click (cortes, segmentos de cilindro/esfera) siguen igual.
        bool acelera;
        int  holdDir, holdCount;    // estado de la rampa (direccion + cuantos seguidos)
        int  PasoAccel(int dir);    // paso acelerado (arranca 1, sube hasta 10)
        // se llama cada vez que el valor cambia (flechas o arrastre). Lo usa la
        // rotacion para reconstruir el quaternion desde los grados (euler).
        void (*onChange)();
        // canal de animacion: si animProp >= 0 la fila muestra el ROMBO de keyframe
        // a la izquierda del label. Valores = AnimPosition/Rotation/Scale/Visible/Render
        // (Core), animComp = AnimX/Y/Z. -1 = no animable (sin rombo).
        int animProp, animComp;
        int AnimProp() W3D_OVERRIDE { return animProp; }
        int AnimComp() W3D_OVERRIDE { return animComp; }
        void Set(float v);          // asigna *value (con clamp), dispara onChange
        void SetRango(float mn, float mx);
        float baseStep();           // valor base para las flechas (redondea si entero)

        PropertyType GetType() W3D_OVERRIDE;

        void button_up() W3D_OVERRIDE;
        void button_down() W3D_OVERRIDE;
        void button_left() W3D_OVERRIDE;
        void button_right() W3D_OVERRIDE;
        bool Cancel() W3D_OVERRIDE;

        bool EditPropertie() W3D_OVERRIDE;

        void RenderPropertiBox(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiBoxBorder(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiValue(Card* propertiBox) W3D_OVERRIDE;
        void RenderPropertiLabel(Card* propertiBox) W3D_OVERRIDE;
        int Resize(int w) W3D_OVERRIDE;
};

// edicion numerica por TEXTO (analogo al rename): mientras esta activa el input llega por g_textFieldActivo
// (SDL_TEXTINPUT en PC; en Symbian las teclas 0-9 y * las rutea el contenedor). Enter aplica, Esc cancela.
extern PropFloat* g_propFloatEditando; // el PropFloat que se esta editando por texto (NULL = ninguno)
bool NumEditActivo();
void NumEditCommit();  // parsea el texto -> Set(valor) (dispara onChange)
void NumEditCancel();  // descarta: el valor queda como estaba

#endif