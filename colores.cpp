#include "colores.h"
#include "objects/RenderColors.h" // gRenderColors (paleta de RENDER del core) para el sync

// ---- FLOAT COLORS (los 4 OS; el GLES1 de Android/Symbian los toma via w3dEngine::Material) ----
float ListaColores[21][4] = {
    { 0.22f, 0.28f, 0.25f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.91f, 0.50f, 0.98f, 1.0f },
    { 0.48f, 0.30f, 1.0f,  1.0f },
    { 0.0f,  0.0f,  0.0f,  1.0f },
    { 0.12f, 0.12f, 0.12f, 1.0f },
    { 0.94f, 0.59f, 0.17f, 0.25f },
    { 0.22f, 0.22f, 0.22f, 1.0f },
    { 0.0f,  0.0f,  0.0f, 0.25f },
    { 0.757f, 0.757f, 0.757f, 1.0f },
    { 0.757f, 0.757f, 0.757f, 1.0f },
    { 0.22f, 0.22f, 0.22f, 1.0f },
    { 0.56f, 0.23f, 0.28f, 1.0f },
    { 0.38f, 0.53f, 0.15f, 1.0f },
    { 0.88f, 0.48f, 0.54f, 1.0f },
    { 0.65f, 0.81f, 0.38f, 1.0f },
    { 0.46f, 0.67f, 0.89f, 1.0f },
    { 1.0f,  1.0f,  0.0f,  1.0f },  // normalVertex (amarillo saturado)
    { 1.0f,  0.0f,  1.0f,  1.0f },  // normalCustom (magenta)
    { 0.0f,  1.0f,  1.0f,  1.0f },  // normalFace (cian)
    { 0.55f, 0.85f, 0.0f,  1.0f }   // seleccionInactiva (verde-rojizo, apenas)
};

// ---- UBYTE COLORS ----
GLubyte ListaColoresUbyte[21][4] = {
    { (GLubyte)(0.22*255), (GLubyte)(0.28*255), (GLubyte)(0.25*255), 255 },
    { 255, 255, 255, 255 },
    { (GLubyte)(0.91*255), (GLubyte)(0.50*255), (GLubyte)(0.98*255), 255 },
    { (GLubyte)(0.48*255), (GLubyte)(0.30*255), 255, 255 },
    { 0, 0, 0, 255 },
    { (GLubyte)(0.12*255), (GLubyte)(0.12*255), (GLubyte)(0.12*255), 255 },
    { (GLubyte)(0.94*255), (GLubyte)(0.59*255), (GLubyte)(0.17*255), (GLubyte)(0.25*255) },
    { (GLubyte)(0.22*255), (GLubyte)(0.22*255), (GLubyte)(0.22*255), 255 },
    { 0, 0, 0, (GLubyte)(0.25*255) },
    { (GLubyte)(0.757*255), (GLubyte)(0.757*255), (GLubyte)(0.757*255), 255 },
    { (GLubyte)(0.757*255), (GLubyte)(0.757*255), (GLubyte)(0.757*255), 255 },
    { (GLubyte)(0.22*255), (GLubyte)(0.22*255), (GLubyte)(0.22*255), 255 },
    { (GLubyte)(0.56*255), (GLubyte)(0.23*255), (GLubyte)(0.28*255), 255 },
    { (GLubyte)(0.38*255), (GLubyte)(0.53*255), (GLubyte)(0.15*255), 255 },
    { (GLubyte)(0.88*255), (GLubyte)(0.48*255), (GLubyte)(0.54*255), 255 },
    { (GLubyte)(0.65*255), (GLubyte)(0.81*255), (GLubyte)(0.38*255), 255 },
    { (GLubyte)(0.46*255), (GLubyte)(0.67*255), (GLubyte)(0.89*255), 255 },
    { 255, 255, 0,   255 },  // normalVertex (amarillo)
    { 255, 0,   255, 255 },  // normalCustom (magenta)
    { 0,   255, 255, 255 },  // normalFace (cian)
    { (GLubyte)(0.55*255), (GLubyte)(0.85*255), 0, 255 } // seleccionInactiva
};

// ===================================================
// Cargador de skin.ini portable (C++03; sin unordered_map/filesystem).
// El loadColors viejo de main.cpp puede migrar a este.
// ===================================================
#include <fstream>
#include <sstream>
#include <string>

static int W3dColorIdPorNombre(const std::string& n) {
    if (n == "background") return ColorID::background;
    if (n == "blanco") return ColorID::blanco;
    if (n == "accent") return ColorID::accent;
    if (n == "accentDark") return ColorID::accentDark;
    if (n == "negro") return ColorID::negro;
    if (n == "gris") return ColorID::gris;
    if (n == "naranjaFace") return ColorID::naranjaFace;
    if (n == "headerColor") return ColorID::headerColor;
    if (n == "negroTransparente") return ColorID::negroTransparente;
    if (n == "grisUI") return ColorID::grisUI;
    if (n == "rojoEje") return ColorID::rojoEje;
    if (n == "LineaPiso") return ColorID::LineaPiso;
    if (n == "LineaPisoRoja") return ColorID::LineaPisoRoja;
    if (n == "LineaPisoVerde") return ColorID::LineaPisoVerde;
    if (n == "ColorTransformX") return ColorID::ColorTransformX;
    if (n == "ColorTransformY") return ColorID::ColorTransformY;
    if (n == "ColorTransformZ") return ColorID::ColorTransformZ;
    if (n == "normalVertex") return ColorID::normalVertex;
    if (n == "normalCustom") return ColorID::normalCustom;
    if (n == "normalFace") return ColorID::normalFace;
    if (n == "seleccionInactiva") return ColorID::seleccionInactiva;
    return -1;
}

// COMPARTIDO (4 OS): copia la paleta de la UI (ListaColores, cargada del skin) a la
// paleta de RENDER del core (gRenderColors). El core no incluye la paleta de la UI;
// este puente la traduce. Antes vivia inline en main.cpp -> Symbian no lo corria y el
// borde de seleccion salia naranja (el default del core) en vez del verde del skin.
void SincronizarRenderColores() {
    for (int k = 0; k < 4; k++) {
        gRenderColors[RC_wireframe][k]    = ListaColores[static_cast<int>(ColorID::grisUI)][k];
        gRenderColors[RC_selActive][k]    = ListaColores[static_cast<int>(ColorID::accent)][k];
        gRenderColors[RC_selInactive][k]  = ListaColores[static_cast<int>(ColorID::seleccionInactiva)][k];
        gRenderColors[RC_normalFace][k]   = ListaColores[static_cast<int>(ColorID::normalFace)][k];
        gRenderColors[RC_normalVert][k]   = ListaColores[static_cast<int>(ColorID::normalVertex)][k];
        gRenderColors[RC_normalCustom][k] = ListaColores[static_cast<int>(ColorID::normalCustom)][k];
        gRenderColors[RC_gizmoDark][k]    = ListaColores[static_cast<int>(ColorID::accentDark)][k];
    }
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
    SincronizarRenderColores(); // UI -> gRenderColors del core (lo necesita Symbian)
    return true;
}
