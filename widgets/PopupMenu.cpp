#include <stdio.h>   // snprintf: el valor entero de un slider ("100%")
#include "WhiskUI/text/font.h"   // UTF8_Largo
#include "WhiskUI/widgets/PopupMenu.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"

PopupMenu* OpenMenu = NULL;
int MenuScreenW = 100000;
int MenuScreenH = 100000;

// px que ocupa la COLUMNA DERECHA de un item (flecha submenu / tilde / slider / atajo).
// El atajo se dibuja a la IZQUIERDA de la flecha o la tilde (ver Render), por eso se suman.
// Se usa item-por-item: el ancho del menu es el MINIMO que hace entrar cada fila (no una
// columna unica compartida) y el label se recorta justo antes de ESTA columna.
std::string (*W3dMenuShortcutText)(const std::string&) = NULL;
void (*W3dPickedMenuHook)(const PopupMenu*, const MenuItem*) = NULL;
std::string MenuShortcutVisible(const MenuItem* it){
    if (!it || it->shortcut.empty()) return std::string();
    if (W3dMenuShortcutText) return W3dMenuShortcutText(it->shortcut);
    return (it->shortcut[0] == '@') ? std::string() : it->shortcut;
}
// EL SLIDER de una fila, en coordenadas del menu: la barra (barX, de 80 de ancho) y, con 'paso', el "-" a su izquierda
// (minusX) y el "+" a su derecha (moreX), cuadrados del alto del renglon (bw). El mismo lugar para dibujar y para tocar
static void GeomSlider(int widthMenu, const MenuItem* it, int& barX, int& minusX, int& moreX, int& bw){
    const int sliderW = 80 * GlobalScale;
    bw = (it->step > 0.0f) ? RowHeightGS : 0;
    const int g = (it->step > 0.0f) ? gapGS : 0;
    moreX = widthMenu - bordersGS - bw;
    barX  = moreX - g - sliderW;
    minusX = barX - g - bw;
}
static int MenuItemRight(const MenuItem* it){
    if (it->valueFloat) return 80 * GlobalScale + gapGS + (it->step > 0.0f ? 2 * (RowHeightGS + gapGS) : 0); // slider (no convive con atajo)
    int icon     = (it->submenu || it->checkbox) ? IconSizeGS : 0;
    int shortcut = UTF8_Length(MenuShortcutVisible(it)) * CharacterWidthGS;
    if (shortcut > 0) return (icon > 0) ? icon + gapGS + shortcut : shortcut;
    return icon;
}

// ---- DRAG del item-slider (AddFloat) ----
// El DOWN sobre la fila de un slider lo ARMA (dentro de Click, que ademas setea el
// valor como siempre -> un click corto sigue siendo click). Mientras el boton siga
// apretado, la app rutea cada motion a MenuSliderDragMove: la X del mouse se
// re-mapea a la barra (mismo mapeo que el click) y el valor cambia EN VIVO.
// El menu dueno de la fila puede ser un SUBMENU: se guarda el menu que la contiene.
static PopupMenu* g_sliderDragMenu = NULL; // menu dueno de la fila arrastrada (NULL = no hay drag)
static MenuItem*  g_sliderDragItem = NULL;

static PopupMenu* g_scrollDragMenu = NULL;
bool MenuScrollDragActive(){ return g_scrollDragMenu && g_scrollDragMenu->open; }
void MenuScrollDragMove(int my){ if (MenuScrollDragActive()) g_scrollDragMenu->ScrollFromY(my); }
void MenuScrollDragRelease(){ g_scrollDragMenu = NULL; }
bool MenuSliderDragActive(){
    return g_sliderDragMenu != NULL && g_sliderDragMenu->open &&
           g_sliderDragItem != NULL;
}

int MenuSliderDragMove(int mx){
    if (!MenuSliderDragActive()) return -1;
    MenuItem* it = g_sliderDragItem;
    if (!it->valueFloat) return -1;
    if (it->gray && !*it->gray) return -1; // se deshabilito en el medio: no editar
    int sliderW = 80 * GlobalScale;        // mismo ancho/mapeo que Click y Render
    int bx, mnx, msx, bw; GeomSlider(g_sliderDragMenu->width, it, bx, mnx, msx, bw);
    int barXscreen = g_sliderDragMenu->x + borderGS + bx;
    float frac = (float)(mx - barXscreen) / (float)sliderW;
    if (frac < 0.0f) frac = 0.0f; if (frac > 1.0f) frac = 1.0f;
    *it->valueFloat = it->vmin + frac * (it->vmax - it->vmin);
    return it->id;
}

void MenuSliderDragRelease(){
    g_sliderDragMenu = NULL;
    g_sliderDragItem = NULL;
}

MenuItem::MenuItem(const std::string& Text, int Id, int Icon, PopupMenu* Submenu,
                   bool* Checkbox){
    // (eran inicializadores de clase: C++03)
    lineUp = false;
    text = Text;
    id = Id;
    icon = Icon;
    submenu = Submenu;
    checkbox = Checkbox;
    gray = NULL;
    shortcut = "";
    green    = false;
    valueFloat = NULL;
    vmin = 0.0f; vmax = 1.0f;
    step = 0.0f; integer = false;
    action = NULL;
}

