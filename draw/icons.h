#ifndef ICONS_H
#define ICONS_H

#ifdef _WIN32
#ifndef W3D_SYMBIAN
    #include <windows.h>
#endif
#endif

#include <cstddef>
#include <vector>
#ifdef W3D_SYMBIAN
    #include <GLES/gl.h>
#else
    #include <GL/gl.h>
#endif

#include "UI.h" // para RenglonHeight

// Tamaños iniciales
extern GLshort IconMesh[8];
extern GLshort IconLineMesh[8];
extern int IconSize;
extern int IconSizeGS;

const size_t ICON_TOTAL = 70; // (constexpr es C++11)

// dialecto C++03 compartido
struct IconType {
    enum Enum {
    camera,
    light,
    mesh,
    visible,
    hidden,
    archive,
    line,
    arrow,
    arrowRight,
    mirror,
    array,
    gamepad,
    instance,
    constraint,
    curve,
    textura,
    empty,
    lista,
    cuadricula,
    carpeta,
    foto,
    object,    // cubo (pestania "Objeto" y Add > Cube)
    material,  // pestania de mesh parts
    plane,     // Add > Plane
    circle,    // Add > Circle
    cono,      // Add > Cone
    normalVertex, // overlay normales: vertex normal
    normalCustom, // overlay normales: custom/split normal
    normalFace,   // overlay normales: face normal
    cilindro,     // Add > Cylinder
    selVertex,    // Edit Mode: seleccion de vertices (default)
    selEdge,      // Edit Mode: seleccion de bordes
    selFace,      // Edit Mode: seleccion de caras
    pivotMedian,     // Transform Pivot: punto medio
    pivotIndividual, // Transform Pivot: origenes individuales
    pivotActive,     // Transform Pivot: elemento activo
    pivotCursor,     // Transform Pivot: cursor 3D
    notifError,      // notificacion de ERROR (se pinta rojo)
    notifOk,         // notificacion OK/exito (se pinta verde)
    modificador,     // pestania "Modificadores" en Properties
    armature,        // esqueleto (Add > Armature / outliner)
    keyframe,        // rombo: tarjeta "Keyframe" del panel de propiedades
    monitor,         // pantalla: el menu "View" (sin texto, para ahorrar ancho de barra)
    snap,            // iman: el boton "Snap" de la barra del viewport3d
    guardar,         // diskette: Guardar proyecto (tarjeta Archivo)
    camera_off,      // camara TACHADA: el objeto no se renderiza (outliner)
    borrar,          // tacho: item "Borrar" del menu Animacion (Delete Keyframe)
    mas,             // signo +: menu "Add"/Anadir de la barra del viewport
    seleccion,       // cursor/marco: boton "Seleccionar" de la barra (reemplaza el texto largo)
    // FALLOFF: la curva de caida de una herramienta con radio (menu Falloff del pincel). Cada
    // uno DIBUJA su curva, asi se eligen de un vistazo sin leer. Van en el orden del enum
    // W3dFalloffTipo salvo los que todavia no tienen arte (smooth, sharper, inverse square),
    // que caen a texto hasta que llegue el png.
    falloff_custom,
    falloff_smoother,
    falloff_sphere,
    falloff_root,
    falloff_sharp,
    falloff_linear,
    falloff_constant
        , menos,  // signo -: modo "Restar" de una capa del Mix de animaciones
        igual,    // signo =: el modo Mix (capas mezcladas) y el modo "Mezclar" de una capa
        niebla,   // tres lineas onduladas: el objeto Niebla (glFog en el arbol)
        limpiarz, // capas con una goma: el objeto Limpiar Z
        recorte,  // rectangulo punteado dentro de la pantalla: el objeto Recorte
        hitbox,   // caja de alambre con una marca: el objeto Hitbox (caja de deteccion)
        // el OUTLINER POR RECURSOS (selector de vista): los tipos de recurso que no tenian icono
        animacion, // tira de pelicula con un "play": los animsets (clips de esqueleto)
        prefab,    // caja en perspectiva: un prefab (una "clase" instanciable)
        libreria,  // libros en un estante: una libreria externa (otro .w3d de solo lectura)
        // (las raices ESCENA y JUEGO usan los iconos que ya habia: camera y gamepad)
        // la BIBLIOTECA (el outliner como explorador del .w3d): los tipos de archivo y el filtro
        sonido,    // parlante con ondas: un sonido (.wav / .ogg)
        script,    // hoja con llaves: un script .lua
        video,     // pantalla con un "play": un video
        filtro,    // embudo: el filtro por tipo de la biblioteca
        // las LIBRERIAS externas: lo que genera un proxy W3D es de SOLO LECTURA (outliner)
        candado    // candado cerrado: de solo lectura (lo generado por un proxy de una libreria)
    };
    Enum v;
    IconType(Enum e) : v(e) {}
    operator Enum() const { return v; }
};

struct IconRect {
    int x, y, w, h;
};

class IconUV {
    public:
        GLfloat uvs[8];
#ifdef W3D_SYMBIAN
        GLfloat uvsTri[12]; // strip expandido (driver del N95)
#endif
};

// ===================================================
//  Vector global de íconos UV
// ===================================================
extern std::vector<IconUV*> IconsUV;

// nombre estable de cada icono (el archivo atlas/iconos/<nombre>.png del skin)
const char* IconoNombre(int i);
// el INVERSO: indice del icono que se llama asi, -1 si no existe. Lo usa el codigo que elige
// el icono por NOMBRE en vez de por enum (ej: el falloff, que ya nombra su arte en su tabla).
int IconoIndice(const char* nombre);
// arma IconsUV desde una lista de rects EXTERNA (el atlas dinamico); un rect con
// x = -1 usa el arte legacy de font.png (que vive en el (0,0) del atlas nuevo)
void CrearIconosDesde(const IconRect* lista, int texW, int texH);

// ===================================================
// Funciones
// ===================================================
void SetIconScale(int scale);
void CrearIconos(int texW, int texH);

#endif