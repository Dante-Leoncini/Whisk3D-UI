#ifndef OBJECT2D_H
#define OBJECT2D_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <vector>

// RVCT (C++03) no conoce override: misma guarda que objects/Objects.h
#if defined(__cplusplus) && __cplusplus < 201103L
    #ifndef override
        #define override
    #endif
#endif

#if defined(__ANDROID__) || defined(W3D_SYMBIAN)
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

// dialecto C++03 compartido
struct Object2DType {
    enum Enum {
        text,
        empty,
        Image,
        Rectangle
    };
    Enum v;
    Object2DType(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

class Object2D {
public:
    Object2D* Parent;     // (inicializados en el constructor: C++03)
    bool visible;
    int x, y;
    GLshort width;
    GLshort height;
    int scaleX;
    int scaleY;
    GLubyte opacity;

    GLubyte color[3];

    std::vector<Object2D*> Childrens;

    Object2D(Object2D* parent = NULL);

    virtual Object2DType getType();
    virtual void RenderObject(bool usarColorPropio = true);
    virtual ~Object2D();

    void Render(bool usarColorPropio = true);
};

#endif