const char* (*W3dMenuTranslate)(const char*) = NULL;

PopupMenu::PopupMenu(){
    title = "";
    closeFar = true;
    x = 0;
    y = 0;
    width = 10;
    height = 10;
    reserveRight = 0;
    selectIndex = -1;
    scroll = 0;
    sbHover = false;
    open    = false;
    openSubmenu = NULL;
    parent = NULL;
    action = NULL;
    card = new Card(NULL, 10, 10);
    highlight = new Card(NULL, 10, RowHeightGS);
    sliderCard = new Card(NULL, 10, RowHeightGS);
}

PopupMenu::~PopupMenu(){
    if (g_sliderDragMenu == this) MenuSliderDragRelease();
    for (size_t i = 0; i < items.size(); i++) delete items[i];
    items.clear();
    delete card;
    delete highlight;
    delete sliderCard;
}

void PopupMenu::Clear(){
    if (g_sliderDragMenu == this) MenuSliderDragRelease(); // los items se borran: no dejar un puntero colgado
    for (size_t i = 0; i < items.size(); i++) delete items[i];
    items.clear();
    selectIndex = -1;
}

void PopupMenu::Build(const MenuDef* defs, int n){
    Clear();
    for (int i = 0; i < n; i++){
        const MenuDef& d = defs[i];
        const char* txt = (W3dMenuTranslate && d.text) ? W3dMenuTranslate(d.text) : (d.text ? d.text : "");
        const int ic = d.icon ? d.icon - 1 : -1;   // 0 = sin icono; los reales venian +1 (ICONO)
        MenuItem* it;
        if (d.checkbox)     it = AddCheck(txt, i, d.checkbox, ic);
        else if (d.submenu) it = Add(txt, i, ic, *d.submenu);
        else                it = Add(txt, i, ic);
        it->action = d.action;
        if (d.shortcut) it->shortcut = d.shortcut;
    }
}

bool PopupMenu::Run(int id){
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->id == id){
            if (items[i]->action){ items[i]->action(); return true; }  // declarativo: la accion vive en el item
            break;
        }
    }
    if (action) { action(id); return true; }   // menu del camino viejo (id -> switch): sigue funcionando igual
    return false;
}

MenuItem* PopupMenu::Add(const std::string& text, int id, int icon,
                             PopupMenu* submenu){
    MenuItem* it = new MenuItem(text, id, icon, submenu);
    items.push_back(it);
    return it;
}

MenuItem* PopupMenu::AddCheck(const std::string& text, int id, bool* state,
                                  int icon){
    MenuItem* it = new MenuItem(text, id, icon, NULL, state);
    items.push_back(it);
    return it;
}

MenuItem* PopupMenu::AddFloat(const std::string& text, int id, float* value,
                                  float vmin, float vmax, int icon){
    MenuItem* it = new MenuItem(text, id, icon, NULL, NULL);
    it->valueFloat = value;
    it->vmin = vmin; it->vmax = vmax;
    items.push_back(it);
    return it;
}

void PopupMenu::Resize(){
    bool hasIcons = false;
    // ANCHO MINIMO ITEM POR ITEM: para cada fila = label + (si tiene) su columna derecha
    // (flecha/tilde/slider/atajo). El ancho del menu es el MAYOR de esas filas, NO
    // maxLabel + maxColumnaDerecha por separado (eso lo hacia demasiado ancho cuando el
    // label mas largo y el atajo/slider mas ancho estaban en filas distintas -> "Overlays").
    int maxRow = 0;
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->icon >= 0) hasIcons = true;
        int row = UTF8_Length(items[i]->text) * CharacterWidthGS;
        int right = MenuItemRight(items[i]);
        if (right > 0) row += gapGS * 2 + right; // separacion label <-> columna derecha
        if (row > maxRow) maxRow = row;
    }
    if (UTF8_Length(title) * CharacterWidthGS > maxRow) // la cabecera no tiene columna derecha
        maxRow = UTF8_Length(title) * CharacterWidthGS;
    // si alguna fila tiene icono, TODAS reservan la columna izquierda (los labels se alinean)
    width = bordersGS + gapGS * 2 + maxRow;
    if (hasIcons) width += IconSizeGS + gapGS;
    reserveRight = 0; // ya no se usa una reserva unica: el recorte del label es por item (ver Render)
    // NUNCA mas ancho que la pantalla (Android/Symbian). Al achicarse, el label se recorta del
    // lado DERECHO (RenderBitmapText left), justo antes de la columna derecha de esa fila.
    int maxW = MenuScreenW - gapGS * 2;
    if (width > maxW) width = maxW;
    int rowH = RowHeightGS + gapGS;
    int titleH = title.empty() ? 0 : rowH;
    height = bordersGS + titleH + (int)items.size() * rowH - gapGS;
    // CLAMP: nunca mas alto que la pantalla (menu con 129 clips) -> se recorta a un nro ENTERO de filas y aparece
    // scrollbar. Sin esto el menu se salia por abajo y el hover caia en el viewport (abria el timeline). Con aire.
    int maxH = MenuScreenH - RowHeightGS;
    if (height > maxH){
        int visRows = (maxH - bordersGS - titleH + gapGS) / rowH; if (visRows < 1) visRows = 1;
        height = bordersGS + titleH + visRows * rowH - gapGS;
    }
    int ms = MaxScroll();
    if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;

    card->Resize(width, height);
    highlight->Resize(width - bordersGS, RowHeightGS + GlobalScale);
}

