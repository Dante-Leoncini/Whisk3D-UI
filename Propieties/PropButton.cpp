#include "PropButton.h"
#include "w3dGraphics.h"
#include "WhiskUI/text/bitmapText.h"   // el label de la izquierda (modo withLabel)

PropButton::PropButton(const std::string& Name, int Icon):
    PropertieBase(Name){
    // ancho maximo de la columna (no se adapta al texto)
    button = new Button(Name, Icon, false);
    withLabel = false;
    inColumn = false;
    colButton = 0;
    hidden = false;
    gray = false;
    action = NULL;
    extras[0] = extras[1] = NULL; actionExtra[0] = actionExtra[1] = NULL; nExtras = 0; slotsExtras = 0; colExtras = 0;
    prefix = NULL; actionPrefix = NULL; colPrefix = 0; labelChars = 0;
    focusKb = -1;
}
void PropButton::AddPrefix(void (*newAction)(), const char* help) {
    if (prefix) return;
    prefix = new Button("", -1, false);
    prefix->square = true;
    prefix->rowHeight = true;
    if (help) prefix->help = help;
    actionPrefix = newAction;
}
void PropButton::AddExtra(int icon, void (*newAction)(), const char* help) {
    if (nExtras >= 2) return;
    Button* b = new Button("", icon, false);
    b->square = true;
    b->rowHeight = true;
    if (help) b->help = help;
    extras[nExtras] = b; actionExtra[nExtras] = newAction; nExtras++;
}
// los extras van alineados a la DERECHA de su zona: el ultimo en el ultimo lugar (asi la UV coincide en todas las filas)
int PropButton::ExtraX(int i) const {
    const int slots = slotsExtras > nExtras ? slotsExtras : nExtras;
    const int step = (int)RowHeightGS + gapGS;
    return colExtras + (slots - nExtras + i) * step;
}
int PropButton::ExtraInX(int lx) const {
    for (int i = 0; i < nExtras; i++) {
        if (!extras[i] || !extras[i]->visible) continue;
        const int x = ExtraX(i);
        if (lx >= x && lx < x + extras[i]->width) return i;
    }
    return -1;
}

// el orden VISUAL de izquierda a derecha: el cuadradito, el boton grande y los chicos (solo los visibles)
bool PropButton::KbMove(int dir) {
    if (hidden || !withLabel) return false;
    int order[4]; int n = 0;
    if (prefix && prefix->visible && actionPrefix) order[n++] = -2;
    order[n++] = -1;
    for (int i = 0; i < nExtras; i++) if (extras[i] && extras[i]->visible && actionExtra[i]) order[n++] = i;
    int pos = 0;
    for (int i = 0; i < n; i++) if (order[i] == focusKb) pos = i;
    if (order[pos] != focusKb) { for (int i = 0; i < n; i++) if (order[i] == -1) pos = i; }   // (el foco ya no existe)
    const int newValue = pos + (dir > 0 ? 1 : -1);
    if (newValue < 0 || newValue >= n) { focusKb = order[pos]; return false; }
    focusKb = order[newValue];
    return true;
}

PropButton::~PropButton(){
    delete button;
    for (int i = 0; i < nExtras; i++) delete extras[i];
    delete prefix;
}

PropertyType PropButton::GetType(){
    return PropertyType::Button;
}

int PropButton::Resize(int w){
    if (hidden) return 0;
    if (withLabel) {   // desplegable de propiedades: flechita a la derecha + box igual a los inputs
        button->caretMenu = button->dropdown;   // (un NUMERO o un color no despliega nada: sin flechita)
        button->rowHeight = true;
    }
    // con label: el nombre mide su TEXTO (hasta el 55 % del renglon) y el boton ocupa TODO el resto. Antes el boton
    // iba solo en la columna de valores: con una tarjeta de valores angosta el desplegable quedaba chiquito a la
    // derecha y lo elegido no se leia ni con el panel a pantalla completa
    // animable: deja la columna del rombo de keyframe a la derecha (g_propKeyBtnCol, la publica el grupo)
    const int key = (animProp >= 0) ? g_propKeyBtnCol : 0;
    if (withLabel && inColumn) {   // como un input: el boton en la columna de valores, del ancho de sus boxes
        colButton = PropColLabel;
        colPrefix = colButton; colExtras = w - bordersGS;
        button->Resize(w - bordersGS - PropColLabel - g_propKeyBtnCol);
    } else if (withLabel) {
        int chars = 0;
        for (size_t i = 0; i < name.size(); i++) if (((unsigned char)name[i] & 0xC0) != 0x80) chars++;   // (UTF-8)
        if (labelChars > 0) chars = labelChars;
        colButton = chars * CharacterWidthGS + gapGS * 2 + bordersGS;
        if (colButton > w * 55 / 100) colButton = w * 55 / 100;
        // el CUADRADITO (el color base) va pegado despues del nombre; el boton grande arranca despues de el
        colPrefix = colButton;
        if (prefix && prefix->visible) { prefix->Resize((int)RowHeightGS); colButton += prefix->width + gapGS; }
        // los EXTRAS van a la DERECHA (sus lugares reservados), el boton grande ocupa lo del medio
        const int slots = slotsExtras > nExtras ? slotsExtras : nExtras;
        const int zone = slots > 0 ? slots * ((int)RowHeightGS + gapGS) : 0;
        for (int i = 0; i < nExtras; i++) if (extras[i]) extras[i]->Resize((int)RowHeightGS);
        colExtras = w - bordersGS - key - zone + gapGS;
        button->Resize(w - colButton - bordersGS - key - zone);
    } else {
        button->Resize(w - bordersGS - key);
    }
    return Advance();
}

