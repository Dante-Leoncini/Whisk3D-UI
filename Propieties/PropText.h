#ifndef PROPTEXT_H
#define PROPTEXT_H

#include "PropertieBase.h"
#include "WhiskUI/TextField.h"   // TextField + g_textFieldActivo (caja editable compartida)
#include "WhiskUI/bitmapText.h"
#include "w3dGraphics.h"         // w3dEngine (Color4fv) + ListaColores/ColorID (resaltado select-all)

// fila de TEXTO EDITABLE (nombre del export, output del render, paths...). Al hacer
// click se ENFOCA (g_textFieldActivo = &field) y el texto llega por SDL_TEXTINPUT
// (ruteado en controles.cpp). Header-only: no agrega .cpp al MMP de Symbian.
class PropText : public PropertieBase {
    public:
        TextField field;
        bool oculto;        // true = no ocupa fila (lo usa el campo de rename: visible solo al renombrar)
        void (*onClick)();  // NULL = campo editable normal; si no, al clickear se llama esto (ej: Path -> Browse) y NO se edita

        PropText(const std::string& Name, const std::string& valor = "")
            : PropertieBase(Name) { field.SetText(valor); oculto = false; onClick = 0; }

        PropertyType GetType() { return PropertyType::Text; }
        int Resize(int w) { if (oculto) return 0; width = w; return RenglonHeightGS + gapGS; }

        // click: enfoca el campo (lo des-enfoca quien corresponda)
        bool EditPropertie() { g_textFieldActivo = &field; editando = false; return false; }

        void RenderPropertiBox(Card* propertiBox) {
            if (oculto) return;
            propertiBox->Render(false);
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
        void RenderPropertiBoxBorder(Card* propertiBox) {
            if (oculto) return;
            w3dEngine::Translatef(0, -RenglonHeightGS - gapGS, 0);
            propertiBox->RenderBorder(false);
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
        void RenderPropertiValue(Card* propertiBox) {
            if (oculto) return;
            int boxW = propertiBox->width - bordersGS;
            bool foco = (g_textFieldActivo == &field);
            if (foco && field.selectAll) {
                // TODO seleccionado (al renombrar): texto en color accent, SIN caret. Tipear lo reemplaza.
                w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::accent)]);
                RenderBitmapText(field.text, textAlign::left, boxW);
                w3dEngine::Color4fv(ListaColores[static_cast<int>(ColorID::blanco)]); // restaurar
            } else {
                std::string shown = field.text;
                if (foco) // cursor "|" en la posicion del caret
                    shown = field.text.substr(0, field.caret) + "|" + field.text.substr(field.caret);
                RenderBitmapText(shown, textAlign::left, boxW);
            }
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
        void RenderPropertiLabel(Card* propertiBox) {
            if (oculto) return;
            RenderBitmapText(name, textAlign::right, PropColEtiqueta - bordersGS);
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
};

#endif // PROPTEXT_H
