#ifndef GLESDRAW_H
#define GLESDRAW_H

/*
 * Dibujo 2D portable. En PC son los draws de siempre (STRIP / drawElements
 * con vertices GLshort). En el N95 (W3D_SYMBIAN) el driver NO dibuja
 * TRIANGLE_STRIP, glDrawElements ni vertices SHORT en la fase 2D del frame:
 * se expande a GL_TRIANGLES con vertices FLOAT en un pool estatico rotativo
 * (nada de arrays en el stack: el driver captura los punteros tarde).
 */

#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
    // GLES 1.1 no tiene las variantes "v" ni los tokens sin sufijo OES
    #ifndef glColor4fv
        #define glColor4fv(p) glColor4f((p)[0], (p)[1], (p)[2], (p)[3])
    #endif
    #ifndef GL_POINT_SPRITE
        #define GL_POINT_SPRITE 0x8861
    #endif
    #ifndef GL_COORD_REPLACE
        #define GL_COORD_REPLACE 0x8862
    #endif
#else
    #ifdef _WIN32
#ifndef W3D_SYMBIAN
        #include <windows.h>
#endif // GL/gl.h necesita APIENTRY/WINGDIAPI
    #endif
    #include <GL/gl.h>
#endif

// alto de pantalla para el flip de Y (arbol arriba-izq -> GL abajo-izq);
// lo setea cada plataforma al hacer resize
extern int W3dPantallaAlto;

// quad en orden de strip (4 vertices GLshort xy) + UVs float (8)
void W3dDrawStrip4(const GLshort* aVerts8, const GLfloat* aUVs8);

// malla indexada chica (hasta 24 indices GLubyte), vertices GLshort xy
void W3dDrawElemsB(const GLshort* aVerts, const GLfloat* aUVs,
                   const GLubyte* aIdx, int aCount);

// lineas indexadas (GL_LINES): el drawElements del N95 tampoco es confiable
// en el pase 3D para lineas. Vertices de 3 componentes.
void W3dDrawLinesS(int aLineWidth, int aCantidad,
                   const GLshort* aVerts3, const GLushort* aIdx);
void W3dDrawLinesF(const GLfloat* aVerts3, const GLushort* aIdx, int aIdxCount);

#endif
