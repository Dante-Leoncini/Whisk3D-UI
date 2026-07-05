#include "colores.h"

#include <fstream>
#include <sstream>
#include <string>

float ListaColores[ColorID::Count][4] = {
    { 0.22f,  0.28f,  0.25f,  1.0f  },  // background
    { 1.0f,   1.0f,   1.0f,   1.0f  },  // blanco
    { 0.91f,  0.50f,  0.98f,  1.0f  },  // accent
    { 0.48f,  0.30f,  1.0f,   1.0f  },  // accentDark
    { 0.0f,   0.0f,   0.0f,   1.0f  },  // negro
    { 0.12f,  0.12f,  0.12f,  1.0f  },  // gris
    { 0.22f,  0.22f,  0.22f,  1.0f  },  // headerColor
    { 0.0f,   0.0f,   0.0f,   0.25f },  // negroTransparente
    { 0.757f, 0.757f, 0.757f, 1.0f  }   // grisUI
};

GLubyte ListaColoresUbyte[ColorID::Count][4] = {
    { (GLubyte)(0.22*255),  (GLubyte)(0.28*255),  (GLubyte)(0.25*255),  255 },
    { 255, 255, 255, 255 },
    { (GLubyte)(0.91*255),  (GLubyte)(0.50*255),  (GLubyte)(0.98*255),  255 },
    { (GLubyte)(0.48*255),  (GLubyte)(0.30*255),  255,                  255 },
    { 0, 0, 0, 255 },
    { (GLubyte)(0.12*255),  (GLubyte)(0.12*255),  (GLubyte)(0.12*255),  255 },
    { (GLubyte)(0.22*255),  (GLubyte)(0.22*255),  (GLubyte)(0.22*255),  255 },
    { 0, 0, 0, (GLubyte)(0.25*255) },
    { (GLubyte)(0.757*255), (GLubyte)(0.757*255), (GLubyte)(0.757*255), 255 }
};

static int W3dColorIdPorNombre(const std::string& n) {
    if (n == "background") return ColorID::background;
    if (n == "blanco") return ColorID::blanco;
    if (n == "accent") return ColorID::accent;
    if (n == "accentDark") return ColorID::accentDark;
    if (n == "negro") return ColorID::negro;
    if (n == "gris") return ColorID::gris;
    if (n == "headerColor") return ColorID::headerColor;
    if (n == "negroTransparente") return ColorID::negroTransparente;
    if (n == "grisUI") return ColorID::grisUI;
    return -1;
}

bool loadColorsW3d(const char* aPath) {
    std::ifstream file(aPath);
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ls(line);
        std::string nombre;
        float r, g, b, a;
        if (!(ls >> nombre >> r >> g >> b >> a)) continue;
        int id = W3dColorIdPorNombre(nombre);
        if (id < 0) continue;
#ifndef __ANDROID__
        ListaColores[id][0] = r;
        ListaColores[id][1] = g;
        ListaColores[id][2] = b;
        ListaColores[id][3] = a;
#endif
        ListaColoresUbyte[id][0] = (GLubyte)(r * 255.0f);
        ListaColoresUbyte[id][1] = (GLubyte)(g * 255.0f);
        ListaColoresUbyte[id][2] = (GLubyte)(b * 255.0f);
        ListaColoresUbyte[id][3] = (GLubyte)(a * 255.0f);
    }
    return true;
}