// cuantas filas de ITEMS entran en la ventana visible (segun el alto ya clampeado)
int PopupMenu::VisibleRows() const {
    int rowH = RowHeightGS + gapGS;
    int titleH = title.empty() ? 0 : rowH;
    int v = (height - bordersGS - titleH + gapGS) / rowH;
    return v < 1 ? 1 : v;
}
// la franja de la barra: el borde derecho del menu, ancha para apuntarle con el mouse o el dedo
bool PopupMenu::InBarScroll(int mx, int my) const {
    if (!open || MaxScroll() <= 0) return false;
    const int barWidth = GlobalScale * 9;
    return mx >= x + width - borderGS - barWidth && mx <= x + width && my >= y && my <= y + height;
}
void PopupMenu::ScrollFromY(int my) {
    const int ms = MaxScroll();
    if (ms <= 0) return;
    const int rowH = RowHeightGS + gapGS;
    const int titleH = title.empty() ? 0 : rowH;
    const int trackTop = y + borderGS + titleH, trackH = height - bordersGS - titleH;
    const int nItems = items.empty() ? 1 : (int)items.size();
    int thumbH = trackH * VisibleRows() / nItems; if (thumbH < GlobalScale * 6) thumbH = GlobalScale * 6;
    const int range = trackH - thumbH;
    int t = my - trackTop - thumbH / 2;
    if (t < 0) t = 0; if (t > range) t = range;
    scroll = range > 0 ? (t * ms + range / 2) / range : 0;
    if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;
}
int PopupMenu::MaxScroll() const {
    int m = (int)items.size() - VisibleRows();
    return m < 0 ? 0 : m;
}
void PopupMenu::Wheel(int dir){
    if (openSubmenu && openSubmenu->open){ openSubmenu->Wheel(dir); return; } // el submenu es el foco
    int ms = MaxScroll();
    scroll -= dir; // rueda arriba (dir +1) -> ver items ANTERIORES (scroll baja)
    if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;
}
void PopupMenu::EnsureVisible(){
    if (selectIndex < 0) return;
    if (selectIndex < scroll) scroll = selectIndex;
    else { int v = VisibleRows(); if (selectIndex >= scroll + v) scroll = selectIndex - v + 1; }
    int ms = MaxScroll(); if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;
}

void PopupMenu::Open(int sx, int sy, int screenW, int screenH){
    scroll = 0; // arranca desde arriba
    parent = NULL; // (abierto solo: de nivel tope. Como submenu, OpenSubmenuBeside lo vuelve a enganchar)
    Resize();
    x = sx;
    y = sy;
    if (x + width > screenW) x = screenW - width;
    if (y + height > screenH) y = screenH - height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    selectIndex = -1;
    open = true;
    // una lista LARGA con la opcion ACTIVA (verde: la animacion que se esta editando entre cientos de clips) abre
    // MOSTRANDOLA, centrada, en vez de arrancar siempre desde arriba
    if (MaxScroll() > 0)
        for (size_t i = 0; i < items.size(); i++)
            if (items[i] && items[i]->green) {
                int s = (int)i - VisibleRows() / 2;
                const int ms = MaxScroll();
                if (s > ms) s = ms;
                if (s < 0) s = 0;
                scroll = s;
                break;
            }
}

void PopupMenu::WidthMax(int w){
    if (w < IconSizeGS) w = IconSizeGS;
    if (w >= width) return;
    width = w;
    card->Resize(width, height);
    highlight->Resize(width - bordersGS, RowHeightGS + GlobalScale);
}

// donde va un SUBMENU: a la DERECHA de este menu si entra; si no, a la IZQUIERDA; si no entra de ningun lado, del lado
// con mas lugar, angostado (el texto se recorta) para que quede SIEMPRE a la vista una COLUMNA de este menu: por ahi se
// pasa el mouse a las otras opciones. Antes se clampeaba contra el borde y quedaba ENCIMA del padre entero (y del
// abuelo): con menus largos (las mallas por carpeta, sus partes) no se podia ir a otra opcion
void PopupMenu::OpenSubmenuBeside(PopupMenu* sub, int row){
    if (!sub) return;
    const int oy = title.empty() ? 0 : (RowHeightGS + gapGS);
    const int sy = y + borderGS + oy + (row - scroll) * (RowHeightGS + gapGS);   // la fila VISIBLE
    sub->Open(x + width - gapGS, sy, MenuScreenW, MenuScreenH);   // (mide su ancho y acomoda la altura)
    sub->parent = this;
    const int W = MenuScreenW;
    const int right = x + width - gapGS, left = x + gapGS - sub->width;
    if (right + sub->width <= W) { sub->x = right; return; }
    if (left >= 0) { sub->x = left; return; }
    int col = width / 3;                       // la columna de este menu que queda a la vista
    if (col < IconSizeGS * 3) col = IconSizeGS * 3;
    if (col > width) col = width;
    if (W - (x + width) >= x) {                // mas lugar a la derecha: se ve la columna IZQUIERDA de este menu
        sub->x = x + col;
        if (sub->x + sub->width > W) sub->WidthMax(W - sub->x);
    } else {                                   // a la izquierda: se ve la columna DERECHA
        const int end = x + width - col;
        sub->x = end - sub->width;
        if (sub->x < 0) { sub->x = 0; sub->WidthMax(end); }
    }
}

