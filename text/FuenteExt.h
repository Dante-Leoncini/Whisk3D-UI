#ifndef W3D_FUENTE_EXT_H
#define W3D_FUENTE_EXT_H
// GENERADO por tools/fuente_ext.py -- no editar a mano: se cambia el script y se lo vuelve a correr.
// El atlas EXTENDIDO de la fuente (res/Skins/<skin>/font-ext.png): marcas de acentos, letras y signos que no
// entran en font.png y el cirilico. Ver font.cpp: se carga recien cuando un texto lo necesita.

#define W3D_FUENTE_EXT_ANCHO 128
#define W3D_FUENTE_EXT_ALTO 64
#define W3D_FUENTE_EXT_COLUMNAS 21   // celda i: x = 1 + (i % columnas) * 6, y = (i / columnas) * 12

// las marcas de los acentos: su celda (el glifo entero de 5x11, con la marca en su fila)
enum W3dFuenteExtMarca {
    W3dMarcaGraveMin = 0,
    W3dMarcaGraveMay = 1,
    W3dMarcaCircunflejoMin = 2,
    W3dMarcaCircunflejoMay = 3,
    W3dMarcaDieresisMin = 4,
    W3dMarcaDieresisMay = 5,
    W3dMarcaPuntoMin = 6,
    W3dMarcaPuntoMay = 7,
    W3dMarcaBreveMin = 8,
    W3dMarcaBreveMay = 9,
    W3dMarcaCedilla = 10,
    W3dMarcaOgonek = 11,
    W3dMarcaCantidad = 12
};

// los caracteres dibujados en este atlas (ordenados por codepoint): su celda
struct W3dFuenteExtGlifo { unsigned short cp; unsigned char celda; };
static const W3dFuenteExtGlifo W3dFuenteExtGlifos[] = {
    { 0x003B, 20 },   // ;
    { 0x005E, 27 },   // ^
    { 0x0060, 29 },   // `
    { 0x007E, 28 },   // ~
    { 0x00AB, 22 },   // «
    { 0x00B1, 21 },   // ±
    { 0x00BB, 23 },   // »
    { 0x00C6, 19 },   // Æ
    { 0x00DF, 12 },   // ß
    { 0x00E6, 18 },   // æ
    { 0x0131, 13 },   // ı
    { 0x0141, 15 },   // Ł
    { 0x0142, 14 },   // ł
    { 0x0152, 17 },   // Œ
    { 0x0153, 16 },   // œ
    { 0x0411, 30 },   // Б
    { 0x0413, 31 },   // Г
    { 0x0414, 32 },   // Д
    { 0x0416, 33 },   // Ж
    { 0x0417, 34 },   // З
    { 0x0418, 35 },   // И
    { 0x041B, 36 },   // Л
    { 0x041F, 37 },   // П
    { 0x0423, 38 },   // У
    { 0x0424, 39 },   // Ф
    { 0x0426, 40 },   // Ц
    { 0x0427, 41 },   // Ч
    { 0x0428, 42 },   // Ш
    { 0x0429, 43 },   // Щ
    { 0x042A, 44 },   // Ъ
    { 0x042B, 45 },   // Ы
    { 0x042C, 46 },   // Ь
    { 0x042D, 47 },   // Э
    { 0x042E, 48 },   // Ю
    { 0x042F, 49 },   // Я
    { 0x0431, 50 },   // б
    { 0x0432, 51 },   // в
    { 0x0433, 52 },   // г
    { 0x0434, 53 },   // д
    { 0x0436, 54 },   // ж
    { 0x0437, 55 },   // з
    { 0x0438, 56 },   // и
    { 0x043A, 57 },   // к
    { 0x043B, 58 },   // л
    { 0x043C, 59 },   // м
    { 0x043D, 60 },   // н
    { 0x043F, 61 },   // п
    { 0x0442, 62 },   // т
    { 0x0444, 63 },   // ф
    { 0x0446, 64 },   // ц
    { 0x0447, 65 },   // ч
    { 0x0448, 66 },   // ш
    { 0x0449, 67 },   // щ
    { 0x044A, 68 },   // ъ
    { 0x044B, 69 },   // ы
    { 0x044C, 70 },   // ь
    { 0x044D, 71 },   // э
    { 0x044E, 72 },   // ю
    { 0x044F, 73 },   // я
    { 0x2013, 25 },   // –
    { 0x2014, 24 },   // —
    { 0x2026, 26 },   // …
};
static const unsigned W3dFuenteExtGlifosN = 62;

