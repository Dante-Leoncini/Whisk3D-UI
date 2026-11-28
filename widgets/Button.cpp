#include "WhiskUI/text/font.h"   // UTF8_Largo
#include "WhiskUI/widgets/Button.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"

Button::Button(const std::string& Text, int Icon, bool Adapt){
    // (eran inicializadores de clase: C++03)
    text = Text;
    icon = Icon;
    adapt = Adapt;
    dropdown = false;
    joinNext = false;
    caretMenu = false;
    rowHeight = false;
    centered = false;
    square   = false;
    visible = true;
    hover = false;
    separator = false;
    focusMenu = false;
    tint = NULL;
    textColor   = NULL;
    borderColor = NULL;
    alpha = 1.0f;
    iconFlip = 0;
    fill = NULL;
    bar = -1.0f;
    editField = NULL;
    role = -1;
    width = 10;
    height = RowHeightGS;
    sx = -10000;
    sy = -10000;
    card = new Card(NULL, 10, RowHeightGS);
}

Button::~Button(){
    delete card;
}

void Button::Resize(int maxW){
    // contenido: padding + icono opcional + texto (fuente monoespaciada)
    int content = gapGS * 2; // padding L+R (debe matchear el render: gapGS por lado)
    if (icon >= 0) content += IconSizeGS;
    if (icon >= 0 && !text.empty()) content += gapGS;
    content += UTF8_Length(text) * CharacterWidthGS;
    if (caretMenu) content += IconSizeGS + gapGS; // la flechita opcional suma ancho (sino trunca el texto)

    width = adapt ? content : maxW;
    if (width > maxW) width = maxW; // tope: el tamano del contenedor
    if (width < RowHeightGS) width = RowHeightGS;
    // mas alto que un renglon: el icono queda centrado con aire arriba y
    // abajo. La altura depende del modo (tactil: mas alto; compacto: 2px menos)
    // rowHeight: EXACTAMENTE el alto del box de los inputs de propiedades
    // (RowHeightGS + GlobalScale*2, ver GroupPropertie::Resize) -> todo parejo
    height = rowHeight ? RowHeightGS + GlobalScale * 2 : UIButtonHeight();
    if (square) width = height; // botones de icono cuadrados (transporte del timeline)
    card->Resize(width, height);
}

