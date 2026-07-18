#ifndef REC2D_H
#define REC2D_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#if defined(__ANDROID__) || defined(W3D_SYMBIAN)
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "WhiskUI/draw/object2D.h"

class Rec2D : public Object2D {
	public:
		GLshort vertices[8];   // (default en el constructor: C++03)
		// version FLOAT/triangulos del strip (2 triangulos, 6 verts). La UI dibuja por la
		// abstraccion, que usa ESTE camino en los 4 OS (el N95 solo hace GL_TRIANGLES float).
		GLfloat verticesTri[12];

		// Constructor
		Rec2D(Object2D* parent = NULL);

		// Métodos
		void SetSize(GLshort x, GLshort y, GLshort w, GLshort h);
		void SetColor(GLubyte red, GLubyte green, GLubyte blue);
		void RenderObject(bool usarColorPropio = true) override;
};

#endif
