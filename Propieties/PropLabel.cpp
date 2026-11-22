#include "PropLabel.h"
#include "w3dGraphics.h"

PropLabel::PropLabel(const std::string& Name, bool Wrap) : PropertieBase(Name) {
    hidden = false;
    wrap = Wrap;
    red  = false;
}

// parte 'txt' en lineas que entran en 'maxPix' pixeles, cortando SOLO en ESPACIOS (word-wrap). Si una palabra
// sola no entra, la deja igual (no se corta una palabra a la mitad). Al menos 1 linea.
static void ComputeLinesWrap(const std::string& txt, int maxPix, std::vector<std::string>& out) {
    out.clear();
    int maxChars = (CharacterWidthGS > 0) ? (maxPix / CharacterWidthGS) : 0;
    if (maxChars < 1) maxChars = 1;
    std::string line;
    size_t i = 0;
    while (i < txt.size()) {
        while (i < txt.size() && txt[i] == ' ') i++;   // saltar espacios previos
        if (i >= txt.size()) break;
        size_t j = i;
        while (j < txt.size() && txt[j] != ' ') j++;    // palabra = [i, j)
        std::string word = txt.substr(i, j - i);
        if (line.empty())
            line = word;
        else if ((int)(line.size() + 1 + word.size()) <= maxChars)
            line += " " + word;                    // entra en la linea actual
        else { out.push_back(line); line = word; } // salto de linea
        i = j;
    }
    if (!line.empty()) out.push_back(line);
    if (out.empty()) out.push_back("");
}

int PropLabel::Resize(int w) {
    if (hidden) return 0;
    width = w;
    if (!wrap) return RowHeightGS + gapGS;
    // ancho util del texto (mismos margenes que el render de abajo): recalcula las lineas al ancho actual
    int maxPix = w - gapGS * 4; if (maxPix < CharacterWidthGS) maxPix = CharacterWidthGS;
    ComputeLinesWrap(name, maxPix, lines);
    return (int)lines.size() * (RowHeightGS + gapGS);
}

void PropLabel::RenderPropertiBox(Card* propertiBox) {
    if (hidden) return;
    int rows = (wrap && !lines.empty()) ? (int)lines.size() : 1;
    w3dEngine::Translatef(0, (GLfloat)(rows * (RowHeightGS + gapGS)), 0);
}

void PropLabel::RenderPropertiValue(Card* propertiBox) {
    if (hidden) return;
    int rows = (wrap && !lines.empty()) ? (int)lines.size() : 1;
    w3dEngine::Translatef(0, (GLfloat)(rows * (RowHeightGS + gapGS)), 0);
}

void PropLabel::RenderPropertiLabel(Card* propertiBox) {
    if (hidden) return;
    // texto a lo ancho de toda la fila (el pase de labels esta parado en el borde derecho de la caja:
    // volver al margen izquierdo). Gris tenue, alineado a la izquierda.
    if (red) w3dEngine::Color4f(0.92f, 0.28f, 0.24f, 1.0f);
    else SetColorID(ColorID::grayUI, 1.0f);
    if (wrap && !lines.empty()) {
        for (size_t k = 0; k < lines.size(); k++) {
            w3dEngine::PushMatrix();
            w3dEngine::Translatef((GLfloat)(-PropColLabel + gapGS * 2), 0, 0);
            RenderBitmapText(lines[k], textAlign::left, width - gapGS * 2);
            w3dEngine::PopMatrix();
            w3dEngine::Translatef(0, (GLfloat)(RowHeightGS + gapGS), 0); // baja UNA fila por linea
        }
    } else {
        w3dEngine::PushMatrix();
        w3dEngine::Translatef((GLfloat)(-PropColLabel + gapGS * 2), 0, 0);
        RenderBitmapText(name, textAlign::left, width - gapGS * 2);
        w3dEngine::PopMatrix();
        w3dEngine::Translatef(0, (GLfloat)(RowHeightGS + gapGS), 0);
    }
}
