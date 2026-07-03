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
    void Backspace()  { if (selectAll) { text.clear(); caret = 0; selectAll = false; return; }
                        if (caret > 0 && caret <= (int)text.size()) { text.erase(text.begin() + caret - 1); caret--; } }
    void DelForward() { if (selectAll) { text.clear(); caret = 0; selectAll = false; return; }
                        if (caret >= 0 && caret < (int)text.size()) text.erase(text.begin() + caret); }
    void CaretIzq()   { if (selectAll) { selectAll = false; caret = 0; return; } if (caret > 0) caret--; }            // deselecciona al inicio
    void CaretDer()   { if (selectAll) { selectAll = false; caret = (int)text.size(); return; } if (caret < (int)text.size()) caret++; } // al final
};

extern TextField* g_textFieldActivo;  // el campo enfocado (NULL = ninguno)
bool TextFieldInputChar(int c);       // alimenta el campo activo: 8=backspace, 127=supr, imprimibles

#endif // TEXTFIELD_H
