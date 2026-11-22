#include "WhiskUI/text/font.h"   // UTF8_Largo
#include "WhiskUI/widgets/Tab.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"

Tab::Tab(const std::string& Text, int Icon) {
    // (eran inicializadores de clase: C++03)
    text = Text;
    icon = Icon;
    active = false;
    focus = false;
    visible = true;
    hover = false;
    width = 10;
    height = RowHeightGS;
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
    int content = gapGS * 2;
    if (icon >= 0) content += IconSizeGS;
    if (icon >= 0 && !text.empty()) content += gapGS;
    content += UTF8_Length(text) * CharacterWidthGS;

    width = content;
    if (width > maxW) width = maxW;
    if (width < RowHeightGS) width = RowHeightGS;
    height = RowHeightGS + bordersGS;
    card->Resize(width, height);
}

void Tab::Render() {
    const float* gray = ColorList[static_cast<int>(ColorID::gray)];
    const float* background = ColorList[static_cast<int>(ColorID::background)];
    const float* white  = ColorList[static_cast<int>(ColorID::white)];
    const float* grayUI = ColorList[static_cast<int>(ColorID::grayUI)];

    // cuerpo: la activa usa el color del contenido; la inactiva, mas
    // oscura (mezcla con el fondo); hover aclara un toque
    float r, g, b;
    if (active) {
        r = gray[0]; g = gray[1]; b = gray[2];
    } else {
        r = (gray[0] + background[0]) * 0.5f;
        g = (gray[1] + background[1]) * 0.5f;
        b = (gray[2] + background[2]) * 0.5f;
    }
    if (hover && !active) {
        r += (gray[0] - r) * 0.5f;
        g += (gray[1] - g) * 0.5f;
        b += (gray[2] - b) * 0.5f;
    }

    // fondo/borde con la version "tab": ARRIBA redondeado, ABAJO recto (sin
    // esquina ni borde inferior). La ACTIVA llega JUSTO al borde inferior de la barra (pisa la linea
    // del contenido y se funde); las inactivas quedan mas cortas abajo (dejan ver ese borde).
    // +GlobalScale-3: a escala 3 son +0px (coincide con el borde); antes (-2) se pasaba 1px abajo.
    int h = active ? (height + GlobalScale - 3) : (height - GlobalScale);
    card->Resize(width, h);
    w3dEngine::Color4f(r, g, b, 1.0f);
    card->RenderObjectTab();
    w3dEngine::Color4f(background[0], background[1], background[2], 1.0f);
    card->RenderBorderTab();

    // contenido: icono + texto. Color: VERDE si esta enfocada con las flechas
    // (foco); BLANCA si es la activa (o hover); GRIS si es inactiva.
    const float* accent = ColorList[static_cast<int>(ColorID::accent)];
    w3dEngine::PushMatrix();
    w3dEngine::Translatef((GLfloat)gapGS, 0, 0);
    if (focus)               w3dEngine::Color4f(accent[0], accent[1], accent[2], 1.0f);
    else if (active||hover) w3dEngine::Color4f(white[0], white[1], white[2], 1.0f);
    else                    w3dEngine::Color4f(grayUI[0], grayUI[1], grayUI[2], 1.0f);
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
