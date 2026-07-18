#include "WhiskUI/widgets/PopupMenu.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"

PopupMenu* MenuAbierto = NULL;
int MenuPantallaW = 100000;
int MenuPantallaH = 100000;

// px que ocupa la COLUMNA DERECHA de un item (flecha submenu / tilde / slider / atajo).
// El atajo se dibuja a la IZQUIERDA de la flecha o la tilde (ver Render), por eso se suman.
// Se usa item-por-item: el ancho del menu es el MINIMO que hace entrar cada fila (no una
// columna unica compartida) y el label se recorta justo antes de ESTA columna.
static int MenuItemDerecha(const MenuItem* it){
    if (it->valorFloat) return 80 * GlobalScale + gapGS; // slider (no convive con atajo)
    int icono = (it->submenu || it->checkbox) ? IconSizeGS : 0;
    int atajo = (int)it->atajo.size() * CharacterWidthGS;
    if (atajo > 0) return (icono > 0) ? icono + gapGS + atajo : atajo;
    return icono;
}

MenuItem::MenuItem(const std::string& Text, int Id, int Icon, PopupMenu* Submenu,
                   bool* Checkbox){
    // (eran inicializadores de clase: C++03)
    text = Text;
    id = Id;
    icon = Icon;
    submenu = Submenu;
    checkbox = Checkbox;
    gris = NULL;
    atajo = "";
    verde = false;
    valorFloat = NULL;
    vmin = 0.0f; vmax = 1.0f;
    accion = NULL;
}

const char* (*W3dMenuTraducir)(const char*) = NULL;

PopupMenu::PopupMenu(){
    titulo = "";
    x = 0;
    y = 0;
    width = 10;
    height = 10;
    reservaDerecha = 0;
    selectIndex = -1;
    scroll = 0;
    abierto = false;
    submenuAbierto = NULL;
    action = NULL;
    card = new Card(NULL, 10, 10);
    highlight = new Card(NULL, 10, RenglonHeightGS);
    sliderCard = new Card(NULL, 10, RenglonHeightGS);
}

PopupMenu::~PopupMenu(){
    for (size_t i = 0; i < items.size(); i++) delete items[i];
    items.clear();
    delete card;
    delete highlight;
    delete sliderCard;
}

void PopupMenu::Limpiar(){
    for (size_t i = 0; i < items.size(); i++) delete items[i];
    items.clear();
    selectIndex = -1;
}

void PopupMenu::Construir(const MenuDef* defs, int n){
    Limpiar();
    for (int i = 0; i < n; i++){
        const MenuDef& d = defs[i];
        const char* txt = (W3dMenuTraducir && d.text) ? W3dMenuTraducir(d.text) : (d.text ? d.text : "");
        const int ic = d.icon ? d.icon - 1 : -1;   // 0 = sin icono; los reales venian +1 (ICONO)
        MenuItem* it;
        if (d.checkbox)     it = AgregarCheck(txt, i, d.checkbox, ic);
        else if (d.submenu) it = Agregar(txt, i, ic, *d.submenu);
        else                it = Agregar(txt, i, ic);
        it->accion = d.accion;
        if (d.atajo) it->atajo = d.atajo;
    }
}

void PopupMenu::Ejecutar(int id){
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->id == id){
            if (items[i]->accion){ items[i]->accion(); return; }  // declarativo: la accion vive en el item
            break;
        }
    }
    if (action) action(id);   // menu del camino viejo (id -> switch): sigue funcionando igual
}

MenuItem* PopupMenu::Agregar(const std::string& text, int id, int icon,
                             PopupMenu* submenu){
    MenuItem* it = new MenuItem(text, id, icon, submenu);
    items.push_back(it);
    return it;
}

MenuItem* PopupMenu::AgregarCheck(const std::string& text, int id, bool* estado,
                                  int icon){
    MenuItem* it = new MenuItem(text, id, icon, NULL, estado);
    items.push_back(it);
    return it;
}

MenuItem* PopupMenu::AgregarFloat(const std::string& text, int id, float* valor,
                                  float vmin, float vmax, int icon){
    MenuItem* it = new MenuItem(text, id, icon, NULL, NULL);
    it->valorFloat = valor;
    it->vmin = vmin; it->vmax = vmax;
    items.push_back(it);
    return it;
}

