#include "PropLabel.h"
#include "w3dGraphics.h"

PropLabel::PropLabel(const std::string& Name, bool Wrap) : PropertieBase(Name) {
    oculto = false;
    wrap = Wrap;
}

// parte 'txt' en lineas que entran en 'maxPix' pixeles, cortando SOLO en ESPACIOS (word-wrap). Si una palabra
// sola no entra, la deja igual (no se corta una palabra a la mitad). Al menos 1 linea.
static void CalcularLineasWrap(const std::string& txt, int maxPix, std::vector<std::string>& out) {
    out.clear();
    int maxChars = (CharacterWidthGS > 0) ? (maxPix / CharacterWidthGS) : 0;
    if (maxChars < 1) maxChars = 1;
    std::string linea;
    size_t i = 0;
    while (i < txt.size()) {
        while (i < txt.size() && txt[i] == ' ') i++;   // saltar espacios previos
        if (i >= txt.size()) break;
        size_t j = i;
        while (j < txt.size() && txt[j] != ' ') j++;    // palabra = [i, j)
        std::string palabra = txt.substr(i, j - i);
        if (linea.empty())
            linea = palabra;
        else if ((int)(linea.size() + 1 + palabra.size()) <= maxChars)
            linea += " " + palabra;                     // entra en la linea actual
        else { out.push_back(linea); linea = palabra; } // salto de linea
        i = j;
    }
    if (!linea.empty()) out.push_back(linea);
    if (out.empty()) out.push_back("");
}

int PropLabel::Resize(int w) {
    if (oculto) return 0;
    width = w;
    if (!wrap) return RenglonHeightGS + gapGS;
    // ancho util del texto (mismos margenes que el render de abajo): recalcula las lineas al ancho actual
    int maxPix = w - gapGS * 4; if (maxPix < CharacterWidthGS) maxPix = CharacterWidthGS;
    CalcularLineasWrap(name, maxPix, lineas);
    return (int)lineas.size() * (RenglonHeightGS + gapGS);
}

void PropLabel::RenderPropertiBox(Card* propertiBox) {
    if (oculto) return;
    int filas = (wrap && !lineas.empty()) ? (int)lineas.size() : 1;
    w3dEngine::Translatef(0, (GLfloat)(filas * (RenglonHeightGS + gapGS)), 0);
}

void PropLabel::RenderPropertiValue(Card* propertiBox) {
    if (oculto) return;
    int filas = (wrap && !lineas.empty()) ? (int)lineas.size() : 1;
    w3dEngine::Translatef(0, (GLfloat)(filas * (RenglonHeightGS + gapGS)), 0);
}

void PropLabel::RenderPropertiLabel(Card* propertiBox) {
    if (oculto) return;
    // texto a lo ancho de toda la fila (el pase de labels esta parado en el borde derecho de la caja:
    // volver al margen izquierdo). Gris tenue, alineado a la izquierda.
    SetColorID(ColorID::grisUI, 1.0f);
    if (wrap && !lineas.empty()) {
        for (size_t k = 0; k < lineas.size(); k++) {
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(-PropColEtiqueta + gapGS * 2), 0, 0);
            RenderBitmapText(lineas[k], textAlign::left, width - gapGS * 2);
            w3dEngine::PopMatrix();
            w3dEngine::Translatef(0, (GLfloat)(RenglonHeightGS + gapGS), 0); // baja UNA fila por linea
        }
    } else {
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)(-PropColEtiqueta + gapGS * 2), 0, 0);
        RenderBitmapText(name, textAlign::left, width - gapGS * 2);
        w3dEngine::PopMatrix();
        w3dEngine::Translatef(0, (GLfloat)(RenglonHeightGS + gapGS), 0);
    }
}
