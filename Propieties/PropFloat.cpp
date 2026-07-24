#include "PropFloat.h"
#include "w3dGraphics.h"
#include <cstdio>   // snprintf (valor -> texto)
#include <cstdlib>  // atof (texto -> valor)

// ==== edicion numerica por TEXTO (click/OK -> tipear + enter). g_textFieldActivo apunta al field de este prop ====
PropFloat* g_propFloatEditando = NULL;

void PropFloat::IniciarEdicionTexto(){
    if (!value) return;
    originalValue = *value;               // por si se cancela con Esc
    char buf[32];
    if (entero) snprintf(buf, sizeof(buf), "%d", (int)(*value + 0.5f));
    else        snprintf(buf, sizeof(buf), "%g", *value); // compacto: "0", "2", "1.45"
    field.SetText(buf);
    field.SelectAll();                    // TODO seleccionado: la 1ra tecla reemplaza
    g_textFieldActivo = &field;           // el input (SDL_TEXTINPUT / teclas del Nokia) entra a este campo
    g_propFloatEditando = this;
    editando = true;
}

bool NumEditActivo(){ return g_propFloatEditando != NULL; }

void NumEditCommit(){
    if (g_propFloatEditando){
        PropFloat* p = g_propFloatEditando;
        p->Set((float)atof(p->field.text.c_str())); // texto vacio/invalido -> 0 (atof)
        p->editando = false;
    }
    g_propFloatEditando = NULL;
    g_textFieldActivo = NULL;
}

void NumEditCancel(){
    if (g_propFloatEditando) g_propFloatEditando->editando = false; // NO aplica: el valor queda como estaba
    g_propFloatEditando = NULL;
    g_textFieldActivo = NULL;
}

PropFloat::PropFloat(const std::string& Name, const std::string& Unit):
    unit(Unit), PropertieBase(Name){
    // (eran inicializadores de clase: C++03)
    value = NULL;
    originalValue = 1.0f;
    stepFino = 0.01f;
    stepGrueso = 0.1f;
    dragStep = 0.1f;   // por pixel arrastrado
    minVal = 0.0f; maxVal = 1.0f; tieneRango = false;
    onChange = NULL;
    entero = false; centrado = false; flechas = false;
    acelera = false; holdDir = 0; holdCount = 0;
    // (unit viene del init-list: el ° de rotacion vive)
};

// paso acelerado del izq/der cuando acelera==true: arranca en 1 (preciso) y sube DE A POCO hasta 10.
// Misma idea que el color picker (consecutivos en la misma direccion rampan; cambio de direccion resetea).
int PropFloat::PasoAccel(int dir){
    if (dir == holdDir) holdCount++; else { holdDir = dir; holdCount = 0; }
    int c = holdCount;
    return (c < 5) ? 1 : (c < 12) ? 2 : (c < 22) ? 5 : 10;
}

void PropFloat::Set(float v){
    if (tieneRango){
        if (v < minVal) v = minVal;
        if (v > maxVal) v = maxVal;
    }
    if (value) *value = v;
    if (onChange) onChange(); // ej: rotacion -> reconstruye el quaternion
};

void PropFloat::SetRango(float mn, float mx){
    minVal = mn; maxVal = mx; tieneRango = true;
};

void PropFloat::RenderPropertiBox(Card* propertiBox){
    if (value){
        propertiBox->Render(false);
        w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
    }
}

void PropFloat::RenderPropertiBoxBorder(Card* propertiBox){
    if (value){
        w3dEngine::Translatef(0, -RenglonHeightGS - gapGS, 0);
        propertiBox->RenderBorder(false);
        w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
    }
};

void PropFloat::RenderPropertiValue(Card* propertiBox){
    if (value){
        int boxW = propertiBox->width - bordersGS;
        textAlign al = centrado ? textAlign::center : textAlign::left;
        // EDITANDO POR TEXTO: mostrar el campo (con TODO seleccionado en accent, o con caret) en vez del valor.
        if (g_propFloatEditando == this && g_textFieldActivo == &field){
            if (field.selectAll){
                w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::accent)]); // todo sel: tipear reemplaza
                RenderBitmapText(field.text, al, boxW);
                w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]);
            } else {
                std::string shown = field.text.substr(0, field.caret) + "|" + field.text.substr(field.caret);
                RenderBitmapText(shown, al, boxW);
            }
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
            return;
        }
        if (flechas){
            // < a la izquierda y > a la derecha (el hit-test del click lo hace
            // el contenedor, que conoce el rect del box)
            int aw = RenglonHeightGS;
            w3dEngine::PushMatrix(); RenderBitmapText("<", textAlign::center, aw); w3dEngine::PopMatrix();
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(boxW - aw), 0, 0);
            RenderBitmapText(">", textAlign::center, aw);
            w3dEngine::PopMatrix();
        }
        if (entero){
            int r = (int)(*value + 0.5f); // vertices: siempre >= 3 (positivo)
            std::ostringstream ss; ss << r << unit;   // la unidad tambien en enteros ("px")
            RenderBitmapText(ss.str(), al, boxW);
        } else {
            RenderBitmapFloat(*value, al, boxW, unit);
        }
        w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
    }
}

void PropFloat::RenderPropertiLabel(Card* propertiBox){
    if (value){
        RenderBitmapText(name, textAlign::right, PropColEtiqueta - bordersGS);
        w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
    }
};

// si es entero, partimos del valor REDONDEADO (el arrastre pudo dejarlo en 8.6
// pero en pantalla se ve 9, asi que la flecha debe ir 9->10, no 8.6->9.6)
float PropFloat::baseStep(){
    return entero ? (float)((int)(*value + 0.5f)) : *value;
}

void PropFloat::button_up(){
    holdDir = holdCount = 0;            // arriba/abajo = paso fino: resetea la rampa del izq/der
    Set(baseStep() + stepFino);
};

void PropFloat::button_down(){
    holdDir = holdCount = 0;
    Set(baseStep() - stepFino);
};

void PropFloat::button_left(){
    if (acelera) Set(baseStep() - PasoAccel(-1)); // arranca en 1, acelera (shininess)
    else         Set(baseStep() - stepGrueso);    // click-a-click clasico (contadores)
};

void PropFloat::button_right(){
    if (acelera) Set(baseStep() + PasoAccel(1));
    else         Set(baseStep() + stepGrueso);
};

bool PropFloat::Cancel(){
    //std::cout << "se cancelo originalValue: "<< originalValue << "\n";
    *value = originalValue;
    //std::cout << "quedo asi: "<< *value << "\n";
    editando = false;
    return editando;
};

bool PropFloat::EditPropertie(){
    editando = !editando;
    if (editando){
        originalValue = *value;
        //std::cout << "se guardo originalValue: "<< originalValue << "\n";
    }
    return editando;
};

PropertyType PropFloat::GetType(){
    return PropertyType::Float;
}

int PropFloat::Resize(int w){
    if (!value) return 0; // sin valor no se dibuja: no ocupa fila
    width = w;
    return RenglonHeightGS + gapGS;
}