void PopupMenu::Close(){
    if (g_sliderDragMenu == this) MenuSliderDragRelease(); // menu cerrado (ESC, etc): corta el drag
    if (g_scrollDragMenu == this) g_scrollDragMenu = NULL;
    sbHover = false;
    open    = false;
    selectIndex = -1;
    if (openSubmenu){
        openSubmenu->Close();
        openSubmenu = NULL;
    }
    if (OpenMenu == this) OpenMenu = NULL;
}

bool PopupMenu::Contains(int mx, int my) const {
    return mx >= x && mx < x + width && my >= y && my < y + height;
}
bool PopupMenu::ContainsWithSubmenus(int mx, int my) const {
    if (Contains(mx, my)) return true;
    return openSubmenu && openSubmenu->open && openSubmenu->ContainsWithSubmenus(mx, my);
}

void PopupMenu::Render(){
    bool hasIcons = false;
    bool hasSub = false;
    bool hasCheck = false;
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->icon >= 0) hasIcons = true;
        if (items[i]->submenu) hasSub = true;
        if (items[i]->checkbox) hasCheck = true;
    }

    const float* gray = ColorList[static_cast<int>(ColorID::gray)];
    const float* background = ColorList[static_cast<int>(ColorID::background)];

    w3dEngine::PushMatrix();
    w3dEngine::Translatef((GLfloat)x, (GLfloat)y, 0);

    // fondo: tarjeta con BORDE VERDE (el menu activo es el foco)
    w3dEngine::Color4f(gray[0], gray[1], gray[2], 1.0f);
    card->RenderObject(false);
    SetColorID(ColorID::accent, 1.0f);
    card->RenderBorder(false);

    w3dEngine::Translatef((GLfloat)borderGS, (GLfloat)borderGS, 0);
    if (!title.empty()){
        // cabecera
        SetColorID(ColorID::white, 1.0f);
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
        RenderBitmapText(title, textAlign::left, width - bordersGS);
        w3dEngine::PopMatrix();
        // LINEA separadora bajo el titulo: un gris MAS CLARO que el fondo del menu
        // (el fondo es 'gris' 0.12; el negro no se veia -> grayLine 0x494949).
        {
            const float* ac = ColorList[static_cast<int>(ColorID::grayLine)];
            float y0 = (float)RowHeightGS, y1 = y0 + 2.0f;
            float x0 = (float)gapGS, x1 = (float)(width - bordersGS);
            GLfloat sep[18] = { x0,y0,0,  x1,y0,0,  x1,y1,0,
                                x0,y0,0,  x1,y1,0,  x0,y1,0 };
            w3dEngine::Color4f(ac[0], ac[1], ac[2], 1.0f);
            w3dEngine::Disable(w3dEngine::Texture2D);
            w3dEngine::DisableArray(w3dEngine::TexCoordArray);
            w3dEngine::VertexPointer3f(0, sep);
            w3dEngine::DrawTrianglesArray(6);
            w3dEngine::EnableArray(w3dEngine::TexCoordArray);
            w3dEngine::Enable(w3dEngine::Texture2D);
        }
        w3dEngine::Translatef(0, (GLfloat)(RowHeightGS + gapGS), 0);
    }
    // solo las filas VISIBLES [scroll .. scroll+VisibleRows): asi el menu no se sale de la pantalla
    int _first = scroll, _last = scroll + VisibleRows();
    if (_first < 0) _first = 0; if (_last > (int)items.size()) _last = (int)items.size();
    for (int i = _first; i < _last; i++){
        // la LINEA separadora de un grupo (en el gap de arriba de la fila; el mismo gris que la del titulo)
        if (items[i]->lineUp && i > _first) {
            const float* ac = ColorList[static_cast<int>(ColorID::grayLine)];
            const float y1 = (float)(-gapGS / 2 + 1), y0 = y1 - 1.0f;   // 1 px de alto (exacto, no escalado)
            const float x0 = (float)gapGS, x1 = (float)(width - bordersGS);
            GLfloat sep[18] = { x0,y0,0,  x1,y0,0,  x1,y1,0,
                                x0,y0,0,  x1,y1,0,  x0,y1,0 };
            w3dEngine::Color4f(ac[0], ac[1], ac[2], 1.0f);
            w3dEngine::Disable(w3dEngine::Texture2D);
            w3dEngine::DisableArray(w3dEngine::TexCoordArray);
            w3dEngine::VertexPointer3f(0, sep);
            w3dEngine::DrawTrianglesArray(6);
            w3dEngine::EnableArray(w3dEngine::TexCoordArray);
            w3dEngine::Enable(w3dEngine::Texture2D);
        }
        bool highlighted = (i == selectIndex);
        if (highlighted){
            SetColorID(ColorID::headerColor, 1.0f);
            highlight->RenderObject(false);
        }
        bool disabled = (items[i]->gray && !*items[i]->gray);
        if (disabled){
            // depende de un master apagado (ej: overlays off): NO desaparece,
            // se ve al 50% transparente (mismo color de texto, media opacidad)
            SetColorID(ColorID::grayUI, 0.5f);
        } else if (items[i]->green){
            // opcion ACTIVA (ej: el modo de pivot actual): icono + texto en VERDE
            SetColorID(ColorID::accent, 1.0f);
        } else if (highlighted){
            SetColorID(ColorID::white, 1.0f);
        } else {
            SetColorID(ColorID::grayUI, 1.0f);
        }
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
        if (items[i]->icon >= 0){
            // 1 px MAS ABAJO (exacto, no escalado): asi queda centrado en vertical con el texto de la fila
            w3dEngine::PushMatrix();
            w3dEngine::Translatef(0, 1.0f, 0);
            W3dDrawStrip4(IconMesh, IconsUV[items[i]->icon]->uvs);
            w3dEngine::PopMatrix();
        }
        int textStartX = gapGS + (hasIcons ? IconSizeGS + gapGS : 0);
        if (hasIcons) w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
        // el texto se corta ANTES de la columna derecha de ESTA fila (tildes/slider/atajo) ->
        // recorte del lado DERECHO. Reserva por item (no una columna unica compartida).
        int rightItem = MenuItemRight(items[i]);
        int labelMax = width - textStartX - (rightItem > 0 ? rightItem + gapGS * 2 : 0) - gapGS;
        if (labelMax < CharacterWidthGS) labelMax = CharacterWidthGS;
        RenderBitmapText(items[i]->text, textAlign::left, labelMax);
        w3dEngine::PopMatrix();
        if (items[i]->submenu){
            // indicador de submenu a la derecha
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - bordersGS - IconSizeGS), 0, 0);
            W3dDrawStrip4(IconMesh,
                IconsUV[static_cast<size_t>(IconType::arrowRight)]->uvs);
            w3dEngine::PopMatrix();
        }
        if (items[i]->checkbox){
            // checkbox a la derecha: EL del editor (caja redondeada + tilde), el mismo del panel de propiedades.
            // Antes era un tilde SUELTO: sin la tilde no se distinguia de un boton comun.
            int side = CheckboxSide();
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - bordersGS - side), 0, 0);
            DrawCheckbox(side, *items[i]->checkbox);
            w3dEngine::PopMatrix();
        }
        if (items[i]->valueFloat){
            // SLIDER a la derecha: track + relleno (fraccion) + valor centrado
            float range = items[i]->vmax - items[i]->vmin;
            float frac = range > 0.0f ? (*items[i]->valueFloat - items[i]->vmin) / range : 0.0f;
            if (frac < 0.0f) frac = 0.0f; if (frac > 1.0f) frac = 1.0f;
            int sliderW = 80 * GlobalScale;
            int barX, minusX, moreX, bw; GeomSlider(width, items[i], barX, minusX, moreX, bw);
            int barH = RowHeightGS - 2 * GlobalScale;
            if (items[i]->step > 0.0f){
                // el "-" y el "+" (cajitas del alto del renglon, como los botones de la UI)
                const int xs[2] = { minusX, moreX }; const char* ts[2] = { "-", "+" };
                for (int b = 0; b < 2; b++){
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)xs[b], (GLfloat)GlobalScale, 0);
                    SetColorID(ColorID::headerColor, 1.0f);
                    sliderCard->Resize(bw, barH); sliderCard->RenderObject(false);
                    w3dEngine::PopMatrix();
                    SetColorID(ColorID::white, 1.0f);
                    w3dEngine::PushMatrix();
                    w3dEngine::Translatef((GLfloat)xs[b], 0, 0);
                    RenderBitmapText(ts[b], textAlign::center, bw);
                    w3dEngine::PopMatrix();
                }
            }
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)barX, (GLfloat)GlobalScale, 0);
            SetColorID(ColorID::headerColor, 1.0f);
            sliderCard->Resize(sliderW, barH); sliderCard->RenderObject(false);
            int fillW = (int)(frac * sliderW);
            if (fillW > 2){
                SetColorID(ColorID::accent, 1.0f);
                sliderCard->Resize(fillW, barH); sliderCard->RenderObject(false);
            }
            w3dEngine::PopMatrix();
            SetColorID(ColorID::white, 1.0f);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)barX, 0, 0);
            if (items[i]->integer){
                char b[32]; snprintf(b, sizeof(b), "%d%s", (int)(*items[i]->valueFloat + (*items[i]->valueFloat < 0 ? -0.5f : 0.5f)), items[i]->unit.c_str());
                RenderBitmapText(b, textAlign::center, sliderW);
            } else RenderBitmapFloat(*items[i]->valueFloat, textAlign::center, sliderW);
            w3dEngine::PopMatrix();
        }
        const std::string shortcutTxt = MenuShortcutVisible(items[i]);
        if (!shortcutTxt.empty()){
            // hint del atajo: gris tenue (un poco mas oscuro que el texto),
            // alineado al borde derecho del menu (igual en todos los menus)
            SetColorID(ColorID::grayUI, 0.5f);
            int shortcutDX = width - bordersGS - gapGS;
            if (items[i]->submenu) shortcutDX -= IconSizeGS + gapGS; // dejar la flecha del submenu a la derecha del atajo
            if (items[i]->checkbox) shortcutDX -= CheckboxSide() + gapGS; // dejar el checkbox a la derecha del atajo (no superponer)
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)shortcutDX, 0, 0);
            RenderBitmapText(shortcutTxt, textAlign::right, width);
            w3dEngine::PopMatrix();
        }
        w3dEngine::Translatef(0, (GLfloat)(RowHeightGS + gapGS), 0);
    }
    w3dEngine::PopMatrix();

    // SCROLLBAR (si hay mas items que los visibles): thumb proporcional pegado al borde derecho
    if (MaxScroll() > 0){
        int rowH = RowHeightGS + gapGS;
        int titleH = title.empty() ? 0 : rowH;
        int trackTop = borderGS + titleH, trackH = height - bordersGS - titleH;
        int nItems = items.empty() ? 1 : (int)items.size();
        int thumbH = trackH * VisibleRows() / nItems; if (thumbH < GlobalScale * 6) thumbH = GlobalScale * 6;
        int ms = MaxScroll();
        int thumbY = trackTop + (ms > 0 ? scroll * (trackH - thumbH) / ms : 0);
        // (como las barras de los paneles: con el puntero encima se agranda y va en blanco; agarrada, en verde)
        const bool grabbed = (g_scrollDragMenu == this);
        int sbW = GlobalScale * ((sbHover || grabbed) ? 5 : 2), sbX = width - borderGS - sbW;
        const float* ac = ColorList[static_cast<int>(grabbed ? ColorID::accent : (sbHover ? ColorID::white : ColorID::grayUI))];
        float x0 = (float)sbX, x1 = (float)(sbX + sbW), y0 = (float)thumbY, y1 = (float)(thumbY + thumbH);
        GLfloat sb[18] = { x0,y0,0, x1,y0,0, x1,y1,0,  x0,y0,0, x1,y1,0, x0,y1,0 };
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)x, (GLfloat)y, 0);
        w3dEngine::Color4f(ac[0], ac[1], ac[2], (sbHover || grabbed) ? 1.0f : 0.85f);
        w3dEngine::Disable(w3dEngine::Texture2D);
        w3dEngine::DisableArray(w3dEngine::TexCoordArray);
        w3dEngine::VertexPointer3f(0, sb);
        w3dEngine::DrawTrianglesArray(6);
        w3dEngine::EnableArray(w3dEngine::TexCoordArray);
        w3dEngine::Enable(w3dEngine::Texture2D);
        w3dEngine::PopMatrix();
    }

    // el submenu abierto se dibuja encima
    if (openSubmenu && openSubmenu->open) openSubmenu->Render();
}

