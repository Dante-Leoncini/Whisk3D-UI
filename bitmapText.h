#ifndef BITMAPTEXT_H
#define BITMAPTEXT_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <string>
#include <sstream>   // std::ostringstream
#include <iomanip>   // std::setprecision, std::fixed

#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "UI.h"
#include "WhiskUI/UI.h"
#include "WhiskUI/font.h"

// dialecto C++03 compartido
struct textAlign {
    enum Enum {
        left,
        right,
        center
    };
    Enum v;
    textAlign(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

void RenderBitmapText(const std::string& text, textAlign align = textAlign::left, int maxPixels = 1920);

void RenderBitmapFloat(float value, textAlign align = textAlign::left, int maxPixels = 1920, const std::string& unit = "");

#endif