void Button::Render(){
    const float* gray = ColorList[static_cast<int>(ColorID::gray)];
    const float* white = ColorList[static_cast<int>(ColorID::white)];
    const float* background = ColorList[static_cast<int>(ColorID::background)];

    w3dEngine::PushMatrix();
    // interior: el mismo solido que las tarjetas (hover: mas claro). 'tinte'
    // permite un boton de color (p.ej. la accion verde) con el mismo estilo.
    const float* base = tint ? tint : gray;
    // 'alpha' viene del contenedor: 1.0 en los paneles (nada cambia) y el barAlpha de
    // la barra en el viewport 3D, donde el fondo deja ver la escena y los botones
    // quedaban opacos encima. El icono ya se dibujaba con blend habilitado: lo opaco
    // era este Card.
    const float a = (alpha < 0.0f) ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
    if (hover) {
        w3dEngine::Color4f(base[0] + (white[0] - base[0]) * 0.15f,
                  base[1] + (white[1] - base[1]) * 0.15f,
                  base[2] + (white[2] - base[2]) * 0.15f, a);
    } else {
        w3dEngine::Color4f(base[0], base[1], base[2], a);
    }
    card->RenderObject(false);
    // borde oscuro (hover: blanco): es lo que lo hace notar como boton.
    // borderColor fuerza el color (p.ej. = fondo para un boton "desactivado").
    const float* accent = ColorList[static_cast<int>(ColorID::accent)];
    if (borderColor)      w3dEngine::Color4f(borderColor[0], borderColor[1], borderColor[2], a);
    else if (focusMenu)   w3dEngine::Color4f(accent[0], accent[1], accent[2], a); // menu enfocado
    else if (hover)      w3dEngine::Color4f(white[0], white[1], white[2], a);
    else                 w3dEngine::Color4f(background[0], background[1], background[2], a);
    card->RenderBorder(false);
    // la BARRA de un numero de 0 a 1 (el relleno de su largo, en accent oscuro)
    if (bar >= 0.0f && !editField) {
        const float f = bar > 1.0f ? 1.0f : bar;
        const int wb = (int)((float)(width - 2 * GlobalScale) * f);
        if (wb > 0) {
            const float* ad = ColorList[static_cast<int>(ColorID::accentDark)];
            w3dEngine::Color4f(ad[0], ad[1], ad[2], a);
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)GlobalScale, (GLfloat)GlobalScale, 0);
            card->Resize(wb, height - 2 * GlobalScale); card->RenderObject(false);
            card->Resize(width, height);
            w3dEngine::PopMatrix();
        }
    }
    // el CUADRADO de color (el color base, la emision): ocupa el boton con un margen
    if (fill) {
        const int m = 3 * GlobalScale;
        w3dEngine::Color4f(fill[0], fill[1], fill[2], 1.0f);
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)m, (GLfloat)m, 0);
        card->Resize(width - 2 * m, height - 2 * m); card->RenderObject(false);
        card->Resize(width, height);
        w3dEngine::PopMatrix();
    }
    w3dEngine::PopMatrix();

    // contenido: icono opcional a la izquierda + texto (se recorta si
    // el texto es mas grande que el boton)
    w3dEngine::PushMatrix();
    int x0 = gapGS;
    // solo ICONO (sin texto): centrado en el boton (los cuadrados de solo icono quedaban corridos a la derecha)
    if (text.empty() && icon >= 0 && !editField) { x0 = (width - IconSizeGS) / 2; if (x0 < 0) x0 = 0; }
    else if ((centered || bar >= 0.0f) && !editField) {   // (el numero de una barra va en el medio) // como INPUT (editField) se alinea a la IZQUIERDA, no centrado
        // icono+texto centrados en el boton (OK / Cancel / switch)
        int content = UTF8_Length(text) * CharacterWidthGS;
        if (icon >= 0) content += IconSizeGS + (text.empty() ? 0 : gapGS);
        x0 = (width - content) / 2;
        if (x0 < gapGS) x0 = gapGS;
    }
    w3dEngine::Translatef((GLfloat)x0, 0, 0);
    int available = width - x0 - gapGS;
    // caret opcional: reservar el lugar de la flechita de la derecha
    if (caretMenu && !editField) available -= IconSizeGS + gapGS;
    const float* tc = textColor ? textColor
        : (focusMenu ? ColorList[static_cast<int>(ColorID::accent)]
        : (hover ? white : ColorList[static_cast<int>(ColorID::grayUI)]));
    w3dEngine::Color4f(tc[0], tc[1], tc[2], 1.0f);
    if (icon >= 0) {
        w3dEngine::PushMatrix();
        w3dEngine::Translatef(0, (GLfloat)((height - IconSizeGS) / 2), 0); // centrado
        const GLfloat* uv = IconsUV[icon]->uvs;
        GLfloat fuv[8];
        if (iconFlip == 1) { // horizontal: intercambia columnas U
            fuv[0]=uv[2]; fuv[1]=uv[1]; fuv[2]=uv[0]; fuv[3]=uv[3];
            fuv[4]=uv[6]; fuv[5]=uv[5]; fuv[6]=uv[4]; fuv[7]=uv[7]; uv = fuv;
        } else if (iconFlip == 2) { // vertical: intercambia filas V
            fuv[0]=uv[4]; fuv[1]=uv[5]; fuv[2]=uv[6]; fuv[3]=uv[7];
            fuv[4]=uv[0]; fuv[5]=uv[1]; fuv[6]=uv[2]; fuv[7]=uv[3]; uv = fuv;
        }
        W3dDrawStrip4(IconMesh, uv);
        w3dEngine::PopMatrix();
        w3dEngine::Translatef((GLfloat)(IconSizeGS + gapGS), 0, 0);
        available -= IconSizeGS + gapGS;
    }
    // texto: si es un INPUT (editField), el campo (todo seleccionado = fondo atras; si no, el cursor que parpadea)
    if (editField && available > 0) {
        w3dEngine::Translatef(0, (GLfloat)((height - LetterHeightGS) / 2), 0);
        RenderTextField(*editField, g_activeTextField == editField, textAlign::left, available);
    } else if (!text.empty() && available > 0) {
        w3dEngine::Translatef(0, (GLfloat)((height - LetterHeightGS) / 2), 0);
        RenderBitmapText(text, textAlign::left, available);
    }
    w3dEngine::PopMatrix();

    // caret opcional: triangulo hacia abajo a la derecha (deja claro que abre un menu). OPT-IN via caretMenu
    if (caretMenu && !editField) {
        w3dEngine::PushMatrix();
        w3dEngine::Color4f(tc[0], tc[1], tc[2], 1.0f);
        w3dEngine::Translatef((GLfloat)(width - gapGS - IconSizeGS), (GLfloat)((height - IconSizeGS) / 2), 0);
        W3dDrawStrip4(IconMesh, IconsUV[static_cast<int>(IconType::arrow)]->uvs); // 'arrow' apunta abajo por defecto
        w3dEngine::PopMatrix();
    }

    // dejar el color como lo esperan las filas de propiedades
    w3dEngine::Color4f(background[0], background[1], background[2], 1.0f);
}

bool Button::Contains(int mx, int my) const {
    return mx >= sx && mx < sx + width && my >= sy && my < sy + height;
}