// avance de fila: con label el boton entra en la GRILLA de renglones (su box sobresale
// GlobalScale arriba y abajo, igual que el de los inputs); solo cambia el paso si es
// un boton suelto de ancho completo (mas alto)
int PropButton::Advance() const {
    return (withLabel ? RowHeightGS : button->height) + gapGS;
}

PropButton* PropButton::running = 0;

bool PropButton::EditPropertie(){
    if (gray) return false;   // atenuado (ej: Render Animation sin animaciones): no ejecuta la accion
    if (action) {   // la accion del boton (click o enter)
        PropButton* before = running;
        running = this;
        action();
        running = before;
    }
    return false; // un boton no entra en modo edicion
}

void PropButton::RenderPropertiBox(Card* propertiBox){
    if (hidden) return;
    // el origen de este pase es la columna de valores: sin label el boton arranca en el borde izquierdo del cuerpo
    // del grupo; con label, despues de su nombre (ColButton)
    // los EXTRAS (despues del nombre, antes del boton grande)
    if (withLabel && nExtras > 0) {
        for (int i = 0; i < nExtras; i++) {
            if (!extras[i] || !extras[i]->visible) continue;
            const GLfloat ex = (GLfloat)(ExtraX(i) - PropColLabel);
            w3dEngine::Translatef(ex, 0, 0);
            extras[i]->Render();
            w3dEngine::Translatef(-ex, 0, 0);
        }
    }
    if (withLabel && prefix && prefix->visible) {
        const GLfloat px = (GLfloat)(colPrefix - PropColLabel);
        w3dEngine::Translatef(px, 0, 0);
        prefix->Render();
        w3dEngine::Translatef(-px, 0, 0);
    }
    const GLfloat dx = (GLfloat)(ColButton() - PropColLabel);
    w3dEngine::Translatef(dx, 0, 0);
    static const float grayDimmed[4] = {0.35f, 0.35f, 0.35f, 1.0f};
    const float* txtPrev = button->textColor;
    if (gray) button->textColor = grayDimmed; // texto+icono atenuados
    button->Render();
    if (gray) button->textColor = txtPrev;
    w3dEngine::Translatef(-dx, (GLfloat)Advance(), 0);
}

void PropButton::RenderPropertiBoxBorder(Card* propertiBox){
    if (hidden) return;
    // fila seleccionada con el teclado: borde resaltado sobre el boton (o sobre el chico / el cuadradito con foco)
    Button* withFocus = button; int col = ColButton();
    if (withLabel && focusKb >= 0 && focusKb < nExtras && extras[focusKb] && extras[focusKb]->visible) {
        withFocus = extras[focusKb]; col = ExtraX(focusKb);
    } else if (withLabel && focusKb == -2 && prefix && prefix->visible) {
        withFocus = prefix; col = colPrefix;
    }
    const GLfloat dx = (GLfloat)(col - PropColLabel);
    w3dEngine::Translatef(dx, (GLfloat)-Advance(), 0);
    withFocus->card->RenderBorder(false);
    w3dEngine::Translatef(-dx, (GLfloat)Advance(), 0);
}

void PropButton::RenderPropertiValue(Card* propertiBox){
    if (hidden) return;
    w3dEngine::Translatef(0, (GLfloat)Advance(), 0);
}

void PropButton::RenderPropertiLabel(Card* propertiBox){
    if (hidden) return;
    if (withLabel && inColumn) {   // la etiqueta como la de un input (la misma funcion, la misma columna)
        RenderBitmapLabel(name, PropColLabel - bordersGS);
        w3dEngine::Translatef(0, (GLfloat)Advance(), 0);
        return;
    }
    if (withLabel) {   // (el origen del pase es el final de la columna de etiquetas) al borde izquierdo del renglon; si no
                      // entra (panel angosto) se corta por el FINAL: "Modo de re", no "do de render"
        const GLfloat dx = (GLfloat)(bordersGS + gapGS * 2 - PropColLabel);
        w3dEngine::Translatef(dx, 0, 0);
        RenderBitmapText(name, textAlign::left, colButton - bordersGS - gapGS * 2);
        w3dEngine::Translatef(-dx, 0, 0);
    }
    w3dEngine::Translatef(0, (GLfloat)Advance(), 0);
}
