#include "WhiskUI/Tab.h"
#include "w3dGraphics.h"
#include "WhiskUI/glesdraw.h"

Tab::Tab(const std::string& Text, int Icon) {
    // (eran inicializadores de clase: C++03)
    text = Text;
    icon = Icon;
    activa = false;
    foco = false;
    visible = true;
    hover = false;
    width = 10;
    height = RenglonHeightGS;
    sx = -10000;
    sy = -10000;
    card = new Card(NULL, 10, 10);
    rect = new Rec2D();
}

Tab::~Tab() {
    delete card;
    delete rect;
}

void Tab::Resize(int maxW) {
    int contenido = gapGS * 2;
    if (icon >= 0) contenido += IconSizeGS;
    if (icon >= 0 && !text.empty()) contenido += gapGS;
    contenido += (int)text.size() * CharacterWidthGS;

    width = contenido;
    if (width > maxW) width = maxW;
    if (width < RenglonHeightGS) width = RenglonHeightGS;
    height = RenglonHeightGS + bordersGS;
    card->Resize(width, height);
}

void Tab::Render() {
    const float* gris = ListaColores[static_cast<int>(ColorID::gris)];
    const float* fondo = ListaColores[static_cast<int>(ColorID::background)];
    const float* blanco = ListaColores[static_cast<int>(ColorID::blanco)];
    const float* grisUI = ListaColores[static_cast<int>(ColorID::grisUI)];

    // cuerpo: la activa usa el color del contenido; la inactiva, mas
    // oscura (mezcla con el fondo); hover aclara un toque
    float r, g, b;
    if (activa) {
        r = gris[0]; g = gris[1]; b = gris[2];
    } else {
        r = (gris[0] + fondo[0]) * 0.5f;
        g = (gris[1] + fondo[1]) * 0.5f;
        b = (gris[2] + fondo[2]) * 0.5f;
    }
    if (hover && !activa) {
        r += (gris[0] - r) * 0.5f;
        g += (gris[1] - g) * 0.5f;
        b += (gris[2] - b) * 0.5f;
    }

    // fondo/borde con la version "tab": ARRIBA redondeado, ABAJO recto (sin
    // esquina ni borde inferior). La ACTIVA llega JUSTO al borde inferior de la barra (pisa la linea
    // del contenido y se funde); las inactivas quedan mas cortas abajo (dejan ver ese borde).
    // +GlobalScale-3: a escala 3 son +0px (coincide con el borde); antes (-2) se pasaba 1px abajo.
    int h = activa ? (height + GlobalScale - 3) : (height - GlobalScale);
    card->Resize(width, h);
    w3dEngine::Color4f(r, g, b, 1.0f);
    card->RenderObjectTab();
    w3dEngine::Color4f(fondo[0], fondo[1], fondo[2], 1.0f);
    card->RenderBorderTab();

    // contenido: icono + texto. Color: VERDE si esta enfocada con las flechas
    // (foco); BLANCA si es la activa (o hover); GRIS si es inactiva.
    const float* accent = ListaColores[static_cast<int>(ColorID::accent)];
    w3dEngine::PushMatrix();
    w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
    if (foco)               w3dEngine::Color4f(accent[0], accent[1], accent[2], 1.0f);
    else if (activa||hover) w3dEngine::Color4f(blanco[0], blanco[1], blanco[2], 1.0f);
    else                    w3dEngine::Color4f(grisUI[0], grisUI[1], grisUI[2], 1.0f);
    if (icon >= 0) {
        w3dEngine::PushMatrix();
        w3dEngine::Translatef(0, (GLfloat)((height - IconSizeGS) / 2), 0);
        W3dDrawStrip4(IconMesh, IconsUV[icon]->uvs);
        w3dEngine::PopMatrix();
        w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
    }
    if (!text.empty()) {
        w3dEngine::Translatef(0, (GLfloat)((height - LetterHeightGS) / 2), 0);
        RenderBitmapText(text, textAlign::left, width - gapGS * 2);
    }
    w3dEngine::PopMatrix();
}

bool Tab::Contains(int mx, int my) const {
    return mx >= sx && mx < sx + width && my >= sy && my < sy + height;
}
