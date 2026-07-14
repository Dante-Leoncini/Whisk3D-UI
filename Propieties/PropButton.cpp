#include "PropButton.h"
#include "w3dGraphics.h"

PropButton::PropButton(const std::string& Name, int Icon):
    PropertieBase(Name){
    // ancho maximo de la columna (no se adapta al texto)
    button = new Button(Name, Icon, false);
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
    button->Resize(w - bordersGS);
    return button->height + gapGS; // el boton es mas alto que un renglon
}

bool PropButton::EditPropertie(){
    if (gris) return false;   // atenuado (ej: Render Animation sin animaciones): no ejecuta la accion
    if (action) action(); // la accion del boton (click o enter)
    return false; // un boton no entra en modo edicion
}

void PropButton::RenderPropertiBox(Card* propertiBox){
    if (oculto) return;
    // el boton arranca en el borde izquierdo del cuerpo del grupo
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, 0, 0);
    static const float grisAtenuado[4] = {0.35f, 0.35f, 0.35f, 1.0f};
    const float* txtPrev = button->colorTexto;
    if (gris) button->colorTexto = grisAtenuado; // texto+icono atenuados
    button->Render();
    if (gris) button->colorTexto = txtPrev;
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, (GLfloat)(button->height + gapGS), 0);
}

void PropButton::RenderPropertiBoxBorder(Card* propertiBox){
    if (oculto) return;
    // fila seleccionada con el teclado: borde resaltado sobre el boton
    w3dEngine::Translatef((GLfloat)-PropColEtiqueta, (GLfloat)(-button->height - gapGS), 0);
    button->card->RenderBorder(false);
    w3dEngine::Translatef((GLfloat)PropColEtiqueta, (GLfloat)(button->height + gapGS), 0);
}

void PropButton::RenderPropertiValue(Card* propertiBox){
    if (oculto) return;
    w3dEngine::Translatef(0, (GLfloat)(button->height + gapGS), 0);
}

void PropButton::RenderPropertiLabel(Card* propertiBox){
    if (oculto) return;
    w3dEngine::Translatef(0, (GLfloat)(button->height + gapGS), 0);
}