void PopupMenu::Resize(){
    bool hayIconos = false;
    // ANCHO MINIMO ITEM POR ITEM: para cada fila = label + (si tiene) su columna derecha
    // (flecha/tilde/slider/atajo). El ancho del menu es el MAYOR de esas filas, NO
    // maxLabel + maxColumnaDerecha por separado (eso lo hacia demasiado ancho cuando el
    // label mas largo y el atajo/slider mas ancho estaban en filas distintas -> "Overlays").
    int maxFila = 0;
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->icon >= 0) hayIconos = true;
        int fila = (int)items[i]->text.size() * CharacterWidthGS;
        int der = MenuItemDerecha(items[i]);
        if (der > 0) fila += gapGS * 2 + der; // separacion label <-> columna derecha
        if (fila > maxFila) maxFila = fila;
    }
    if ((int)titulo.size() * CharacterWidthGS > maxFila) // la cabecera no tiene columna derecha
        maxFila = (int)titulo.size() * CharacterWidthGS;
    // si alguna fila tiene icono, TODAS reservan la columna izquierda (los labels se alinean)
    width = bordersGS + gapGS * 2 + maxFila;
    if (hayIconos) width += IconSizeGS + gapGS;
    reservaDerecha = 0; // ya no se usa una reserva unica: el recorte del label es por item (ver Render)
    // NUNCA mas ancho que la pantalla (Android/Symbian). Al achicarse, el label se recorta del
    // lado DERECHO (RenderBitmapText left), justo antes de la columna derecha de esa fila.
    int maxW = MenuPantallaW - gapGS * 2;
    if (width > maxW) width = maxW;
    int rowH = RenglonHeightGS + gapGS;
    int tituloH = titulo.empty() ? 0 : rowH;
    height = bordersGS + tituloH + (int)items.size() * rowH - gapGS;
    // CLAMP: nunca mas alto que la pantalla (menu con 129 clips) -> se recorta a un nro ENTERO de filas y aparece
    // scrollbar. Sin esto el menu se salia por abajo y el hover caia en el viewport (abria el timeline). Con aire.
    int maxH = MenuPantallaH - RenglonHeightGS;
    if (height > maxH){
        int visRows = (maxH - bordersGS - tituloH + gapGS) / rowH; if (visRows < 1) visRows = 1;
        height = bordersGS + tituloH + visRows * rowH - gapGS;
    }
    int ms = MaxScroll();
    if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;

    card->Resize(width, height);
    highlight->Resize(width - bordersGS, RenglonHeightGS + GlobalScale);
}

// cuantas filas de ITEMS entran en la ventana visible (segun el alto ya clampeado)
int PopupMenu::FilasVisibles() const {
    int rowH = RenglonHeightGS + gapGS;
    int tituloH = titulo.empty() ? 0 : rowH;
    int v = (height - bordersGS - tituloH + gapGS) / rowH;
    return v < 1 ? 1 : v;
}
int PopupMenu::MaxScroll() const {
    int m = (int)items.size() - FilasVisibles();
    return m < 0 ? 0 : m;
}
void PopupMenu::Wheel(int dir){
    if (submenuAbierto && submenuAbierto->abierto){ submenuAbierto->Wheel(dir); return; } // el submenu es el foco
    int ms = MaxScroll();
    scroll -= dir; // rueda arriba (dir +1) -> ver items ANTERIORES (scroll baja)
    if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;
}
void PopupMenu::AsegurarVisible(){
    if (selectIndex < 0) return;
    if (selectIndex < scroll) scroll = selectIndex;
    else { int v = FilasVisibles(); if (selectIndex >= scroll + v) scroll = selectIndex - v + 1; }
    int ms = MaxScroll(); if (scroll < 0) scroll = 0; if (scroll > ms) scroll = ms;
}

void PopupMenu::Abrir(int sx, int sy, int pantallaW, int pantallaH){
    scroll = 0; // arranca desde arriba
    Resize();
    x = sx;
    y = sy;
    if (x + width > pantallaW) x = pantallaW - width;
    if (y + height > pantallaH) y = pantallaH - height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    selectIndex = -1;
    abierto = true;
}

void PopupMenu::Cerrar(){
    abierto = false;
    selectIndex = -1;
    if (submenuAbierto){
        submenuAbierto->Cerrar();
        submenuAbierto = NULL;
    }
    if (MenuAbierto == this) MenuAbierto = NULL;
}