// cirilicas que son IGUALES a una latina: se dibujan con la de font.png
struct W3dFuenteExtIgual { unsigned short cp; unsigned short latina; };
static const W3dFuenteExtIgual W3dFuenteExtIguales[] = {
    { 0x0410, 'A' },   // А
    { 0x0412, 'B' },   // В
    { 0x0415, 'E' },   // Е
    { 0x041A, 'K' },   // К
    { 0x041C, 'M' },   // М
    { 0x041D, 'H' },   // Н
    { 0x041E, 'O' },   // О
    { 0x0420, 'P' },   // Р
    { 0x0421, 'C' },   // С
    { 0x0422, 'T' },   // Т
    { 0x0425, 'X' },   // Х
    { 0x0430, 'a' },   // а
    { 0x0435, 'e' },   // е
    { 0x043E, 'o' },   // о
    { 0x0440, 'p' },   // р
    { 0x0441, 'c' },   // с
    { 0x0443, 'y' },   // у
    { 0x0445, 'x' },   // х
};
static const unsigned W3dFuenteExtIgualesN = 18;

// letras COMPUESTAS: la letra base + una marca. marca: 0 = agudo y 1 = tilde (de font.png, ver font.cpp);
// 2 + n = la celda n de este atlas (ya elegida: la version de la marca para mayuscula o minuscula)
struct W3dFuenteExtCompuesta { unsigned short cp; unsigned short base; unsigned char marca; };
static const W3dFuenteExtCompuesta W3dFuenteExtCompuestas[] = {
    { 0x00C0, 0x0041, 3 },   // À = A + Grave
    { 0x00C1, 0x0041, 0 },   // Á = A + Agudo
    { 0x00C2, 0x0041, 5 },   // Â = A + Circunflejo
    { 0x00C3, 0x0041, 1 },   // Ã = A + Tilde
    { 0x00C4, 0x0041, 7 },   // Ä = A + Dieresis
    { 0x00C7, 0x0043, 12 },   // Ç = C + Cedilla
    { 0x00C8, 0x0045, 3 },   // È = E + Grave
    { 0x00C9, 0x0045, 0 },   // É = E + Agudo
    { 0x00CA, 0x0045, 5 },   // Ê = E + Circunflejo
    { 0x00CB, 0x0045, 7 },   // Ë = E + Dieresis
    { 0x00CC, 0x0049, 3 },   // Ì = I + Grave
    { 0x00CD, 0x0049, 0 },   // Í = I + Agudo
    { 0x00CE, 0x0049, 5 },   // Î = I + Circunflejo
    { 0x00CF, 0x0049, 7 },   // Ï = I + Dieresis
    { 0x00D2, 0x004F, 3 },   // Ò = O + Grave
    { 0x00D3, 0x004F, 0 },   // Ó = O + Agudo
    { 0x00D4, 0x004F, 5 },   // Ô = O + Circunflejo
    { 0x00D5, 0x004F, 1 },   // Õ = O + Tilde
    { 0x00D6, 0x004F, 7 },   // Ö = O + Dieresis
    { 0x00D9, 0x0055, 3 },   // Ù = U + Grave
    { 0x00DA, 0x0055, 0 },   // Ú = U + Agudo
    { 0x00DB, 0x0055, 5 },   // Û = U + Circunflejo
    { 0x00DC, 0x0055, 7 },   // Ü = U + Dieresis
    { 0x00DD, 0x0059, 0 },   // Ý = Y + Agudo
    { 0x00E0, 0x0061, 2 },   // à = a + Grave
    { 0x00E1, 0x0061, 0 },   // á = a + Agudo
    { 0x00E2, 0x0061, 4 },   // â = a + Circunflejo
    { 0x00E3, 0x0061, 1 },   // ã = a + Tilde
    { 0x00E4, 0x0061, 6 },   // ä = a + Dieresis
    { 0x00E7, 0x0063, 12 },   // ç = c + Cedilla
    { 0x00E8, 0x0065, 2 },   // è = e + Grave
    { 0x00E9, 0x0065, 0 },   // é = e + Agudo
    { 0x00EA, 0x0065, 4 },   // ê = e + Circunflejo
    { 0x00EB, 0x0065, 6 },   // ë = e + Dieresis
    { 0x00EC, 0x0131, 2 },   // ì = ı + Grave
    { 0x00ED, 0x0069, 0 },   // í = i + Agudo
    { 0x00EE, 0x0131, 4 },   // î = ı + Circunflejo
    { 0x00EF, 0x0131, 6 },   // ï = ı + Dieresis
    { 0x00F2, 0x006F, 2 },   // ò = o + Grave
    { 0x00F3, 0x006F, 0 },   // ó = o + Agudo
    { 0x00F4, 0x006F, 4 },   // ô = o + Circunflejo
    { 0x00F5, 0x006F, 1 },   // õ = o + Tilde
    { 0x00F6, 0x006F, 6 },   // ö = o + Dieresis
    { 0x00F9, 0x0075, 2 },   // ù = u + Grave
    { 0x00FA, 0x0075, 0 },   // ú = u + Agudo
    { 0x00FB, 0x0075, 4 },   // û = u + Circunflejo
    { 0x00FC, 0x0075, 6 },   // ü = u + Dieresis
    { 0x00FD, 0x0079, 0 },   // ý = y + Agudo
    { 0x00FF, 0x0079, 6 },   // ÿ = y + Dieresis
    { 0x0104, 0x0041, 13 },   // Ą = A + Ogonek
    { 0x0105, 0x0061, 13 },   // ą = a + Ogonek
    { 0x0106, 0x0043, 0 },   // Ć = C + Agudo
    { 0x0107, 0x0063, 0 },   // ć = c + Agudo
    { 0x0118, 0x0045, 13 },   // Ę = E + Ogonek
    { 0x0119, 0x0065, 13 },   // ę = e + Ogonek
    { 0x011E, 0x0047, 11 },   // Ğ = G + Breve
    { 0x011F, 0x0067, 10 },   // ğ = g + Breve
    { 0x0130, 0x0049, 9 },   // İ = I + Punto
    { 0x0143, 0x004E, 0 },   // Ń = N + Agudo
    { 0x0144, 0x006E, 0 },   // ń = n + Agudo
    { 0x015A, 0x0053, 0 },   // Ś = S + Agudo
    { 0x015B, 0x0073, 0 },   // ś = s + Agudo
    { 0x015E, 0x0053, 12 },   // Ş = S + Cedilla
    { 0x015F, 0x0073, 12 },   // ş = s + Cedilla
    { 0x0178, 0x0059, 7 },   // Ÿ = Y + Dieresis
    { 0x0179, 0x005A, 0 },   // Ź = Z + Agudo
    { 0x017A, 0x007A, 0 },   // ź = z + Agudo
    { 0x017B, 0x005A, 9 },   // Ż = Z + Punto
    { 0x017C, 0x007A, 8 },   // ż = z + Punto
    { 0x0401, 0x0045, 7 },   // Ё = E + Dieresis
    { 0x0419, 0x0418, 11 },   // Й = И + Breve
    { 0x0439, 0x0438, 10 },   // й = и + Breve
    { 0x0451, 0x0065, 6 },   // ё = e + Dieresis
};
static const unsigned W3dFuenteExtCompuestasN = 73;

#endif
