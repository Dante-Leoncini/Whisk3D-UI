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
    extras[0] = extras[1] = NULL; accionExtra[0] = accionExtra[1] = NULL; nExtras = 0; slotsExtras = 0; colExtras = 0;
    prefijo = NULL; accionPrefijo = NULL; colPrefijo = 0; labelChars = 0;
}
void PropButton::AgregarPrefijo(void (*accion)(), const char* ayuda) {
    if (prefijo) return;
    prefijo = new Button("", -1, false);
    prefijo->cuadrado = true;
    prefijo->altoRenglon = true;
    if (ayuda) prefijo->ayuda = ayuda;
    accionPrefijo = accion;
}
void PropButton::AgregarExtra(int icono, void (*accion)(), const char* ayuda) {
    if (nExtras >= 2) return;
    Button* b = new Button("", icono, false);
    b->cuadrado = true;
    b->altoRenglon = true;
    if (ayuda) b->ayuda = ayuda;
    extras[nExtras] = b; accionExtra[nExtras] = accion; nExtras++;
}
// los extras van alineados a la DERECHA de su zona: el ultimo en el ultimo lugar (asi la UV coincide en todas las filas)
int PropButton::ExtraX(int i) const {
    const int slots = slotsExtras > nExtras ? slotsExtras : nExtras;
    const int paso = (int)RenglonHeightGS + gapGS;
    return colExtras + (slots - nExtras + i) * paso;
}
int PropButton::ExtraEnX(int lx) const {
    for (int i = 0; i < nExtras; i++) {
        if (!extras[i] || !extras[i]->visible) continue;
        const int x = ExtraX(i);
        if (lx >= x && lx < x + extras[i]->width) return i;
    }
    return -1;
}

PropButton::~PropButton(){
    delete button;
    for (int i = 0; i < nExtras; i++) delete extras[i];
    delete prefijo;
}

PropertyType PropButton::GetType(){
    return PropertyType::Button;
}

int PropButton::Resize(int w){
    if (oculto) return 0;
    if (conLabel) {   // desplegable de propiedades: flechita a la derecha + box igual a los inputs
        button->caretMenu = button->desplegable;   // (un NUMERO o un color no despliega nada: sin flechita)
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
        if (labelChars > 0) chars = labelChars;
        colBoton = chars * CharacterWidthGS + gapGS * 2 + bordersGS;
        if (colBoton > w * 55 / 100) colBoton = w * 55 / 100;
        // el CUADRADITO (el color base) va pegado despues del nombre; el boton grande arranca despues de el
        colPrefijo = colBoton;
        if (prefijo && prefijo->visible) { prefijo->Resize((int)RenglonHeightGS); colBoton += prefijo->width + gapGS; }
        // los EXTRAS van a la DERECHA (sus lugares reservados), el boton grande ocupa lo del medio
        const int slots = slotsExtras > nExtras ? slotsExtras : nExtras;
        const int zona = slots > 0 ? slots * ((int)RenglonHeightGS + gapGS) : 0;
        for (int i = 0; i < nExtras; i++) if (extras[i]) extras[i]->Resize((int)RenglonHeightGS);
        colExtras = w - bordersGS - key - zona + gapGS;
        button->Resize(w - colBoton - bordersGS - key - zona);
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
    // los EXTRAS (despues del nombre, antes del boton grande)
    if (conLabel && nExtras > 0) {
        for (int i = 0; i < nExtras; i++) {
            if (!extras[i] || !extras[i]->visible) continue;
            const GLfloat ex = (GLfloat)(ExtraX(i) - PropColEtiqueta);
            w3dEngine::Translatef(ex, 0, 0);
            extras[i]->Render();
            w3dEngine::Translatef(-ex, 0, 0);
        }
    }
    if (conLabel && prefijo && prefijo->visible) {
        const GLfloat px = (GLfloat)(colPrefijo - PropColEtiqueta);
        w3dEngine::Translatef(px, 0, 0);
        prefijo->Render();
        w3dEngine::Translatef(-px, 0, 0);
    }
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