bool PopupMenu::Contains(int mx, int my) const {
    return mx >= x && mx < x + width && my >= y && my < y + height;
}

void PopupMenu::Render(){
    bool hayIconos = false;
    bool haySub = false;
    bool hayCheck = false;
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->icon >= 0) hayIconos = true;
        if (items[i]->submenu) haySub = true;
        if (items[i]->checkbox) hayCheck = true;
    }

    const float* gris = ListaColores[static_cast<int>(ColorID::gris)];
    const float* fondo = ListaColores[static_cast<int>(ColorID::background)];

    w3dEngine::PushMatrix();
    w3dEngine::Translatef((GLfloat)x, (GLfloat)y, 0);

    // fondo: tarjeta con BORDE VERDE (el menu activo es el foco)
    w3dEngine::Color4f(gris[0], gris[1], gris[2], 1.0f);
    card->RenderObject(false);
    SetColorID(ColorID::accent, 1.0f);
    card->RenderBorder(false);

    w3dEngine::Translatef((GLfloat)borderGS, (GLfloat)borderGS, 0);
    if (!titulo.empty()){
        // cabecera
        SetColorID(ColorID::blanco, 1.0f);
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
        RenderBitmapText(titulo, textAlign::left, width - bordersGS);
        w3dEngine::PopMatrix();
        // LINEA separadora bajo el titulo (rect fino: triangulos por la abstraccion, sin textura).
        // El MAS oscuro (negro): el fondo del menu ya es 'gris' (0.12), asi la linea se ve mas oscura.
        {
            const float* ac = ListaColores[static_cast<int>(ColorID::negro)];
            float y0 = (float)RenglonHeightGS, y1 = y0 + 2.0f;
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
        w3dEngine::Translatef(0, (GLfloat)(RenglonHeightGS + gapGS), 0);
    }
    // solo las filas VISIBLES [scroll .. scroll+FilasVisibles): asi el menu no se sale de la pantalla
    int _first = scroll, _last = scroll + FilasVisibles();
    if (_first < 0) _first = 0; if (_last > (int)items.size()) _last = (int)items.size();
    for (int i = _first; i < _last; i++){
        bool resaltada = (i == selectIndex);
        if (resaltada){
            SetColorID(ColorID::headerColor, 1.0f);
            highlight->RenderObject(false);
        }
        bool deshabilitada = (items[i]->gris && !*items[i]->gris);
        if (deshabilitada){
            // depende de un master apagado (ej: overlays off): NO desaparece,
            // se ve al 50% transparente (mismo color de texto, media opacidad)
            SetColorID(ColorID::grisUI, 0.5f);
        } else if (items[i]->verde){
            // opcion ACTIVA (ej: el modo de pivot actual): icono + texto en VERDE
            SetColorID(ColorID::accent, 1.0f);
        } else if (resaltada){
            SetColorID(ColorID::blanco, 1.0f);
        } else {
            SetColorID(ColorID::grisUI, 1.0f);
        }
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
        if (items[i]->icon >= 0){
            W3dDrawStrip4(IconMesh, IconsUV[items[i]->icon]->uvs);
        }
        int textStartX = gapGS + (hayIconos ? IconSizeGS + gapGS : 0);
        if (hayIconos) w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
        // el texto se corta ANTES de la columna derecha de ESTA fila (tildes/slider/atajo) ->
        // recorte del lado DERECHO. Reserva por item (no una columna unica compartida).
        int derItem = MenuItemDerecha(items[i]);
        int labelMax = width - textStartX - (derItem > 0 ? derItem + gapGS * 2 : 0) - gapGS;
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
            int lado = CheckboxLado();
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - bordersGS - lado), 0, 0);
            DibujarCheckbox(lado, *items[i]->checkbox);
            w3dEngine::PopMatrix();
        }
        if (items[i]->valorFloat){
            // SLIDER a la derecha: track + relleno (fraccion) + valor centrado
            float rango = items[i]->vmax - items[i]->vmin;
            float frac = rango > 0.0f ? (*items[i]->valorFloat - items[i]->vmin) / rango : 0.0f;
            if (frac < 0.0f) frac = 0.0f; if (frac > 1.0f) frac = 1.0f;
            int sliderW = 80 * GlobalScale;
            int barX = width - bordersGS - sliderW;
            int barH = RenglonHeightGS - 2 * GlobalScale;
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
            SetColorID(ColorID::blanco, 1.0f);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)barX, 0, 0);
            RenderBitmapFloat(*items[i]->valorFloat, textAlign::center, sliderW);
            w3dEngine::PopMatrix();
        }
        if (!items[i]->atajo.empty()){
            // hint del atajo: gris tenue (un poco mas oscuro que el texto),
            // alineado al borde derecho del menu (igual en todos los menus)
            SetColorID(ColorID::grisUI, 0.5f);
            int atajoDX = width - bordersGS - gapGS;
            if (items[i]->submenu) atajoDX -= IconSizeGS + gapGS; // dejar la flecha del submenu a la derecha del atajo
            if (items[i]->checkbox) atajoDX -= CheckboxLado() + gapGS; // dejar el checkbox a la derecha del atajo (no superponer)
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)atajoDX, 0, 0);
            RenderBitmapText(items[i]->atajo, textAlign::right, width);
            w3dEngine::PopMatrix();
        }
        w3dEngine::Translatef(0, (GLfloat)(RenglonHeightGS + gapGS), 0);
    }
    w3dEngine::PopMatrix();

    // SCROLLBAR (si hay mas items que los visibles): thumb proporcional pegado al borde derecho
    if (MaxScroll() > 0){
        int rowH = RenglonHeightGS + gapGS;
        int tituloH = titulo.empty() ? 0 : rowH;
        int trackTop = borderGS + tituloH, trackH = height - bordersGS - tituloH;
        int nItems = items.empty() ? 1 : (int)items.size();
        int thumbH = trackH * FilasVisibles() / nItems; if (thumbH < GlobalScale * 6) thumbH = GlobalScale * 6;
        int ms = MaxScroll();
        int thumbY = trackTop + (ms > 0 ? scroll * (trackH - thumbH) / ms : 0);
        int sbW = GlobalScale * 2, sbX = width - borderGS - sbW;
        const float* ac = ListaColores[static_cast<int>(ColorID::grisUI)];
        float x0 = (float)sbX, x1 = (float)(sbX + sbW), y0 = (float)thumbY, y1 = (float)(thumbY + thumbH);
        GLfloat sb[18] = { x0,y0,0, x1,y0,0, x1,y1,0,  x0,y0,0, x1,y1,0, x0,y1,0 };
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)x, (GLfloat)y, 0);
        w3dEngine::Color4f(ac[0], ac[1], ac[2], 0.85f);
        w3dEngine::Disable(w3dEngine::Texture2D);
        w3dEngine::DisableArray(w3dEngine::TexCoordArray);
        w3dEngine::VertexPointer3f(0, sb);
        w3dEngine::DrawTrianglesArray(6);
        w3dEngine::EnableArray(w3dEngine::TexCoordArray);
        w3dEngine::Enable(w3dEngine::Texture2D);
        w3dEngine::PopMatrix();
    }

    // el submenu abierto se dibuja encima
    if (submenuAbierto && submenuAbierto->abierto) submenuAbierto->Render();
}

