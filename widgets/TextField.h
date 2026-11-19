#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include <string>

// ============================================================================
//  CAJA DE TEXTO EDITABLE (reutilizable, 4 OS): nombre de .obj, output de render
//  (paths como "../render.png"), formulas en Properties, renombrar, etc.
//  Estado PURO (sin UI): el render/click lo hace quien la usa; la ENTRADA llega
//  por TextFieldInputChar (PC: SDL_TEXTINPUT; Symbian: su teclado), ruteada al
//  campo enfocado (g_textFieldActivo).
// ============================================================================
struct TextField {
    std::string text;
    int caret;                 // posicion del cursor (0..len)
    bool selectAll;            // true = TODO seleccionado: tipear/borrar reemplaza todo; la flecha deselecciona
    int ancla;                 // la otra punta de una seleccion PARCIAL (Shift + flechas, arrastrar el mouse); -1 = ninguna
    bool soloLectura;          // se puede seleccionar y copiar, no cambiar (los datos de un recurso)
    TextField() : caret(0), selectAll(false), ancla(-1), soloLectura(false) {}
    void SetText(const std::string& t) { text = t; caret = (int)t.size(); selectAll = false; ancla = -1; }
    void SelectAll() { selectAll = true; caret = (int)text.size(); ancla = -1; } // al renombrar: todo seleccionado
    // ---- LA SELECCION: todo (selectAll) o un tramo (de ancla al cursor) ----
    bool HaySeleccion() const { return (selectAll && !text.empty()) || (!selectAll && ancla >= 0 && ancla != caret); }
    void Seleccion(int& a, int& b) const {
        const int n = (int)text.size();
        if (selectAll) { a = 0; b = n; return; }
        a = ancla < caret ? ancla : caret; b = ancla < caret ? caret : ancla;
        if (a < 0) a = 0; if (b > n) b = n; if (a > b) a = b;
    }
    std::string Seleccionado() const { if (!HaySeleccion()) return std::string(); int a, b; Seleccion(a, b); return text.substr((size_t)a, (size_t)(b - a)); }
    void Deseleccionar() { selectAll = false; ancla = -1; }
    // borra lo seleccionado (el cursor queda donde empezaba). false = no habia seleccion
    bool BorrarSeleccion() {
        if (soloLectura) return false;
        if (!HaySeleccion()) { Deseleccionar(); return false; }
        int a, b; Seleccion(a, b);
        text.erase((size_t)a, (size_t)(b - a)); caret = a; Deseleccionar();
        return true;
    }
    void InsertChar(int c) {
        if (soloLectura) return;
        BorrarSeleccion();   // (tipear con algo seleccionado lo reemplaza)
        if (caret < 0) caret = 0; if (caret > (int)text.size()) caret = (int)text.size();
        text.insert(text.begin() + caret, (char)c); caret++;
    }
    // PEGAR: un texto entero en el cursor (reemplaza lo seleccionado). Una sola linea: los saltos van como espacios
    void InsertarTexto(const std::string& t) {
        if (soloLectura) return;
        BorrarSeleccion();
        if (caret < 0) caret = 0; if (caret > (int)text.size()) caret = (int)text.size();
        std::string s; s.reserve(t.size());
        for (size_t i = 0; i < t.size(); i++) { const char c = t[i]; if (c == '\r') continue; s += (c == '\n' || c == '\t') ? ' ' : c; }
        text.insert((size_t)caret, s); caret += (int)s.size();
    }
    // borrar/mover son UTF-8-aware: una "ñ" (2 bytes) o cualquier char multibyte se trata como UNA unidad
    // (los bytes de continuacion son 10xxxxxx = (b & 0xC0)==0x80). Asi el teclado en español no deja medio glifo.
    int Anterior(int p) const { if (p <= 0) return 0; p--; while (p > 0 && ((unsigned char)text[p] & 0xC0) == 0x80) p--; return p; }
    int Siguiente(int p) const { const int n = (int)text.size(); if (p >= n) return n; p++; while (p < n && ((unsigned char)text[p] & 0xC0) == 0x80) p++; return p; }
    void Backspace()  { if (soloLectura || BorrarSeleccion()) return;
                        if (caret <= 0 || caret > (int)text.size()) return;
                        const int a = Anterior(caret); text.erase(text.begin() + a, text.begin() + caret); caret = a; }
    void DelForward() { if (soloLectura || BorrarSeleccion()) return;
                        if (caret < 0 || caret >= (int)text.size()) return;
                        text.erase(text.begin() + caret, text.begin() + Siguiente(caret)); }
    // EL CURSOR a 'p'. Con Shift extiende la seleccion (el ancla queda donde estaba el cursor); sin Shift la saca
    void MoverA(int p, bool shift) {
        if (p < 0) p = 0; if (p > (int)text.size()) p = (int)text.size();
        if (shift) { if (selectAll) { ancla = 0; caret = (int)text.size(); selectAll = false; } else if (ancla < 0) ancla = caret; }
        else Deseleccionar();
        caret = p;
    }
    // las flechas: sin Shift y con algo seleccionado, el cursor va a esa punta de la seleccion (y la saca)
    void CaretIzq(bool shift = false) { if (!shift && HaySeleccion()) { int a, b; Seleccion(a, b); caret = a; Deseleccionar(); return; }
                                        MoverA(Anterior(caret), shift); }
    void CaretDer(bool shift = false) { if (!shift && HaySeleccion()) { int a, b; Seleccion(a, b); caret = b; Deseleccionar(); return; }
                                        MoverA(Siguiente(caret), shift); }
    void Inicio(bool shift = false) { MoverA(0, shift); }
    void Fin(bool shift = false)    { MoverA((int)text.size(), shift); }
    // Ctrl + flechas: de a palabras (lo que no es espacio ni puntuacion)
    static bool LetraDePalabra(char c) { return (unsigned char)c >= 0x80 || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    void PalabraIzq(bool shift = false) { int p = selectAll && !shift ? 0 : caret; while (p > 0 && !LetraDePalabra(text[p - 1])) p--; while (p > 0 && LetraDePalabra(text[p - 1])) p--; MoverA(p, shift); }
    void PalabraDer(bool shift = false) { const int n = (int)text.size(); int p = selectAll && !shift ? n : caret; while (p < n && !LetraDePalabra(text[p])) p++; while (p < n && LetraDePalabra(text[p])) p++; MoverA(p, shift); }
};

extern TextField* g_textFieldActivo;  // el campo enfocado (NULL = ninguno)
// EL DIBUJO de un campo (bitmapText.cpp), en el origen actual y con el color actual. Sin foco: el texto. Con foco y
// TODO seleccionado: un fondo atras del texto (tipear reemplaza). Si no: el CURSOR, un palito que parpadea.
// align = textAlign (0 izquierda, 1 derecha, 2 centro). Igual en las 5 plataformas (PC, Symbian, Android, web, Mac).
void RenderTextField(const TextField& f, bool foco, int align, int maxPixels);
bool TextFieldParpadeoTick();         // true = el palito cambio (prendido/apagado): redibujar
bool TextFieldInputChar(int c);       // alimenta el campo activo: 8=backspace, 127=supr, imprimibles

// ---- EL MOUSE Y EL PORTAPAPELES sobre el campo enfocado (los 4 OS) ----
// RenderTextField anota DONDE se dibujo el campo enfocado (en la pantalla). Con eso el mouse pone el cursor (y con
// Shift extiende), arrastrar selecciona, doble click selecciona todo, y el click derecho / mantener apretado abre el
// menu del texto (Cortar, Copiar, Pegar, Seleccionar todo: main/ui/MenuTexto)
bool TextFieldPuntoEnActivo(int mx, int my);      // el punto cae sobre el campo enfocado (lo ultimo que se dibujo)
bool TextFieldMouseAbajo(int mx, int my, bool shift, bool doble);   // false = el punto no es del campo enfocado
bool TextFieldMouseArrastrar(int mx);             // con el boton apretado: extiende la seleccion (false = no arrastra)
void TextFieldMouseArriba();
// Ctrl+C / Ctrl+X / Ctrl+V / Ctrl+A del campo enfocado (con el portapapeles del sistema). false = no hay campo
bool TextFieldCopiar();
bool TextFieldCortar();
bool TextFieldPegar();
bool TextFieldSeleccionarTodo();

// ---- CANCELAR (Esc en PC, C/Cancel en el keypad) ----
// Los duenos de un campo CONFIRMAN al perder el foco comparando el texto con su valor (el Name de
// Properties, el rename en linea del outliner...). Para que Esc DESCARTE sin que cada dueno lo sepa,
// el campo vuelve al texto que tenia AL ENFOCARSE y recien ahi pierde el foco: el dueno no ve cambios.
// El texto "de antes" se anota al enfocar (TextFieldEnfocar) y, para los que asignan g_textFieldActivo
// a mano, antes de la primera tecla y en el seguimiento por cuadro (TextFieldSeguirFoco).
void TextFieldEnfocar(TextField* f);  // g_textFieldActivo = f + anota su texto actual
void TextFieldSeguirFoco();           // si el foco cambio por afuera, anota el texto del nuevo campo
bool TextFieldCancelar();             // restaura el texto de antes y suelta el foco (false = no habia campo)
void TextFieldFinDeCuadro();          // (el ultimo paso de cada cuadro de la UI) seguimiento + fin del restaurado
// Los duenos que escriben EN VIVO (cada tecla ya cambio su dato: el texto de un boton 2D, el nombre de
// una paleta, las distancias de un LOD...) no confirman al perder el foco: para ellos, el campo que un
// Esc acaba de DESCARTAR sigue "enfocado" hasta el fin de ese cuadro, asi escriben el texto restaurado.
// Usan TextFieldEnVivo en lugar de comparar g_textFieldActivo.
extern TextField* g_textFieldRestaurado;
inline bool TextFieldEnVivo(const TextField* f) { return f && (g_textFieldActivo == f || g_textFieldRestaurado == f); }

#endif // TEXTFIELD_H
