#include "WhiskUI/PopupMenu.h"
#include "w3dGraphics.h"
#include "WhiskUI/glesdraw.h"

PopupMenu* MenuAbierto = NULL;
int MenuPantallaW = 100000;
int MenuPantallaH = 100000;

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
}

PopupMenu::PopupMenu(){
    titulo = "";
    x = 0;
    y = 0;
    width = 10;
    height = 10;
    selectIndex = -1;
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
    bool haySub = false;
    bool hayCheck = false;
    bool hayFloat = false;
    int maxChars = 0;
    int maxAtajo = 0;
    for (size_t i = 0; i < items.size(); i++){
        if (items[i]->icon >= 0) hayIconos = true;
        if (items[i]->submenu) haySub = true;
        if (items[i]->checkbox) hayCheck = true;
        if (items[i]->valorFloat) hayFloat = true;
        if ((int)items[i]->text.size() > maxChars)
            maxChars = (int)items[i]->text.size();
        if ((int)items[i]->atajo.size() > maxAtajo)
            maxAtajo = (int)items[i]->atajo.size();
    }
    if ((int)titulo.size() > maxChars) maxChars = (int)titulo.size();
    // columna derecha UNICA, alineada al borde: flecha de submenu, check o
    // atajo (nunca dos en la misma fila) -> ancho = el mayor de ellos
    int rightCol = 0;
    if (haySub || hayCheck) rightCol = IconSizeGS;
    if (maxAtajo * CharacterWidthGS > rightCol) rightCol = maxAtajo * CharacterWidthGS;
    // si una opcion tiene submenu (flecha) Y atajo a la vez (ej. "Delete" -> X + flecha), el atajo
    // se dibuja A LA IZQUIERDA de la flecha (ver Render) -> reservar lugar para los dos juntos
    int maxAtajoSub = 0;
    for (size_t i = 0; i < items.size(); i++)
        if (items[i]->submenu && (int)items[i]->atajo.size() > maxAtajoSub)
            maxAtajoSub = (int)items[i]->atajo.size();
    if (maxAtajoSub > 0){
        int both = IconSizeGS + gapGS + maxAtajoSub * CharacterWidthGS;
        if (both > rightCol) rightCol = both;
    }
    // si ninguna opcion tiene icono, el lugar queda libre para el texto
    width = bordersGS + gapGS * 2 + maxChars * CharacterWidthGS;
    if (hayIconos) width += IconSizeGS + gapGS;
    if (rightCol > 0) width += rightCol + gapGS * 2;
    if (hayFloat) width += 80 * GlobalScale + gapGS; // barra del slider
    height = bordersGS + (int)items.size() * (RenglonHeightGS + gapGS) - gapGS;
    if (!titulo.empty()) height += RenglonHeightGS + gapGS; // la cabecera

    card->Resize(width, height);
    highlight->Resize(width - bordersGS, RenglonHeightGS + GlobalScale);
}

void PopupMenu::Abrir(int sx, int sy, int pantallaW, int pantallaH){
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
    w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::accent)][0],
              ListaColores[static_cast<int>(ColorID::accent)][1],
              ListaColores[static_cast<int>(ColorID::accent)][2], 1.0f);
    card->RenderBorder(false);

    w3dEngine::Translatef((GLfloat)borderGS, (GLfloat)borderGS, 0);
    if (!titulo.empty()){
        // cabecera
        w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::blanco)][0],
                  ListaColores[static_cast<int>(ColorID::blanco)][1],
                  ListaColores[static_cast<int>(ColorID::blanco)][2], 1.0f);
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
    for (size_t i = 0; i < items.size(); i++){
        bool resaltada = ((int)i == selectIndex);
        if (resaltada){
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::headerColor)][0],
                      ListaColores[static_cast<int>(ColorID::headerColor)][1],
                      ListaColores[static_cast<int>(ColorID::headerColor)][2], 1.0f);
            highlight->RenderObject(false);
        }
        bool deshabilitada = (items[i]->gris && !*items[i]->gris);
        if (deshabilitada){
            // depende de un master apagado (ej: overlays off): NO desaparece,
            // se ve al 50% transparente (mismo color de texto, media opacidad)
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::grisUI)][0],
                      ListaColores[static_cast<int>(ColorID::grisUI)][1],
                      ListaColores[static_cast<int>(ColorID::grisUI)][2], 0.5f);
        } else if (items[i]->verde){
            // opcion ACTIVA (ej: el modo de pivot actual): icono + texto en VERDE
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::accent)][0],
                      ListaColores[static_cast<int>(ColorID::accent)][1],
                      ListaColores[static_cast<int>(ColorID::accent)][2], 1.0f);
        } else if (resaltada){
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::blanco)][0],
                      ListaColores[static_cast<int>(ColorID::blanco)][1],
                      ListaColores[static_cast<int>(ColorID::blanco)][2], 1.0f);
        } else {
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::grisUI)][0],
                      ListaColores[static_cast<int>(ColorID::grisUI)][1],
                      ListaColores[static_cast<int>(ColorID::grisUI)][2], 1.0f);
        }
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
        if (items[i]->icon >= 0){
            W3dDrawStrip4(IconMesh, IconsUV[items[i]->icon]->uvs);
        }
        if (hayIconos) w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
        RenderBitmapText(items[i]->text, textAlign::left, width);
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
            // checkbox a la derecha: tilde si esta encendido (estilo PropBool)
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(width - bordersGS - IconSizeGS), 0, 0);
            RenderBitmapText(*items[i]->checkbox ? "\xE2\x9C\x94" : "",
                             textAlign::left, IconSizeGS + gapGS);
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
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::headerColor)][0],
                      ListaColores[static_cast<int>(ColorID::headerColor)][1],
                      ListaColores[static_cast<int>(ColorID::headerColor)][2], 1.0f);
            sliderCard->Resize(sliderW, barH); sliderCard->RenderObject(false);
            int fillW = (int)(frac * sliderW);
            if (fillW > 2){
                w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::accent)][0],
                          ListaColores[static_cast<int>(ColorID::accent)][1],
                          ListaColores[static_cast<int>(ColorID::accent)][2], 1.0f);
                sliderCard->Resize(fillW, barH); sliderCard->RenderObject(false);
            }
            w3dEngine::PopMatrix();
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::blanco)][0],
                      ListaColores[static_cast<int>(ColorID::blanco)][1],
                      ListaColores[static_cast<int>(ColorID::blanco)][2], 1.0f);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)barX, 0, 0);
            RenderBitmapFloat(*items[i]->valorFloat, textAlign::center, sliderW);
            w3dEngine::PopMatrix();
        }
        if (!items[i]->atajo.empty()){
            // hint del atajo: gris tenue (un poco mas oscuro que el texto),
            // alineado al borde derecho del menu (igual en todos los menus)
            w3dEngine::Color4f(ListaColores[static_cast<int>(ColorID::grisUI)][0],
                      ListaColores[static_cast<int>(ColorID::grisUI)][1],
                      ListaColores[static_cast<int>(ColorID::grisUI)][2], 0.5f);
            int atajoDX = width - bordersGS - gapGS;
            if (items[i]->submenu) atajoDX -= IconSizeGS + gapGS; // dejar la flecha del submenu a la derecha del atajo
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)atajoDX, 0, 0);
            RenderBitmapText(items[i]->atajo, textAlign::right, width);
            w3dEngine::PopMatrix();
        }
        w3dEngine::Translatef(0, (GLfloat)(RenglonHeightGS + gapGS), 0);
    }
    w3dEngine::PopMatrix();

    // el submenu abierto se dibuja encima
    if (submenuAbierto && submenuAbierto->abierto) submenuAbierto->Render();
}