// abre/cierra el submenu segun la fila resaltada: la opcion con submenu lo
// abre con solo resaltarla (mouse o flechas); al pasar a otra fila se cierra
// (o se abre el submenu de la nueva). Sin necesidad de click.
// TACTIL: un submenu se abre por "hover" (el dedo al apoyarse). En pantallas angostas el submenu se clampea
// y SOLAPA al padre, justo bajo el dedo -> el click del MISMO toque caia en un item del submenu (se apretaba
// solo). Este flag marca "recien abierto": el click inmediato NO selecciona, solo deja el submenu abierto.
// Se limpia al MOVER el puntero DENTRO del submenu (asi con mouse, mover+click sigue seleccionando normal).
static bool g_submenuAcabaDeAbrir = false;
// un TAP DELIBERADO sobre un item (ej: soltar sobre un clip de un submenu SCROLLABLE, ruteado por el drag-soltar) NO
// debe bloquearse por este guard: el guard es solo para el MISMO toque que ABRE el submenu solapado bajo el dedo. Sin
// esto, en el desplegable de las PROPIEDADES (panel derecho) el submenu se abre a la IZQUIERDA sobre el cursor y el
// tap caia en el guard -> "no se selecciona, queda en Escena". El timeline (submenu hacia arriba) no lo sufria.
void MenuLimpiarGuardAbrir(){ g_submenuAcabaDeAbrir = false; }

