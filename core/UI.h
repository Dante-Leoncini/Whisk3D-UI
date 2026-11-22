#ifndef UI_H
#define UI_H

extern int GlobalScale;
extern int uiMargin;
extern int padding;
extern int border;
extern int gap;
extern int RowHeight;
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
extern int RowHeightGS;
extern int LetterWidthGS;
extern int LetterHeightGS;
extern int paddingViewportGS;
extern int MinViewportHeightGS;
extern int MinViewportWidthGS;
extern int CharacterWidthGS;
// el GROSOR de las lineas de la UI que se dibujan con triangulos (el borde de los viewports): 1 hasta la escala x3; en
// las pantallas de mucha densidad (x4 en adelante: los telefonos, el iPhone en Safari) el doble, sino ni se ven
int LineScaleUI();

// mouse
extern int dx;
extern int dy;

// teclas
extern bool LAltPressed;
extern bool LShiftPressed;
extern bool LCtrlPressed;

// modo VIEW (toggle tactil de Edit Mode): con 1 dedo se ORBITA/panea/zoom aunque haya una operacion de
// edicion en curso (mover/rotar/extrude/strip) SIN cancelarla. Se apaga y segui editando desde el nuevo angulo.
extern bool g_viewEditMode;

// modo "pantalla tactil": botones/barras mas grandes (area de click y drag mas
// grande para el dedo) sin engrosar los bordes. Apagado (default) = modo
// compacto, pensado para el N95 (botones y barra mas bajos).
extern bool UITouch;

// alto de un boton segun el modo (tactil: mas alto; compacto: 2px menos).
// Lo usan Button::Resize y la barra de herramientas.
int UIButtonHeight();

// padding (arriba y abajo) del boton dentro de la barra de herramientas: 1px
// en compacto, mas en tactil. Simetrico.
int UIBarPadding();

#endif