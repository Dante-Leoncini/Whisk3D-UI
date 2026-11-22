#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include <string>

// ============================================================================
//  CAJA DE TEXTO EDITABLE (reutilizable, 4 OS): nombre de .obj, output de render
//  (paths como "../render.png"), formulas en Properties, renombrar, etc.
//  Estado PURO (sin UI): el render/click lo hace quien la usa; la ENTRADA llega
//  por TextFieldInputChar (PC: SDL_TEXTINPUT; Symbian: su teclado), ruteada al
//  campo enfocado (g_activeTextField).
// ============================================================================
struct TextField {
    std::string text;
    int caret;               // posicion del cursor (0..len)
    bool selectAll;          // true = TODO seleccionado: tipear/borrar reemplaza todo; la flecha deselecciona
    int anchor;              // la otra punta de una seleccion PARCIAL (Shift + flechas, arrastrar el mouse); -1 = ninguna
    bool readOnly;           // se puede seleccionar y copiar, no cambiar (los datos de un recurso)
    // un NUMERO (un PropFloat, el valor de un mapa del material): solo entran digitos, el punto y la matematica
    // (+ - * / y parentesis). El keypad del telefono escribe los digitos directo (sin el multi-tap de las letras)
    bool numeric;
    TextField() : caret(0), selectAll(false), anchor(-1), readOnly(false), numeric(false) {}
    static bool CharNumeric(int c) { return (c >= '0' && c <= '9') || c == '.' || c == ',' || c == '+' || c == '-' ||
                                             c == '*' || c == '/' || c == '(' || c == ')' || c == ' '; }
    void SetText(const std::string& t) { text = t; caret = (int)t.size(); selectAll = false; anchor = -1; }
    void SelectAll() { selectAll = true; caret = (int)text.size(); anchor = -1; } // al renombrar: todo seleccionado
    // ---- LA SELECCION: todo (selectAll) o un tramo (de ancla al cursor) ----
    bool HasSelection() const { return (selectAll && !text.empty()) || (!selectAll && anchor >= 0 && anchor != caret); }
    void Selection(int& a, int& b) const {
        const int n = (int)text.size();
        if (selectAll) { a = 0; b = n; return; }
        a = anchor < caret ? anchor : caret; b = anchor < caret ? caret : anchor;
        if (a < 0) a = 0; if (b > n) b = n; if (a > b) a = b;
    }
    std::string Selected() const { if (!HasSelection()) return std::string(); int a, b; Selection(a, b); return text.substr((size_t)a, (size_t)(b - a)); }
    void Deselect() { selectAll = false; anchor = -1; }
    // borra lo seleccionado (el cursor queda donde empezaba). false = no habia seleccion
    bool DeleteSelection() {
        if (readOnly) return false;
        if (!HasSelection()) { Deselect(); return false; }
        int a, b; Selection(a, b);
        text.erase((size_t)a, (size_t)(b - a)); caret = a; Deselect();
        return true;
    }
    void InsertChar(int c) {
        if (readOnly) return;
        DeleteSelection();   // (tipear con algo seleccionado lo reemplaza)
        if (caret < 0) caret = 0; if (caret > (int)text.size()) caret = (int)text.size();
        text.insert(text.begin() + caret, (char)c); caret++;
    }
    // PEGAR: un texto entero en el cursor (reemplaza lo seleccionado). Una sola linea: los saltos van como espacios
    void InsertText(const std::string& t) {
        if (readOnly) return;
        DeleteSelection();
        if (caret < 0) caret = 0; if (caret > (int)text.size()) caret = (int)text.size();
        std::string s; s.reserve(t.size());
        for (size_t i = 0; i < t.size(); i++) { const char c = t[i]; if (c == '\r') continue; s += (c == '\n' || c == '\t') ? ' ' : c; }
        text.insert((size_t)caret, s); caret += (int)s.size();
    }
    // borrar/mover son UTF-8-aware: una "ñ" (2 bytes) o cualquier char multibyte se trata como UNA unidad
    // (los bytes de continuacion son 10xxxxxx = (b & 0xC0)==0x80). Asi el teclado en español no deja medio glifo.
    int Prev(int p) const { if (p <= 0) return 0; p--; while (p > 0 && ((unsigned char)text[p] & 0xC0) == 0x80) p--; return p; }
    int Next(int p) const { const int n = (int)text.size(); if (p >= n) return n; p++; while (p < n && ((unsigned char)text[p] & 0xC0) == 0x80) p++; return p; }
    void Backspace()  { if (readOnly || DeleteSelection()) return;
                        if (caret <= 0 || caret > (int)text.size()) return;
                        const int a = Prev(caret); text.erase(text.begin() + a, text.begin() + caret); caret = a; }
    void DelForward() { if (readOnly || DeleteSelection()) return;
                        if (caret < 0 || caret >= (int)text.size()) return;
                        text.erase(text.begin() + caret, text.begin() + Next(caret)); }
    // EL CURSOR a 'p'. Con Shift extiende la seleccion (el ancla queda donde estaba el cursor); sin Shift la saca
    void MoveTo(int p, bool shift) {
        if (p < 0) p = 0; if (p > (int)text.size()) p = (int)text.size();
        if (shift) { if (selectAll) { anchor = 0; caret = (int)text.size(); selectAll = false; } else if (anchor < 0) anchor = caret; }
        else Deselect();
        caret = p;
    }
    // las flechas: sin Shift y con algo seleccionado, el cursor va a esa punta de la seleccion (y la saca)
    void CaretLeft(bool shift = false) { if (!shift && HasSelection()) { int a, b; Selection(a, b); caret = a; Deselect(); return; }
                                        MoveTo(Prev(caret), shift); }
    void CaretRight(bool shift = false) { if (!shift && HasSelection()) { int a, b; Selection(a, b); caret = b; Deselect(); return; }
                                        MoveTo(Next(caret), shift); }
    void Start(bool shift = false) { MoveTo(0, shift); }
    void End(bool shift = false)    { MoveTo((int)text.size(), shift); }
    // Ctrl + flechas: de a palabras (lo que no es espacio ni puntuacion)
    static bool WordLetter(char c) { return (unsigned char)c >= 0x80 || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    void WordLeft(bool shift = false) { int p = selectAll && !shift ? 0 : caret; while (p > 0 && !WordLetter(text[p - 1])) p--; while (p > 0 && WordLetter(text[p - 1])) p--; MoveTo(p, shift); }
    void WordRight(bool shift = false) { const int n = (int)text.size(); int p = selectAll && !shift ? n : caret; while (p < n && !WordLetter(text[p])) p++; while (p < n && WordLetter(text[p])) p++; MoveTo(p, shift); }
};

extern TextField* g_activeTextField;  // el campo enfocado (NULL = ninguno)
// EL DIBUJO de un campo (bitmapText.cpp), en el origen actual y con el color actual. Sin foco: el texto. Con foco y
// TODO seleccionado: un fondo atras del texto (tipear reemplaza). Si no: el CURSOR, un palito que parpadea.
// align = textAlign (0 izquierda, 1 derecha, 2 centro). Igual en las 5 plataformas (PC, Symbian, Android, web, Mac).
void RenderTextField(const TextField& f, bool focus, int align, int maxPixels);
bool TextFieldBlinkTick();            // true = el palito cambio (prendido/apagado): redibujar
bool TextFieldInputChar(int c);       // alimenta el campo activo: 8=backspace, 127=supr, imprimibles

// ---- EL MOUSE Y EL PORTAPAPELES sobre el campo enfocado (los 4 OS) ----
// RenderTextField anota DONDE se dibujo el campo enfocado (en la pantalla). Con eso el mouse pone el cursor (y con
// Shift extiende), arrastrar selecciona, doble click selecciona todo, y el click derecho / mantener apretado abre el
// menu del texto (Cortar, Copiar, Pegar, Seleccionar todo: main/ui/MenuTexto)
bool TextFieldPointInActive(int mx, int my);      // el punto cae sobre el campo enfocado (lo ultimo que se dibujo)
bool TextFieldMouseDown(int mx, int my, bool shift, bool isDouble);   // false = el punto no es del campo enfocado
bool TextFieldMouseDrag(int mx);             // con el boton apretado: extiende la seleccion (false = no arrastra)
void TextFieldMouseUp();
// Ctrl+C / Ctrl+X / Ctrl+V / Ctrl+A del campo enfocado (con el portapapeles del sistema). false = no hay campo
bool TextFieldCopy();
bool TextFieldCut();
bool TextFieldPaste();
bool TextFieldSelectAll();

// ---- CANCELAR (Esc en PC, C/Cancel en el keypad) ----
// Los duenos de un campo CONFIRMAN al perder el foco comparando el texto con su valor (el Name de
// Properties, el rename en linea del outliner...). Para que Esc DESCARTE sin que cada dueno lo sepa,
// el campo vuelve al texto que tenia AL ENFOCARSE y recien ahi pierde el foco: el dueno no ve cambios.
// El texto "de antes" se anota al enfocar (TextFieldFocus) y, para los que asignan g_activeTextField
// a mano, antes de la primera tecla y en el seguimiento por cuadro (TextFieldContinueFocus).
void TextFieldFocus(TextField* f);  // g_activeTextField = f + anota su texto actual
void TextFieldContinueFocus();      // si el foco cambio por afuera, anota el texto del nuevo campo
bool TextFieldCancel();             // restaura el texto de antes y suelta el foco (false = no habia campo)
void TextFieldFrameEnd();           // (el ultimo paso de cada cuadro de la UI) seguimiento + fin del restaurado
// Los duenos que escriben EN VIVO (cada tecla ya cambio su dato: el texto de un boton 2D, el nombre de
// una paleta, las distancias de un LOD...) no confirman al perder el foco: para ellos, el campo que un
// Esc acaba de DESCARTAR sigue "enfocado" hasta el fin de ese cuadro, asi escriben el texto restaurado.
// Usan TextFieldLive en lugar de comparar g_activeTextField.
extern TextField* g_restoredTextField;
inline bool TextFieldLive(const TextField* f) { return f && (g_activeTextField == f || g_restoredTextField == f); }

#endif // TEXTFIELD_H
