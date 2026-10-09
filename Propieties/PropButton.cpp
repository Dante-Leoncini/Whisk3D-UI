#include "PropButton.h"
#include "w3dGraphics.h"
#include "WhiskUI/text/bitmapText.h"   // el label de la izquierda (modo conLabel)

PropButton::PropButton(const std::string& Name, int Icon):
    PropertieBase(Name){
    // ancho maximo de la columna (no se adapta al texto)
    button = new Button(Name, Icon, false);
    conLabel = false;
    colBoton = 0;
    oculto = false;
    gris = false;
    action = NULL;
    animProp = -1; animComp = 0;
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
    // con label: el nombre mide su TEXTO (hasta el 55 % del renglon) y el boton ocupa TODO el resto. Antes el boton
    // iba solo en la columna de valores: con una tarjeta de valores angosta el desplegable quedaba chiquito a la
    // derecha y lo elegido no se leia ni con el panel a pantalla completa
    // animable: deja la columna del rombo de keyframe a la derecha (g_propKeyBtnCol, la publica el grupo)
    const int key = (animProp >= 0) ? g_propKeyBtnCol : 0;
    if (conLabel) {
        int chars = 0;
        for (size_t i = 0; i < name.size(); i++) if (((unsigned char)name[i] & 0xC0) != 0x80) chars++;   // (UTF-8)
        colBoton = chars * CharacterWidthGS + gapGS * 2 + bordersGS;
        if (colBoton > w * 55 / 100) colBoton = w * 55 / 100;
        button->Resize(w - colBoton - bordersGS - key);
    } else {
        button->Resize(w - bordersGS - key);
    }
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
    // el origen de este pase es la columna de valores: sin label el boton arranca en el borde izquierdo del cuerpo
    // del grupo; con label, despues de su nombre (ColBoton)
    const GLfloat dx = (GLfloat)(ColBoton() - PropColEtiqueta);
    w3dEngine::Translatef(dx, 0, 0);
    static const float grisAtenuado[4] = {0.35f, 0.35f, 0.35f, 1.0f};
    const float* txtPrev = button->colorTexto;
    if (gris) button->colorTexto = grisAtenuado; // texto+icono atenuados
    button->Render();
    if (gris) button->colorTexto = txtPrev;
    w3dEngine::Translatef(-dx, (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiBoxBorder(Card* propertiBox){
    if (oculto) return;
    // fila seleccionada con el teclado: borde resaltado sobre el boton
    const GLfloat dx = (GLfloat)(ColBoton() - PropColEtiqueta);
    w3dEngine::Translatef(dx, (GLfloat)-Avance(), 0);
    button->card->RenderBorder(false);
    w3dEngine::Translatef(-dx, (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiValue(Card* propertiBox){
    if (oculto) return;
    w3dEngine::Translatef(0, (GLfloat)Avance(), 0);
}

void PropButton::RenderPropertiLabel(Card* propertiBox){
    if (oculto) return;
    if (conLabel) {   // (el origen del pase es el final de la columna de etiquetas) al borde izquierdo del renglon; si no
                      // entra (panel angosto) se corta por el FINAL: "Modo de re", no "do de render"
        const GLfloat dx = (GLfloat)(bordersGS + gapGS * 2 - PropColEtiqueta);
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapText(name, textAlign::left, colBoton - bordersGS - gapGS * 2);
        w3dEngine::Translatef(-dx, 0, 0);
    }
    w3dEngine::Translatef(0, (GLfloat)Avance(), 0);
}
