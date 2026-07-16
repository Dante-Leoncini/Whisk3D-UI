#ifndef CARD_H
#define CARD_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#if defined(__ANDROID__) || defined(W3D_SYMBIAN)
    #include <GLES/gl.h>
#else
    #ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif
#endif

#include "WhiskUI/object2D.h"
#include "WhiskUI/UI.h"
#include "font.h"

// Variables UV/indices
extern GLubyte CardIndices[];
extern GLfloat CardUV[32];
extern GLfloat CardBordeUV[32];
// version "pestania": el fondo y el borde quedan RECTOS abajo (sin esquina
// redondeada ni borde inferior) para fundirse con el contenido de abajo
extern GLfloat CardTabUV[32];
extern GLfloat CardTabBordeUV[32];

void CalcCardUV(int texW, int texH);

// ============================================================================
//  EL CHECKBOX del editor. UN solo lugar decide tamaño y colores, asi el del panel de propiedades y el de los
//  menus no pueden divergir. Un tilde suelto (lo que hacian los menus) no se lee como checkbox: sin la tilde no
//  se distingue de un boton comun.
//  Es una Card (bordes redondeados) rellena: ON = grisUI con el tilde en color de fondo; OFF = fondo, sin tilde.
// ============================================================================
int CheckboxLado();                     // lado del cuadrado (alto de renglon)
const float* CheckboxColorCaja(bool on);// relleno de la caja
const float* CheckboxColorTilde();      // color del tilde (va sobre la caja encendida)
const char*  CheckboxTilde();           // el glyph del tilde
// Dibuja el checkbox COMPLETO (caja + tilde) en el origen actual de la matriz. Necesita la fase CON textura
// (la Card es un 9-patch del atlas y el tilde es texto). Lo usan los menus; el panel de propiedades dibuja la
// caja y el tilde en pasadas separadas (su render es por fases) pero saca de aca tamaño y colores.
void DibujarCheckbox(int lado, bool on);

class Card : public Object2D {
	public:
		GLshort mesh[32]; // (default en el constructor: C++03)

		// Constructor
		Card(Object2D* parent = NULL, int w = 10, int h = 10);

		// Métodos
		void Resize(int w, int h);
		void SetColor(GLubyte red, GLubyte green, GLubyte blue);
		void RenderObject(bool usarColorPropio = true) override;
        void RenderBorder(bool usarColorPropio);
        // como los de arriba pero con el BORDE INFERIOR recto/cuadrado (pestanias)
        void RenderObjectTab();
        void RenderBorderTab();
};

#endif
