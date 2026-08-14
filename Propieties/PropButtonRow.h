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
        std::vector<Button*> botones;
        std::vector<void(*)()> acciones;
        int activo;   // boton activo (nav por teclado), indice en 'botones'
        int filaH;    // alto de la fila (calculado en Resize)

        PropButtonRow() : PropertieBase(""), activo(0), filaH(0) {}
        ~PropButtonRow() { for (size_t i = 0; i < botones.size(); i++) delete botones[i]; }

        // agrega un boton (sin icono por defecto). Devuelve el Button* (p/ setear visible, texto, etc.)
        Button* Agregar(const std::string& texto, void (*accion)(), int icono = -1) {
            Button* b = new Button(texto, icono, false);
            b->centrado = true;
            botones.push_back(b);
            acciones.push_back(accion);
            return b;
        }

        PropertyType GetType() { return PropertyType::ButtonRow; }
        bool Seleccionable() { return VisibleCount() > 0; }

        int VisibleCount() const {
            int n = 0; for (size_t i = 0; i < botones.size(); i++) if (botones[i]->visible) n++; return n;
        }
        int AnchoCelda(int w) const {
            int n = VisibleCount(); if (n < 1) n = 1;
            int total = w - bordersGS - (n - 1) * gapGS;
            int cw = total / n; if (cw < 1) cw = 1; return cw;
        }
        // primer boton VISIBLE (para que 'activo' nunca apunte a uno oculto)
        void ClampActivo() {
            if (activo < 0) activo = 0;
            if (activo >= (int)botones.size()) activo = (int)botones.size() - 1;
            if (activo >= 0 && !botones[activo]->visible) { // buscar el primero visible
                for (size_t i = 0; i < botones.size(); i++) if (botones[i]->visible) { activo = (int)i; return; }
            }
        }

        int Resize(int w) {
            width = w;
            int n = VisibleCount(); if (n == 0) { filaH = 0; return 0; }
            int cw = AnchoCelda(w);
            int h = 0;
            for (size_t i = 0; i < botones.size(); i++) if (botones[i]->visible) {
                botones[i]->Resize(cw); if (botones[i]->height > h) h = botones[i]->height;
            }
            filaH = h + gapGS;
            return filaH;
        }

        void RenderPropertiBox(Card* propertiBox) {
            int n = VisibleCount(); if (n == 0) return;
            int cw = AnchoCelda(width);
            int h = 0;
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)-PropColEtiqueta, 0, 0); // al borde izquierdo del cuerpo
            for (size_t i = 0; i < botones.size(); i++) {
                if (!botones[i]->visible) continue;
                botones[i]->Resize(cw);
                botones[i]->Render();              // dibuja + setea sx/sy absolutos (hit-test del click)
                if (botones[i]->height > h) h = botones[i]->height;
                w3dEngine::Translatef((GLfloat)(cw + gapGS), 0, 0);
            }
            w3dEngine::PopMatrix();
            filaH = h + gapGS;
            w3dEngine::Translatef(0, (GLfloat)filaH, 0); // baja una fila (x queda igual)
        }

        // highlight del teclado: borde del boton ACTIVO (lo llama el grupo solo si la fila esta seleccionada)
        void RenderPropertiBoxBorder(Card* propertiBox) {
            if (VisibleCount() == 0) return;
            ClampActivo();
            int cw = AnchoCelda(width);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)-PropColEtiqueta, (GLfloat)-filaH, 0); // vuelve a la fila
            for (size_t i = 0; i < botones.size(); i++) {
                if (!botones[i]->visible) continue;
                if ((int)i == activo) { botones[i]->card->RenderBorder(false); break; }
                w3dEngine::Translatef((GLfloat)(cw + gapGS), 0, 0);
            }
            w3dEngine::PopMatrix();
        }

        void RenderPropertiValue(Card* propertiBox) { w3dEngine::Translatef(0, (GLfloat)filaH, 0); }
        void RenderPropertiLabel(Card* propertiBox) { w3dEngine::Translatef(0, (GLfloat)filaH, 0); }

        // nav por teclado: mueve 'activo' al anterior/siguiente VISIBLE
        void button_left() {
            for (int i = activo - 1; i >= 0; i--) if (botones[i]->visible) { activo = i; return; }
        }
        void button_right() {
            for (int i = activo + 1; i < (int)botones.size(); i++) if (botones[i]->visible) { activo = i; return; }
        }
        // Enter/OK: dispara la accion del boton activo
        bool EditPropertie() {
            ClampActivo();
            if (activo >= 0 && activo < (int)acciones.size() && botones[activo]->visible && acciones[activo])
                acciones[activo]();
            return false;
        }
        // click: dispara la accion del boton 'i' (lo llama ClickEn tras hit-testear la celda por X)
        void Disparar(int i) {
            if (i >= 0 && i < (int)acciones.size()) { activo = i; if (acciones[i]) acciones[i](); }
        }
};

#endif // PROPBUTTONROW_H