// abre/cierra el submenu segun la fila resaltada: la opcion con submenu lo
// abre con solo resaltarla (mouse o flechas); al pasar a otra fila se cierra
// (o se abre el submenu de la nueva). Sin necesidad de click.
// TACTIL: un submenu se abre por "hover" (el dedo al apoyarse). En pantallas angostas el submenu se clampea
// y SOLAPA al padre, justo bajo el dedo -> el click del MISMO toque caia en un item del submenu (se apretaba
// solo). Este flag marca "recien abierto": el click inmediato NO selecciona, solo deja el submenu abierto.
// Se limpia al MOVER el puntero DENTRO del submenu (asi con mouse, mover+click sigue seleccionando normal).
static bool g_submenuJustOpen = false;
// un TAP DELIBERADO sobre un item (ej: soltar sobre un clip de un submenu SCROLLABLE, ruteado por el drag-soltar) NO
// debe bloquearse por este guard: el guard es solo para el MISMO toque que ABRE el submenu solapado bajo el dedo. Sin
// esto, en el desplegable de las PROPIEDADES (panel derecho) el submenu se abre a la IZQUIERDA sobre el cursor y el
// tap caia en el guard -> "no se selecciona, queda en Escena". El timeline (submenu hacia arriba) no lo sufria.
void MenuClearGuardOpen(){ g_submenuJustOpen = false; }

