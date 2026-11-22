#include "UI.h"

int GlobalScale = 1;
int uiMargin = 10;
int padding = 10;
int border = 4;
int gap = 5;
int RowHeight = 12;
int LetterWidth = 6;
int LetterHeight = 11;
int paddingViewport = 3;
int MinViewportHeight = 30;
int MinViewportWidth = 30;
int CharacterWidth = 6;

int marginGS = uiMargin;
int paddingGS = padding;
int gapGS = gap;
int borderGS = border;
int bordersGS = border*2;
int RowHeightGS = RowHeight;
int LetterWidthGS = LetterWidth;
int LetterHeightGS = LetterHeight;
int paddingViewportGS = paddingViewport;
int MinViewportHeightGS = MinViewportHeight;
int MinViewportWidthGS = MinViewportWidth;
int CharacterWidthGS = CharacterWidth;
int LineScaleUI() { return GlobalScale >= 4 ? GlobalScale / 2 : 1; }

int dx = 0;
int dy = 0;

bool LAltPressed = false;
bool LShiftPressed = false;
bool LCtrlPressed = false;
bool g_viewEditMode = false;

bool UITouch = false; // default: modo compacto (N95)

int UIButtonHeight(){
    int h = RowHeightGS + bordersGS;
    if (UITouch) h += gapGS;        // tactil: boton mas alto (dedo)
    else          h -= 2 * GlobalScale; // compacto: 2px menos alto
    return h;
}

int UIBarPadding(){
    // 1px (logico) arriba y abajo en compacto; mas en tactil (area del dedo)
    return UITouch ? GlobalScale + gapGS : GlobalScale;
}