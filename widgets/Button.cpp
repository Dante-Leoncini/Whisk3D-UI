#include "WhiskUI/widgets/Button.h"
#include "w3dGraphics.h"
#include "WhiskUI/draw/glesdraw.h"

Button::Button(const std::string& Text, int Icon, bool Adaptar){
    // (eran inicializadores de clase: C++03)
    text = Text;
    icon = Icon;
    adaptar = Adaptar;
    desplegable = false;
    caretMenu = false;
    centrado = false;
    cuadrado = false;
    visible = true;
    hover = false;
    focoMenu = false;
    tinte = NULL;
    colorTexto = NULL;
    colorBorde = NULL;
    iconFlip = 0;
    editField = NULL;
    rol = -1;
    width = 10;
    height = RenglonHeightGS;
    sx = -10000;
    sy = -10000;
    card = new Card(NULL, 10, RenglonHeightGS);
}

Button::~Button(){
    delete card;
}

void Button::Resize(int maxW){
    // contenido: padding + icono opcional + texto (fuente monoespaciada)
    int contenido = gapGS * 2; // padding L+R (debe matchear el render: gapGS por lado)
    if (icon >= 0) contenido += IconSizeGS;
    if (icon >= 0 && !text.empty()) contenido += gapGS;
    contenido += (int)text.size() * CharacterWidthGS;
    if (caretMenu) contenido += IconSizeGS + gapGS; // la flechita opcional suma ancho (sino trunca el texto)

    width = adaptar ? contenido : maxW;
    if (width > maxW) width = maxW; // tope: el tamano del contenedor
    if (width < RenglonHeightGS) width = RenglonHeightGS;
    // mas alto que un renglon: el icono queda centrado con aire arriba y
    // abajo. La altura depende del modo (tactil: mas alto; compacto: 2px menos)
    height = UIBotonAltura();
    if (cuadrado) width = height; // botones de icono cuadrados (transporte del timeline)
    card->Resize(width, height);
}

void Button::Render(){
    const float* gris = ListaColores[static_cast<int>(ColorID::gris)];
    const float* blanco = ListaColores[static_cast<int>(ColorID::blanco)];
    const float* fondo = ListaColores[static_cast<int>(ColorID::background)];

    w3dEngine::PushMatrix();
    // interior: el mismo solido que las tarjetas (hover: mas claro). 'tinte'
    // permite un boton de color (p.ej. la accion verde) con el mismo estilo.
    const float* base = tinte ? tinte : gris;
    if (hover) {
        w3dEngine::Color4f(base[0] + (blanco[0] - base[0]) * 0.15f,
                  base[1] + (blanco[1] - base[1]) * 0.15f,
                  base[2] + (blanco[2] - base[2]) * 0.15f, 1.0f);
    } else {
        w3dEngine::Color4f(base[0], base[1], base[2], 1.0f);
    }
    card->RenderObject(false);
    // borde oscuro (hover: blanco): es lo que lo hace notar como boton.
    // colorBorde fuerza el color (p.ej. = fondo para un boton "desactivado").
    const float* accent = ListaColores[static_cast<int>(ColorID::accent)];
    if (colorBorde)      w3dEngine::Color4f(colorBorde[0], colorBorde[1], colorBorde[2], 1.0f);
    else if (focoMenu)   w3dEngine::Color4f(accent[0], accent[1], accent[2], 1.0f); // menu enfocado
    else if (hover)      w3dEngine::Color4f(blanco[0], blanco[1], blanco[2], 1.0f);
    else                 w3dEngine::Color4f(fondo[0], fondo[1], fondo[2], 1.0f);
    card->RenderBorder(false);
    w3dEngine::PopMatrix();

    // contenido: icono opcional a la izquierda + texto (se recorta si
    // el texto es mas grande que el boton)
    w3dEngine::PushMatrix();
    int x0 = gapGS;
    if (centrado && !editField) { // como INPUT (editField) se alinea a la IZQUIERDA, no centrado
        // icono+texto centrados en el boton (OK / Cancel / switch)
        int contenido = (int)text.size() * CharacterWidthGS;
        if (icon >= 0) contenido += IconSizeGS + (text.empty() ? 0 : gapGS);
        x0 = (width - contenido) / 2;
        if (x0 < gapGS) x0 = gapGS;
    }
    w3dEngine::Translatef((GLfloat)x0, 0, 0);
    int disponible = width - x0 - gapGS;
    // caret opcional: reservar lugar a la derecha para la flecha abajo (afordancia de menu)
    if (caretMenu && !editField) disponible -= IconSizeGS + gapGS;
    const float* tc = colorTexto ? colorTexto
        : (focoMenu ? ListaColores[static_cast<int>(ColorID::accent)]
        : (hover ? blanco : ListaColores[static_cast<int>(ColorID::grisUI)]));
    if (editField && editField->selectAll) tc = ListaColores[static_cast<int>(ColorID::accent)]; // todo sel
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
        disponible -= IconSizeGS + gapGS;
    }
    // texto: si es un INPUT (editField), su texto + caret "|" (sin caret si esta TODO seleccionado)
    std::string shown = text;
    if (editField) {
        if (editField->selectAll) shown = editField->text;
        else shown = editField->text.substr(0, editField->caret) + "|" + editField->text.substr(editField->caret);
    }
    if (!shown.empty() && disponible > 0) {
        w3dEngine::Translatef(0, (GLfloat)((height - LetterHeightGS) / 2), 0);
        RenderBitmapText(shown, textAlign::left, disponible);
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
    w3dEngine::Color4f(fondo[0], fondo[1], fondo[2], 1.0f);
}

bool Button::Contains(int mx, int my) const {
    return mx >= sx && mx < sx + width && my >= sy && my < sy + height;
}