void PopupMenu::SyncSubmenu(){
    MenuItem* it = (selectIndex >= 0 && selectIndex < (int)items.size())
                   ? items[selectIndex] : NULL;
    PopupMenu* sub = it ? it->submenu : NULL;
    if (openSubmenu == sub) return; // ya esta como corresponde
    if (openSubmenu){ openSubmenu->Close(); openSubmenu = NULL; }
    if (sub){
        OpenSubmenuBeside(sub, selectIndex);
        openSubmenu = sub;
        g_submenuJustOpen = true; // el proximo click sobre este submenu solo lo deja abierto (no selecciona)
    }
}

bool PopupMenu::MouseMove(int mx, int my){
    if (!open) return false;
    // PRIORIDAD al submenu abierto: si el mouse cae sobre el, lo maneja el submenu y NO se recalcula la fila de
    // ESTE menu. Los submenus anchos se SOLAPAN sobre el padre al clampearse contra el borde de pantalla; sin esta
    // prioridad, moverse por el submenu caia dentro del bounding box del padre -> le cambiaba la opcion y cerraba
    // el submenu (no se podia usar "Set Parent To"). El submenu se dibuja ENCIMA, asi que el hover es suyo.
    if (openSubmenu && openSubmenu->open && openSubmenu->Contains(mx, my)){
        g_submenuJustOpen = false; // el puntero se movio DENTRO del submenu: ya es un target deliberado
        openSubmenu->MouseMove(mx, my);
        return true;
    }
    // si el mouse esta sobre ESTE menu (y no sobre el submenu), manda este menu: resalta la fila y
    // abre/cierra el submenu segun corresponda (sin click)
    if (Contains(mx, my)){
        // sobre la BARRA DE SCROLL: se marca (no resalta ninguna opcion)
        sbHover = InBarScroll(mx, my);
        if (sbHover) return true;
        int oy = title.empty() ? 0 : (RowHeightGS + gapGS);
        int row = scroll + (my - y - borderGS - oy) / (RowHeightGS + gapGS); // + scroll: fila absoluta
        if (row < 0) row = 0;
        if (row >= (int)items.size()) row = (int)items.size() - 1;
        selectIndex = row;
        SyncSubmenu();
        return true;
    }
    sbHover = false;
    // fuera de este menu: el mouse puede estar sobre un sub-submenu mas profundo (que se extiende mas alla del submenu directo)
    if (openSubmenu && openSubmenu->open){
        if (openSubmenu->MouseMove(mx, my)) return true;
    }
    // lejos de todo: el menu se cierra solo (salvo el que no se abrio en el mouse, ver closeFar)
    int margin = 60 * GlobalScale;
    if (closeFar && (mx < x - margin || mx > x + width + margin ||
        my < y - margin || my > y + height + margin)){
        Close();
    }
    return open;
}

