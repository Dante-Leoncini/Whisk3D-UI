#include "PropFloat.h"
#include "w3dGraphics.h"
#include <cstdio>   // snprintf (valor -> texto)
#include <cstdlib>  // atof (texto -> valor)

// ==== edicion numerica por TEXTO (click/OK -> tipear + enter). g_activeTextField apunta al field de este prop ====
PropFloat* g_propFloatEditing = NULL;

void PropFloat::StartEditText(){
    if (!value) return;
    originalValue = *value;               // por si se cancela con Esc
    char buf[32];
    if (integer) snprintf(buf, sizeof(buf), "%d", (int)(*value + 0.5f));
    else        snprintf(buf, sizeof(buf), "%g", *value); // compacto: "0", "2", "1.45"
    field.SetText(buf);
    field.SelectAll();                    // TODO seleccionado: la 1ra tecla reemplaza
    g_activeTextField = &field;           // el input (SDL_TEXTINPUT / teclas del Nokia) entra a este campo
    g_propFloatEditing = this;
    editing = true;
}

bool NumEditActive(){ return g_propFloatEditing != NULL; }
// el evaluador de la matematica (+ - * / y parentesis): lo pone el editor (W3dEvalExpr, NumInput.cpp). Sin el, atof
bool (*g_propFloatEval)(const std::string&, float&) = NULL;

void NumEditCommit(){
    if (g_propFloatEditing){
        PropFloat* p = g_propFloatEditing;
        float v = 0.0f;
        if (!(g_propFloatEval && g_propFloatEval(p->field.text, v))) v = (float)atof(p->field.text.c_str()); // invalido -> atof
        p->Set(v);
        p->editing = false;
    }
    g_propFloatEditing = NULL;
    g_activeTextField = NULL;
}

void NumEditCancel(){
    if (g_propFloatEditing) g_propFloatEditing->editing = false; // NO aplica: el valor queda como estaba
    g_propFloatEditing = NULL;
    g_activeTextField = NULL;
}

PropFloat::PropFloat(const std::string& Name, const std::string& Unit):
    unit(Unit), PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    value = NULL;
    originalValue = 1.0f;
    fineStep = 0.01f;
    thickStep = 0.1f;
    dragStep = 0.1f;   // por pixel arrastrado
    minVal = 0.0f; maxVal = 1.0f; hasRange = false;
    onChange = NULL;
    integer = false; centered = false; arrows = false; mixed = false; options = NULL; nOptions = 0;
    accelerates = false; holdDir = 0; holdCount = 0;
    field.numeric = true;
    // (unit viene del init-list: el ° de rotacion vive)
};

// paso acelerado del izq/der cuando acelera==true: arranca en 1 (preciso) y sube DE A POCO hasta 10.
// Misma idea que el color picker (consecutivos en la misma direccion rampan; cambio de direccion resetea).
int PropFloat::StepAccel(int dir){
    if (dir == holdDir) holdCount++; else { holdDir = dir; holdCount = 0; }
    int c = holdCount;
    return (c < 5) ? 1 : (c < 12) ? 2 : (c < 22) ? 5 : 10;
}

void PropFloat::Set(float v){
    if (hasRange){
        if (v < minVal) v = minVal;
        if (v > maxVal) v = maxVal;
    }
    if (value) *value = v;
    if (onChange) onChange(); // ej: rotacion -> reconstruye el quaternion
};

void PropFloat::SetRange(float mn, float mx){
    minVal = mn; maxVal = mx; hasRange = true;
};

void PropFloat::RenderPropertiBox(Card* propertiBox){
    if (value){
        propertiBox->Render(false);
        w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
    }
}

void PropFloat::RenderPropertiBoxBorder(Card* propertiBox){
    if (value){
        w3dEngine::Translatef(0, -RowHeightGS - gapGS, 0);
        propertiBox->RenderBorder(false);
        w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
    }
};

void PropFloat::RenderPropertiValue(Card* propertiBox){
    if (value){
        int boxW = propertiBox->width - bordersGS;
        textAlign to = centered ? textAlign::center : textAlign::left;
        // EDITANDO POR TEXTO: mostrar el campo (con TODO seleccionado en accent, o con caret) en vez del valor.
        if (g_propFloatEditing == this && g_activeTextField == &field){
            RenderTextField(field, true, to, boxW);
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
            return;
        }
        if (arrows){
            // < a la izquierda y > a la derecha (el hit-test del click lo hace
            // el contenedor, que conoce el rect del box)
            int aw = RowHeightGS;
            w3dEngine::PushMatrix(); RenderBitmapText("<", textAlign::center, aw); w3dEngine::PopMatrix();
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(boxW - aw), 0, 0);
            RenderBitmapText(">", textAlign::center, aw);
            w3dEngine::PopMatrix();
        }
        if (altered) AlteredColor();   // (un override de este proyecto: en amarillo)
        if (mixed){
            RenderBitmapText("-", to, boxW);   // seleccion multiple con valores distintos
        } else if (integer && options && nOptions > 0){
            int r = (int)(*value + 0.5f); if (r < 0) r = 0; if (r >= nOptions) r = nOptions - 1;
            extern const char* (*W3dMenuTranslate)(const char*);
            RenderBitmapText(W3dMenuTranslate ? W3dMenuTranslate(options[r]) : options[r], to, boxW);
        } else if (integer){
            int r = (int)(*value + 0.5f); // vertices: siempre >= 3 (positivo)
            std::ostringstream ss; ss << r << unit;   // la unidad tambien en enteros ("px")
            RenderBitmapText(ss.str(), to, boxW);
        } else {
            RenderBitmapFloat(*value, to, boxW, unit);
        }
        if (altered) SetColorID(ColorID::white);
        w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
    }
}

void PropFloat::RenderPropertiLabel(Card* propertiBox){
    if (value){
        RenderBitmapLabel(name, PropColLabel - bordersGS);
        w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
    }
};

// si es entero, partimos del valor REDONDEADO (el arrastre pudo dejarlo en 8.6
// pero en pantalla se ve 9, asi que la flecha debe ir 9->10, no 8.6->9.6)
float PropFloat::baseStep(){
    return integer ? (float)((int)(*value + 0.5f)) : *value;
}

void PropFloat::button_up(){
    holdDir = holdCount = 0;            // arriba/abajo = paso fino: resetea la rampa del izq/der
    Set(baseStep() + fineStep);
};

void PropFloat::button_down(){
    holdDir = holdCount = 0;
    Set(baseStep() - fineStep);
};

void PropFloat::button_left(){
    if (accelerates) Set(baseStep() - StepAccel(-1)); // arranca en 1, acelera (shininess)
    else         Set(baseStep() - thickStep);         // click-a-click clasico (contadores)
};

void PropFloat::button_right(){
    if (accelerates) Set(baseStep() + StepAccel(1));
    else         Set(baseStep() + thickStep);
};

bool PropFloat::Cancel(){
    //std::cout << "se cancelo originalValue: "<< originalValue << "\n";
    *value = originalValue;
    //std::cout << "quedo asi: "<< *value << "\n";
    editing = false;
    return editing;
};

bool PropFloat::EditPropertie(){
    editing = !editing;
    if (editing){
        originalValue = *value;
        //std::cout << "se guardo originalValue: "<< originalValue << "\n";
    }
    return editing;
};

PropertyType PropFloat::GetType(){
    return PropertyType::Float;
}

int PropFloat::Resize(int w){
    if (!value) return 0; // sin valor no se dibuja: no ocupa fila
    width = w;
    return RowHeightGS + gapGS;
}