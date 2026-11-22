#ifndef PROPTEXT_H
#define PROPTEXT_H

#include "PropertieBase.h"
#include "WhiskUI/widgets/TextField.h"   // TextField + g_activeTextField (caja editable compartida)
#include "WhiskUI/text/bitmapText.h"
#include "w3dGraphics.h"         // w3dEngine (Color4fv) + ColorList/ColorID (resaltado select-all)

// fila de TEXTO EDITABLE (nombre del export, output del render, paths...). Al hacer
// click se ENFOCA (g_activeTextField = &field) y el texto llega por SDL_TEXTINPUT
// (ruteado en controls.cpp). Header-only: no agrega .cpp al MMP de Symbian.
class PropText : public PropertieBase {
    public:
        TextField field;
        bool hidden;        // true = no ocupa fila (lo usa el campo de rename: visible solo al renombrar)
        bool error;         // validacion: true = marcar el campo en ROJO (falta completarlo). Se autolimpia al escribir.
        bool red;           // el TEXTO del valor en rojo: un dato que avisa (un recurso que nadie usa: "0" usuarios)
        void (*onClick)();  // NULL = campo editable normal; si no, al clickear se llama esto (ej: Path -> Browse) y NO se edita

        PropText(const std::string& Name, const std::string& value = "")
            : PropertieBase(Name) { field.SetText(value); hidden = false; error = false; red = false; onClick = 0; }

        PropertyType GetType() { return PropertyType::Text; }
        int Resize(int w) { if (hidden) return 0; width = w; return RowHeightGS + gapGS; }

        // OK/click sobre la fila: si tiene onClick (ej: campo Carpeta -> Browse) lo dispara y NO edita; sino
        // enfoca el campo para tipear (lo des-enfoca quien corresponda). Asi en Symbian OK abre el explorador de
        // archivos en vez de forzarte a escribir el path a mano.
        bool EditPropertie() { if (onClick) { onClick(); return false; } TextFieldFocus(&field); editing = false; return false; }

        void RenderPropertiBox(Card* propertiBox) {
            if (hidden) return;
            propertiBox->Render(false);
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
        }
        void RenderPropertiBoxBorder(Card* propertiBox) {
            if (hidden) return;
            if (error && field.text.find_first_not_of(" \t") != std::string::npos) error = false; // se completo -> limpiar el rojo
            w3dEngine::Translatef(0, -RowHeightGS - gapGS, 0);
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
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
        }
        void RenderPropertiValue(Card* propertiBox) {
            if (hidden) return;
            int boxW = propertiBox->width - bordersGS;
            // (todo seleccionado: fondo atras del texto, tipear lo reemplaza; si no, el cursor que parpadea)
            if (red) w3dEngine::Color4f(240.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
            RenderTextField(field, g_activeTextField == &field, textAlign::left, boxW);
            if (red) w3dEngine::Color4fv(ColorList[static_cast<int>(ColorID::white)]);
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
        }
        void RenderPropertiLabel(Card* propertiBox) {
            if (hidden) return;
            RenderBitmapText(name, textAlign::right, PropColLabel - bordersGS);
            w3dEngine::Translatef(0, RowHeightGS + gapGS, 0);
        }
};

#endif // PROPTEXT_H