// abre/cierra el submenu segun la fila resaltada: la opcion con submenu lo
// abre con solo resaltarla (mouse o flechas); al pasar a otra fila se cierra
// (o se abre el submenu de la nueva). Sin necesidad de click.
void PopupMenu::SincronizarSubmenu(){
    MenuItem* it = (selectIndex >= 0 && selectIndex < (int)items.size())
                   ? items[selectIndex] : NULL;
    PopupMenu* sub = it ? it->submenu : NULL;
    if (submenuAbierto == sub) return; // ya esta como corresponde
    if (submenuAbierto){ submenuAbierto->Cerrar(); submenuAbierto = NULL; }
    if (sub){
        int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
        sub->Abrir(x + width - gapGS,
                   y + borderGS + oy + selectIndex * (RenglonHeightGS + gapGS),
                   MenuPantallaW, MenuPantallaH);
        submenuAbierto = sub;
    }
}

bool PopupMenu::MouseMove(int mx, int my){
    if (!abierto) return false;
    // PRIORIDAD al submenu abierto: si el mouse cae sobre el, lo maneja el submenu y NO se recalcula la fila de
    // ESTE menu. Los submenus anchos se SOLAPAN sobre el padre al clampearse contra el borde de pantalla; sin esta
    // prioridad, moverse por el submenu caia dentro del bounding box del padre -> le cambiaba la opcion y cerraba
    // el submenu (no se podia usar "Set Parent To"). El submenu se dibuja ENCIMA, asi que el hover es suyo.
    if (submenuAbierto && submenuAbierto->abierto && submenuAbierto->Contains(mx, my)){
        submenuAbierto->MouseMove(mx, my);
        return true;
    }
    // si el mouse esta sobre ESTE menu (y no sobre el submenu), manda este menu: resalta la fila y
    // abre/cierra el submenu segun corresponda (sin click)
    if (Contains(mx, my)){
        int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
        int fila = (my - y - borderGS - oy) / (RenglonHeightGS + gapGS);
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
        int id = submenuAbierto->Click(mx, my);
        // si el submenu selecciono algo terminal (se cerro solo), cerramos
        // tambien ESTE menu para que no quede el primero abierto
        if (id >= 0 && !submenuAbierto->abierto){
            submenuAbierto = NULL;
            Cerrar();
        }
        return id;
    }
    if (!Contains(mx, my)){
        Cerrar(); // click afuera: cierra (y el click no llega atras)
        return -1;
    }
    int oy = titulo.empty() ? 0 : (RenglonHeightGS + gapGS);
    int fila = (my - y - borderGS - oy) / (RenglonHeightGS + gapGS);
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
            submenuAbierto = NULL;
            Cerrar();
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
}

void PopupMenu::button_up(){
    if (submenuAbierto){ submenuAbierto->Cerrar(); submenuAbierto = NULL; }
    selectIndex--;
    if (selectIndex < 0) selectIndex = (int)items.size() - 1;
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
                       y + borderGS + oy + selectIndex * (RenglonHeightGS + gapGS),
                       MenuPantallaW, MenuPantallaH);
    it->submenu->selectIndex = 0; // pre-seleccion (Abrir lo deja en -1)
    submenuAbierto = it->submenu;
    return true;
}