int PopupMenu::Click(int mx, int my){
    // el click en el submenu abierto O EN UNO DE SUS SUBMENUS (a cualquier profundidad). Antes solo se miraba el
    // submenu DIRECTO: un click en el nieto (Add > Core > Draw > Draw mesh) le parecia a este menu un click
    // AFUERA, lo cerraba todo y no se creaba nada
    if (openSubmenu && openSubmenu->open &&
        openSubmenu->ContainsWithSubmenus(mx, my)){
        // el submenu se ACABA de abrir por este mismo toque (y quedo solapado bajo el dedo): NO seleccionar,
        // solo dejarlo abierto. El proximo toque ya si elige un item (tactil = 2 toques para submenus).
        if (g_submenuJustOpen){ g_submenuJustOpen = false; return -1; }
        int id = openSubmenu->Click(mx, my);
        // si el submenu selecciono algo terminal (se cerro solo), lo ejecuto EN el submenu (dueno del id) y
        // cierro tambien ESTE menu. NO subo el id: colisiona por posicion con un item del padre (era el bug
        // de "importar GLB -> Circle"). Ver la nota en Enter(). Si lo eligio un submenu MAS profundo, ese ya
        // lo ejecuto (devuelve -1) y cerro su cadena: aca solo queda cerrar este.
        if (!openSubmenu->open){
            PopupMenu* sub = openSubmenu;
            openSubmenu    = NULL;
            Close();
            // un submenu SIN accion propia (ni en sus items ni en el menu: Definir Origen, Aplicar, Transformar,
            // Punto de Vista...) le deja el id a ESTE menu, como antes: si no, el click no hacia nada
            if (id >= 0 && !sub->Run(id) && action) action(id);
            return -1; // ya ejecutado
        }
        return id;
    }
    if (!Contains(mx, my)){
        Close(); // click afuera: cierra (y el click no llega atras)
        return -1;
    }
    // la BARRA DE SCROLL: el agarre salta a donde se apreto y queda agarrado (arrastrar lo sigue moviendo)
    if (InBarScroll(mx, my)) { ScrollFromY(my); g_scrollDragMenu = this; sbHover = true; return -1; }
    int oy = title.empty() ? 0 : (RowHeightGS + gapGS);
    int row = scroll + (my - y - borderGS - oy) / (RowHeightGS + gapGS); // + scroll: fila absoluta
    if (row < 0 || row >= (int)items.size()) return -1;
    selectIndex = row;
    // SLIDER: click en la barra setea el valor segun la X (no cierra el menu)
    MenuItem* itF = items[row];
    if (itF->valueFloat && !(itF->gray && !*itF->gray)){
        int sliderW = 80 * GlobalScale;
        int bx, mnx, msx, bw; GeomSlider(width, itF, bx, mnx, msx, bw);
        // el "-" / el "+": un paso (no arma el arrastre)
        if (itF->step > 0.0f){
            const int lx = mx - x - borderGS;
            if (lx >= mnx && lx < mnx + bw) { float v = *itF->valueFloat - itF->step; if (v < itF->vmin) v = itF->vmin; *itF->valueFloat = v; return itF->id; }
            if (lx >= msx && lx < msx + bw) { float v = *itF->valueFloat + itF->step; if (v > itF->vmax) v = itF->vmax; *itF->valueFloat = v; return itF->id; }
            if (lx < bx) return -1;   // (el nombre de la fila: nada)
        }
        int barXscreen = x + borderGS + bx;
        float frac = (float)(mx - barXscreen) / (float)sliderW;
        if (frac < 0.0f) frac = 0.0f; if (frac > 1.0f) frac = 1.0f;
        *itF->valueFloat = itF->vmin + frac * (itF->vmax - itF->vmin);
        // ARMA el drag: si el boton sigue apretado, los motion siguientes re-mapean
        // la X a la barra y editan el valor en vivo (MenuSliderDragMove)
        g_sliderDragMenu = this; g_sliderDragItem = itF;
        return itF->id;
    }
    // un click en una fila de ESTE menu es para ESA fila. Enter() le pasa la tecla al submenu abierto (la navegacion con
    // el teclado: Enter con el submenu abierto elige SU opcion resaltada) y el submenu de "Malla" ya estaba abierto por el
    // dedo apoyado (el resaltado abre submenus, como el hover del mouse) con su primera opcion resaltada: tocar "Malla" en
    // el telefono agregaba un Plano. Ahora la fila con submenu lo abre (o lo deja abierto) y no elige nada; otra fila
    // cierra el submenu que hubiera abierto y se elige ella
    if (itF->submenu){
        if (openSubmenu == itF->submenu && openSubmenu->open) return -1;   // ya abierto: queda asi
        if (openSubmenu){ openSubmenu->Close(); openSubmenu = NULL; }
        return Enter();   // (lo abre al lado: -2)
    }
    if (openSubmenu){ openSubmenu->Close(); openSubmenu = NULL; }
    return Enter();
}