void PopupMenu::SincronizarSubmenu(){
    MenuItem* it = (selectIndex >= 0 && selectIndex < (int)items.size())
                   ? items[selectIndex] : NULL;
    PopupMenu* sub = it ? it->submenu : NULL;
    if (submenuAbierto == sub) return; // ya esta como corresponde
    if (submenuAbierto){ submenuAbierto->Cerrar(); submenuAbierto = NULL; }
    if (sub){
        int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
        sub->Abrir(x + width - gapGS,
                   y + borderGS + oy + (selectIndex - scroll) * (RenglonHeightGS + gapGS), // fila VISIBLE
                   MenuPantallaW, MenuPantallaH);
        submenuAbierto = sub;
        g_submenuAcabaDeAbrir = true; // el proximo click sobre este submenu solo lo deja abierto (no selecciona)
    }
}

bool PopupMenu::MouseMove(int mx, int my){
    if (!abierto) return false;
    // PRIORIDAD al submenu abierto: si el mouse cae sobre el, lo maneja el submenu y NO se recalcula la fila de
    // ESTE menu. Los submenus anchos se SOLAPAN sobre el padre al clampearse contra el borde de pantalla; sin esta
    // prioridad, moverse por el submenu caia dentro del bounding box del padre -> le cambiaba la opcion y cerraba
    // el submenu (no se podia usar "Set Parent To"). El submenu se dibuja ENCIMA, asi que el hover es suyo.
    if (submenuAbierto && submenuAbierto->abierto && submenuAbierto->Contains(mx, my)){
        g_submenuAcabaDeAbrir = false; // el puntero se movio DENTRO del submenu: ya es un target deliberado
        submenuAbierto->MouseMove(mx, my);
        return true;
    }
    // si el mouse esta sobre ESTE menu (y no sobre el submenu), manda este menu: resalta la fila y
    // abre/cierra el submenu segun corresponda (sin click)
    if (Contains(mx, my)){
        int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
        int fila = scroll + (my - y - borderGS - oy) / (RenglonHeightGS + gapGS); // + scroll: fila absoluta
        if (fila < 0) fila = 0;
        if (fila >= (int)items.size()) fila = (int)items.size() - 1;
        selectIndex = fila;
        SincronizarSubmenu();
        return true;
    }
    // fuera de este menu: el mouse puede estar sobre un sub-submenu mas profundo (que se extiende mas alla del submenu directo)
    if (submenuAbierto && submenuAbierto->abierto){
        if (submenuAbierto->MouseMove(mx, my)) return true;
    }
    // lejos de todo: el menu se cierra solo
    int margen = 60 * GlobalScale;
    if (mx < x - margen || mx > x + width + margen ||
        my < y - margen || my > y + height + margen){
        Cerrar();
    }
    return abierto;
}

int PopupMenu::Click(int mx, int my){
    if (submenuAbierto && submenuAbierto->abierto &&
        submenuAbierto->Contains(mx, my)){
        // el submenu se ACABA de abrir por este mismo toque (y quedo solapado bajo el dedo): NO seleccionar,
        // solo dejarlo abierto. El proximo toque ya si elige un item (tactil = 2 toques para submenus).
        if (g_submenuAcabaDeAbrir){ g_submenuAcabaDeAbrir = false; return -1; }
        int id = submenuAbierto->Click(mx, my);
        // si el submenu selecciono algo terminal (se cerro solo), lo ejecuto EN el submenu (dueno del id) y
        // cierro tambien ESTE menu. NO subo el id: colisiona por posicion con un item del padre (era el bug
        // de "importar GLB -> Circle"). Ver la nota en Enter().
        if (id >= 0 && !submenuAbierto->abierto){
            PopupMenu* sub = submenuAbierto;
            submenuAbierto = NULL;
            Cerrar();
            sub->Ejecutar(id);
            return -1; // ya ejecutado
        }
        return id;
    }
    if (!Contains(mx, my)){
        Cerrar(); // click afuera: cierra (y el click no llega atras)
        return -1;
    }
    int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
    int fila = scroll + (my - y - borderGS - oy) / (RenglonHeightGS + gapGS); // + scroll: fila absoluta
    if (fila < 0 || fila >= (int)items.size()) return -1;
    selectIndex = fila;
    // SLIDER: click en la barra setea el valor segun la X (no cierra el menu)
    MenuItem* itF = items[fila];
    if (itF->valorFloat && !(itF->gris && !*itF->gris)){
        int sliderW = 80 * GlobalScale;
        int barXscreen = x + borderGS + (width - bordersGS - sliderW);
        float frac = (float)(mx - barXscreen) / (float)sliderW;
        if (frac < 0.0f) frac = 0.0f; if (frac > 1.0f) frac = 1.0f;
        *itF->valorFloat = itF->vmin + frac * (itF->vmax - itF->vmin);
        return itF->id;
    }
    return Enter();
}

