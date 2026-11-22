#ifndef PROPBUTTONROW_H
#define PROPBUTTONROW_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/Button.h"
#include "w3dGraphics.h"
#include <vector>

// VARIOS botones en UNA fila: se reparten el ancho en partes iguales (2 -> 50/50, 3 -> 33/33/33...)
// descontando el gap entre ellos. Botones con visible=false se saltean (se redistribuye el ancho).
// Click: hit-test por coords absolutas (Button::Contains). Teclado: flecha izq/der mueve el ACTIVO y
// Enter/OK dispara su accion (sin colapsar la tarjeta). Header-only (no toca el MMP de Symbian).
class PropButtonRow : public PropertieBase {
    public:
        std::vector<Button*> buttons;
        std::vector<void(*)()> actions;
        int active;   // boton activo (nav por teclado), indice en 'botones'
        int rowH;     // alto de la fila (calculado en Resize)

        PropButtonRow() : PropertieBase(""), active(0), rowH(0) {}
        ~PropButtonRow() { for (size_t i = 0; i < buttons.size(); i++) delete buttons[i]; }

        // agrega un boton (sin icono por defecto). Devuelve el Button* (p/ setear visible, texto, etc.)
        Button* Add(const std::string& text, void (*action)(), int icon = -1) {
            Button* b = new Button(text, icon, false);
            b->centered = true;
            buttons.push_back(b);
            actions.push_back(action);
            return b;
        }

        PropertyType GetType() { return PropertyType::ButtonRow; }
        bool Selectable() { return VisibleCount() > 0; }

        int VisibleCount() const {
            int n = 0; for (size_t i = 0; i < buttons.size(); i++) if (buttons[i]->visible) n++; return n;
        }
        int CellWidth(int w) const {
            int n = VisibleCount(); if (n < 1) n = 1;
            int total = w - bordersGS - (n - 1) * gapGS;
            int cw = total / n; if (cw < 1) cw = 1; return cw;
        }
        // primer boton VISIBLE (para que 'activo' nunca apunte a uno oculto)
        void ActiveClamp() {
            if (active < 0) active = 0;
            if (active >= (int)buttons.size()) active = (int)buttons.size() - 1;
            if (active >= 0 && !buttons[active]->visible) { // buscar el primero visible
                for (size_t i = 0; i < buttons.size(); i++) if (buttons[i]->visible) { active = (int)i; return; }
            }
        }

        int Resize(int w) {
            width = w;
            int n = VisibleCount(); if (n == 0) { rowH = 0; return 0; }
            int cw = CellWidth(w);
            int h = 0;
            for (size_t i = 0; i < buttons.size(); i++) if (buttons[i]->visible) {
                buttons[i]->Resize(cw); if (buttons[i]->height > h) h = buttons[i]->height;
            }
            rowH = h + gapGS;
            return rowH;
        }

        void RenderPropertiBox(Card* propertiBox) {
            int n = VisibleCount(); if (n == 0) return;
            int cw = CellWidth(width);
            int h = 0;
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)-PropColLabel, 0, 0); // al borde izquierdo del cuerpo
            for (size_t i = 0; i < buttons.size(); i++) {
                if (!buttons[i]->visible) continue;
                buttons[i]->Resize(cw);
                buttons[i]->Render();              // dibuja + setea sx/sy absolutos (hit-test del click)
                if (buttons[i]->height > h) h = buttons[i]->height;
                w3dEngine::Translatef((GLfloat)(cw + gapGS), 0, 0);
            }
            w3dEngine::PopMatrix();
            rowH = h + gapGS;
            w3dEngine::Translatef(0, (GLfloat)rowH, 0); // baja una fila (x queda igual)
        }

        // highlight del teclado: borde del boton ACTIVO (lo llama el grupo solo si la fila esta seleccionada)
        void RenderPropertiBoxBorder(Card* propertiBox) {
            if (VisibleCount() == 0) return;
            ActiveClamp();
            int cw = CellWidth(width);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)-PropColLabel, (GLfloat)-rowH, 0); // vuelve a la fila
            for (size_t i = 0; i < buttons.size(); i++) {
                if (!buttons[i]->visible) continue;
                if ((int)i == active) { buttons[i]->card->RenderBorder(false); break; }
                w3dEngine::Translatef((GLfloat)(cw + gapGS), 0, 0);
            }
            w3dEngine::PopMatrix();
        }

        void RenderPropertiValue(Card* propertiBox) { w3dEngine::Translatef(0, (GLfloat)rowH, 0); }
        void RenderPropertiLabel(Card* propertiBox) { w3dEngine::Translatef(0, (GLfloat)rowH, 0); }

        // nav por teclado: mueve 'activo' al anterior/siguiente VISIBLE
        void button_left() {
            for (int i = active - 1; i >= 0; i--) if (buttons[i]->visible) { active = i; return; }
        }
        void button_right() {
            for (int i = active + 1; i < (int)buttons.size(); i++) if (buttons[i]->visible) { active = i; return; }
        }
        // Enter/OK: dispara la accion del boton activo
        bool EditPropertie() {
            ActiveClamp();
            if (active >= 0 && active < (int)actions.size() && buttons[active]->visible && actions[active])
                actions[active]();
            return false;
        }
        // click: dispara la accion del boton 'i' (lo llama ClickIn tras hit-testear la celda por X)
        void Fire(int i) {
            if (i >= 0 && i < (int)actions.size()) { active = i; if (actions[i]) actions[i](); }
        }
};

#endif // PROPBUTTONROW_H