int PopupMenu::Enter(){
    if (openSubmenu && openSubmenu->open){
        int id = openSubmenu->Enter();
        if (id >= 0 && !openSubmenu->open){
            // el id pertenece al SUBMENU (es el dueno del item elegido). Hay que ejecutarlo EN el submenu,
            // NO devolverlo al padre: los ids son por-posicion y colisionan (GLB=3 del submenu Imports caia
            // en Circle=3 del menu Add -> "importar GLB" creaba un circulo). Cierro primero (como el camino
            // de un item de nivel tope) y despues ejecuto, por si la accion abre otro popup (file browser).
            PopupMenu* sub = openSubmenu;
            openSubmenu    = NULL;
            Close();
            if (!sub->Run(id) && action) action(id);   // (submenu sin accion propia: ver Click)
            return -1; // ya ejecutado: el llamador (if id>=0 Ejecutar) no re-despacha
        }
        return id;
    }
    if (selectIndex < 0 || selectIndex >= (int)items.size()) return -1;
    MenuItem* it = items[selectIndex];
    // deshabilitada (depende de un master apagado): no responde al click/enter
    if (it->gray && !*it->gray) return -1;
    if (it->submenu){
        // abre el submenu pegado a la derecha de la fila (+ pre-selecciona su 1ra)
        OpenSubmenuCurrent();
        return -2;
    }
    if (it->checkbox){
        // togglea el bool y DEJA el menu abierto (para prender/apagar varios)
        *it->checkbox = !*it->checkbox;
        if (W3dPickedMenuHook) W3dPickedMenuHook(this, it);
        return it->id;
    }
    if (W3dPickedMenuHook) W3dPickedMenuHook(this, it);   // (antes de cerrar: la ruta se lee de los padres abiertos)
    int id = it->id;
    Close();
    return id;
}

void PopupMenu::button_down(){
    // las flechas SOLO mueven el resaltado; el submenu NO se auto-abre (se abre
    // con derecha/enter/ok via OpenSubmenuCurrent). Si habia uno abierto, se cierra.
    if (openSubmenu){ openSubmenu->Close(); openSubmenu = NULL; }
    selectIndex++;
    if (selectIndex >= (int)items.size()) selectIndex = 0;
    EnsureVisible(); // scroll para que el resaltado quede a la vista (menus largos)
}

void PopupMenu::button_up(){
    if (openSubmenu){ openSubmenu->Close(); openSubmenu = NULL; }
    selectIndex--;
    if (selectIndex < 0) selectIndex = (int)items.size() - 1;
    EnsureVisible();
}

MenuItem* PopupMenu::CurrentItem(){
    if (selectIndex < 0 || selectIndex >= (int)items.size()) return NULL;
    return items[selectIndex];
}

// +1 (derecha) / -1 (izquierda): mueve el valor del slider resaltado un paso
void PopupMenu::FitSlider(int dir){
    MenuItem* it = CurrentItem();
    if (!it || !it->valueFloat) return;
    if (it->gray && !*it->gray) return; // deshabilitado
    float step = it->step > 0.0f ? it->step : (it->vmax - it->vmin) / 20.0f;
    if (step <= 0.0f) step = 0.01f;
    float v = *it->valueFloat + (float)dir * step;
    if (v < it->vmin) v = it->vmin;
    if (v > it->vmax) v = it->vmax;
    *it->valueFloat = v;
}

// abre el submenu de la opcion resaltada (pegado a la derecha) y deja su 1ra fila
// pre-seleccionada para poder moverte enseguida con las flechas. false si no hay.
bool PopupMenu::OpenSubmenuCurrent(){
    MenuItem* it = CurrentItem();
    if (!it || !it->submenu) return false;
    if (it->gray && !*it->gray) return false; // deshabilitado
    OpenSubmenuBeside(it->submenu, selectIndex);
    it->submenu->selectIndex = 0; // pre-seleccion (Abrir lo deja en -1)
    openSubmenu = it->submenu;
    return true;
}
