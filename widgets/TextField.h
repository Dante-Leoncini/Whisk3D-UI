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
    TextField() : caret(0), selectAll(false) {}
    void SetText(const std::string& t) { text = t; caret = (int)t.size(); selectAll = false; }
    void SelectAll() { selectAll = true; caret = (int)text.size(); } // al renombrar: todo seleccionado
    void InsertChar(int c) {
        if (selectAll) { text.clear(); caret = 0; selectAll = false; } // tipear con todo sel. -> reemplaza
        if (caret < 0) caret = 0; if (caret > (int)text.size()) caret = (int)text.size();
        text.insert(text.begin() + caret, (char)c); caret++;
    }
    // borrar/mover son UTF-8-aware: una "ñ" (2 bytes) o cualquier char multibyte se trata como UNA unidad
    // (los bytes de continuacion son 10xxxxxx = (b & 0xC0)==0x80). Asi el teclado en español no deja medio glifo.
    void Backspace()  { if (selectAll) { text.clear(); caret = 0; selectAll = false; return; }
                        if (caret <= 0 || caret > (int)text.size()) return;
                        int n = 1; while (caret - n > 0 && ((unsigned char)text[caret-n] & 0xC0) == 0x80) n++;
                        text.erase(text.begin() + caret - n, text.begin() + caret); caret -= n; }
    void DelForward() { if (selectAll) { text.clear(); caret = 0; selectAll = false; return; }
                        if (caret < 0 || caret >= (int)text.size()) return;
                        int n = 1; while (caret + n < (int)text.size() && ((unsigned char)text[caret+n] & 0xC0) == 0x80) n++;
                        text.erase(text.begin() + caret, text.begin() + caret + n); }
    void CaretIzq()   { if (selectAll) { selectAll = false; caret = 0; return; }
                        if (caret > 0) { caret--; while (caret > 0 && ((unsigned char)text[caret] & 0xC0) == 0x80) caret--; } } // deselecciona al inicio
    void CaretDer()   { if (selectAll) { selectAll = false; caret = (int)text.size(); return; }
                        if (caret < (int)text.size()) { caret++; while (caret < (int)text.size() && ((unsigned char)text[caret] & 0xC0) == 0x80) caret++; } } // al final
};

extern TextField* g_textFieldActivo;  // el campo enfocado (NULL = ninguno)
bool TextFieldInputChar(int c);       // alimenta el campo activo: 8=backspace, 127=supr, imprimibles

#endif // TEXTFIELD_H