int PopupMenu::Enter(){
    if (submenuAbierto && submenuAbierto->abierto){
        int id = submenuAbierto->Enter();
        if (id >= 0 && !submenuAbierto->abierto){
            // el id pertenece al SUBMENU (es el dueno del item elegido). Hay que ejecutarlo EN el submenu,
            // NO devolverlo al padre: los ids son por-posicion y colisionan (GLB=3 del submenu Imports caia
            // en Circle=3 del menu Add -> "importar GLB" creaba un circulo). Cierro primero (como el camino
            // de un item de nivel tope) y despues ejecuto, por si la accion abre otro popup (file browser).
            PopupMenu* sub = submenuAbierto;
            submenuAbierto = NULL;
            Cerrar();
            sub->Ejecutar(id);
            return -1; // ya ejecutado: el llamador (if id>=0 Ejecutar) no re-despacha
        }
        return id;
    }
    if (selectIndex < 0 || selectIndex >= (int)items.size()) return -1;
    MenuItem* it = items[selectIndex];
    // deshabilitada (depende de un master apagado): no responde al click/enter
    if (it->gris && !*it->gris) return -1;
    if (it->submenu){
        // abre el submenu pegado a la derecha de la fila (+ pre-selecciona su 1ra)
        AbrirSubmenuActual();
        return -2;
    }
    if (it->checkbox){
        // togglea el bool y DEJA el menu abierto (para prender/apagar varios)
        *it->checkbox = !*it->checkbox;
        return it->id;
    }
    int id = it->id;
    Cerrar();
    return id;
}

void PopupMenu::button_down(){
    // las flechas SOLO mueven el resaltado; el submenu NO se auto-abre (se abre
    // con derecha/enter/ok via AbrirSubmenuActual). Si habia uno abierto, se cierra.
    if (submenuAbierto){ submenuAbierto->Cerrar(); submenuAbierto = NULL; }
    selectIndex++;
    if (selectIndex >= (int)items.size()) selectIndex = 0;
    AsegurarVisible(); // scroll para que el resaltado quede a la vista (menus largos)
}

void PopupMenu::button_up(){
    if (submenuAbierto){ submenuAbierto->Cerrar(); submenuAbierto = NULL; }
    selectIndex--;
    if (selectIndex < 0) selectIndex = (int)items.size() - 1;
    AsegurarVisible();
}

MenuItem* PopupMenu::ItemActual(){
    if (selectIndex < 0 || selectIndex >= (int)items.size()) return NULL;
    return items[selectIndex];
}

// +1 (derecha) / -1 (izquierda): mueve el valor del slider resaltado un paso
void PopupMenu::AjustarSlider(int dir){
    MenuItem* it = ItemActual();
    if (!it || !it->valorFloat) return;
    if (it->gris && !*it->gris) return; // deshabilitado
    float paso = (it->vmax - it->vmin) / 20.0f;
    if (paso <= 0.0f) paso = 0.01f;
    float v = *it->valorFloat + (float)dir * paso;
    if (v < it->vmin) v = it->vmin;
    if (v > it->vmax) v = it->vmax;
    *it->valorFloat = v;
}

// abre el submenu de la opcion resaltada (pegado a la derecha) y deja su 1ra fila
// pre-seleccionada para poder moverte enseguida con las flechas. false si no hay.
bool PopupMenu::AbrirSubmenuActual(){
    MenuItem* it = ItemActual();
    if (!it || !it->submenu) return false;
    if (it->gris && !*it->gris) return false; // deshabilitado
    int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
    it->submenu->Abrir(x + width - gapGS,
                       y + borderGS + oy + (selectIndex - scroll) * (RenglonHeightGS + gapGS), // fila VISIBLE
                       MenuPantallaW, MenuPantallaH);
    it->submenu->selectIndex = 0; // pre-seleccion (Abrir lo deja en -1)
    submenuAbierto = it->submenu;
    return true;
}
