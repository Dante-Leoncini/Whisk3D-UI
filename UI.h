#ifndef UI_H
#define UI_H

extern int GlobalScale;
extern int margin;
extern int padding;
extern int border;
extern int gap;
extern int RenglonHeight;
extern int LetterWidth;
extern int LetterHeight;
extern int paddingViewport;
extern int MinViewportHeight;
extern int MinViewportWidth;
extern int CharacterWidth;

// precalculos con el global scale
extern int marginGS;
extern int paddingGS;
extern int gapGS;
extern int borderGS;
extern int bordersGS;
extern int RenglonHeightGS;
extern int LetterWidthGS;
extern int LetterHeightGS;
extern int paddingViewportGS;
extern int MinViewportHeightGS;
extern int MinViewportWidthGS;
extern int CharacterWidthGS;

// mouse
extern int dx;
extern int dy;

// teclas
extern bool LAltPressed;
extern bool LShiftPressed;
extern bool LCtrlPressed;

// modo "pantalla tactil": botones/barras mas grandes (area de click y drag mas
// grande para el dedo) sin engrosar los bordes. Apagado (default) = modo
// compacto, pensado para el N95 (botones y barra mas bajos).
extern bool UITactil;

// alto de un boton segun el modo (tactil: mas alto; compacto: 2px menos).
// Lo usan Button::Resize y la barra de herramientas.
int UIBotonAltura();

// padding (arriba y abajo) del boton dentro de la barra de herramientas: 1px
// en compacto, mas en tactil. Simetrico.
int UIBarPadding();

#endif