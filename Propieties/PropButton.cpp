#include "PropButton.h"
#include "w3dGraphics.h"
#include "WhiskUI/text/bitmapText.h"   // el label de la izquierda (modo conLabel)

PropButton::PropButton(const std::string& Name, int Icon):
    PropertieBase(Name){
    // ancho maximo de la columna (no se adapta al texto)
    button = new Button(Name, Icon, false);
    conLabel = false;
    oculto = false;
    gris = false;
    action = NULL;
}

PropButton::~PropButton(){
    delete button;
}

PropertyType PropButton::GetType(){
    return PropertyType::Button;
}

int PropButton::Resize(int w){
    if (oculto) return 0;
    if (conLabel) {   // desplegable de propiedades: flechita a la derecha + box igual a los inputs
        button->caretMenu = true;
        button->altoRenglon = true;
    }
    // con label: el boton ocupa SOLO la columna de valores (el nombre va a la izquierda)
    button->Resize(conLabel ? (w - PropColEtiqueta - bordersGS) : (w - bordersGS));
    return Avance();
}

// avance de fila: con label el boton entra en la GRILLA de renglones (su box sobresale
// GlobalScale arriba y abajo, igual que el de los inputs); solo cambia el paso si es
// un boton suelto de ancho completo (mas alto)
int PropButton::Avance() const {
    return (conLabel ? RenglonHeightGS : button->height) + gapGS;
}

bool PropButton::EditPropertie(){
    if (gris) return false;   // atenuado (ej: Render Animation sin animaciones): no ejecuta la accion
    if (action) action(); // la accion del boton (click o enter)
    return false; // un boton no entra en modo edicion
}

void PropButton::RenderPropertiBox(Card* propertiBox){
    if (oculto) return;
    // sin label el boton arranca en el borde izquierdo del cuerpo del grupo;
    // con label queda en la columna de valores (el origen de este pase)
    if (!conLabel) w3dEngine::Translatef((GLfloat)-PropColEtiqueta, 0, 0);
    static const float grisAtenuado[4] = {0.35f, 0.35f, 0.35f, 1.0f};
    const float* txtPrev = button->colorTexto;
    if (gris) button->colorTexto = grisAtenuado; // texto+icono atenuados
    button->Render();
    if (gris) button->colorTexto = txtPrev;
    w3dEngine::Translatef((GLfloat)(conLabel ? 0 : PropColEtiqueta), (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiBoxBorder(Card* propertiBox){
    if (oculto) return;
    // fila seleccionada con el teclado: borde resaltado sobre el boton
    GLfloat off = (GLfloat)(conLabel ? 0 : PropColEtiqueta);
    w3dEngine::Translatef(-off, (GLfloat)-Avance(), 0);
    button->card->RenderBorder(false);
    w3dEngine::Translatef(off, (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiValue(Card* propertiBox){
    if (oculto) return;
    w3dEngine::Translatef(0, (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiLabel(Card* propertiBox){
    if (oculto) return;
    if (conLabel) RenderBitmapText(name, textAlign::right, PropColEtiqueta - bordersGS);
    w3dEngine::Translatef(0, (GLfloat)Avance(), 0);
}
