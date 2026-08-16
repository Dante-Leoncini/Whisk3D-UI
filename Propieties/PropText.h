#ifndef PROPTEXT_H
#define PROPTEXT_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/TextField.h"   // TextField + g_textFieldActivo (caja editable compartida)
#include "WhiskUI/text/bitmapText.h"
#include "w3dGraphics.h"         // w3dEngine (Color4fv) + ListaColores/ColorID (resaltado select-all)

// fila de TEXTO EDITABLE (nombre del export, output del render, paths...). Al hacer
// click se ENFOCA (g_textFieldActivo = &field) y el texto llega por SDL_TEXTINPUT
// (ruteado en controles.cpp). Header-only: no agrega .cpp al MMP de Symbian.
class PropText : public PropertieBase {
    public:
        TextField field;
        bool oculto;        // true = no ocupa fila (lo usa el campo de rename: visible solo al renombrar)
        bool error;         // validacion: true = marcar el campo en ROJO (falta completarlo). Se autolimpia al escribir.
        void (*onClick)();  // NULL = campo editable normal; si no, al clickear se llama esto (ej: Path -> Browse) y NO se edita

        PropText(const std::string& Name, const std::string& valor = "")
            : PropertieBase(Name) { field.SetText(valor); oculto = false; error = false; onClick = 0; }

        PropertyType GetType() { return PropertyType::Text; }
        int Resize(int w) { if (oculto) return 0; width = w; return RenglonHeightGS + gapGS; }

        // OK/click sobre la fila: si tiene onClick (ej: campo Carpeta -> Browse) lo dispara y NO edita; sino
        // enfoca el campo para tipear (lo des-enfoca quien corresponda). Asi en Symbian OK abre el explorador de
        // archivos en vez de forzarte a escribir el path a mano.
        bool EditPropertie() { if (onClick) { onClick(); return false; } g_textFieldActivo = &field; editando = false; return false; }

        void RenderPropertiBox(Card* propertiBox) {
            if (oculto) return;
            propertiBox->Render(false);
            w3dEngine::Translatef(0, RenglonHeightGS + gapGS, 0);
        }
        void RenderPropertiBoxBorder(Card* propertiBox) {
            if (oculto) return;
            if (error && field.text.find_first_not_of(" \t") != std::string::npos) error = false; // se completo -> limpiar el rojo
            w3dEngine::Translatef(0, -RenglonHeightGS - gapGS, 0);
            if (error) {
                // VALIDACION: campo por completar -> BORDE en ROJO. Se pinta con el color PROPIO de la
                // card (RenderBorder(true)), seteado a rojo temporalmente y restaurado (sin tocar el relleno).
                GLubyte r = propertiBox->color[0], g = propertiBox->color[1], b = propertiBox->color[2];
                propertiBox->color[0] = 240; propertiBox->color[1] = 90; propertiBox->color[2] = 90;
                propertiBox->RenderBorder(true);
                propertiBox->color[0] = r; propertiBox->color[1] = g; propertiBox->color[2] = b;
            } else {
                propertiBox->RenderBorder(false);
            }
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
