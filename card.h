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
