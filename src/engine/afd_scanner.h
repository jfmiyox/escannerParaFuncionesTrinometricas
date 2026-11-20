#ifndef ENGINE_AFD_SCANNER_H
#define ENGINE_AFD_SCANNER_H

#include <string>
#include <vector>
#include "../core/token.h"
#include "../core/lex_error.h"
#include "../core/language_spec.h"
#include "char_source.h"

// Capa: ENGINE (AFD generico, agnostico al lenguaje).
//
// EscanerAFD implementa el automata finito determinista por despacho
// directo (sin Lex/Flex). La UNICA dependencia de lenguaje es la
// especificacion inyectada (IEspecificacionLenguaje): de ahi salen las
// palabras reservadas y las directivas. Cambiar de lenguaje equivale a
// inyectar otra especificacion, sin tocar una linea de este motor.
//
// Politica de errores (modo recovery): cada fallo se registra en la
// lista de errores y el escaneo continua desde el caracter siguiente,
// de modo que una pasada reporta TODOS los errores, no solo el primero.
//
// Restriccion de vida: la especificacion debe vivir mas que el escaner
// (en la practica es un singleton por idioma, ej. EspecificacionC).
class EscanerAFD {
public:
    // Construye el escaner sobre el fuente completo y la especificacion
    // del lenguaje a reconocer.
    EscanerAFD(const std::string& fuente, const IEspecificacionLenguaje& especificacion);

    // Escanea todo el fuente y retorna la lista de componentes,
    // garantizando un END_OF_FILE como ultimo elemento.
    std::vector<ComponenteLexico> tokenizar();

    // Indica si durante el escaneo se registro al menos un error lexico.
    bool tieneErrores() const { return !errores_.empty(); }

    // Retorna los errores acumulados, en orden de aparicion.
    const std::vector<ErrorLexico>& errores() const { return errores_; }

private:
    // Analiza un identificador o palabra reservada: consume
    // [letra/digito/_]* y luego consulta la tabla del lenguaje.
    // Lo ausente en la tabla se clasifica como ID.
    ComponenteLexico analizarIdentificador();

    // Analiza un numero: hexadecimal (0x...), flotante o entero decimal.
    // Un "0x" sin digitos hexadecimales se reporta como error.
    ComponenteLexico analizarNumero();

    // Analiza un literal de cadena entre comillas dobles, validando las
    // secuencias de escape. El salto de linea o el fin de archivo antes
    // del cierre se reportan como error (modo recovery: igual retorna
    // el componente parcial para poder continuar).
    ComponenteLexico analizarCadena();

    // Analiza un literal de caracter entre comillas simples, con o sin
    // secuencia de escape. La falta de cierre se reporta como error.
    ComponenteLexico analizarCaracter();

    // Omite un comentario de linea (desde "//" hasta antes de '\n').
    // No produce componente: solo adelanta el cursor.
    void omitirComentarioLinea();

    // Omite un comentario de bloque (desde "/*" hasta "*/").
    // Si el archivo termina antes del cierre, reporta el error.
    // No produce componente: solo adelanta el cursor.
    void omitirComentarioBloque();

    // Analiza una directiva de preprocesador: consume '#', omite espacios
    // y lee el nombre. Si no existe en la tabla del lenguaje, reporta
    // el error y retorna UNKNOWN.
    ComponenteLexico analizarDirectiva();

    // Analiza operadores y puntuacion (simples, dobles y "...").
    // Ante un caracter sin regla asociada, reporta el error y retorna
    // UNKNOWN con ese caracter como lexema.
    ComponenteLexico analizarOperadorOPuntuacion();

    // Funcion principal del AFD: reparte cada posicion inicial hacia el
    // estado que corresponde segun el caracter actual. Omite espacios y
    // comentarios en un bucle (sin recursion) y retorna el siguiente
    // componente real o END_OF_FILE.
    ComponenteLexico siguienteElemento();

    // Registra un error lexico con la posicion actual del cursor.
    void reportarError(const std::string& mensaje, const std::string& contexto);

    // Crea un componente con la posicion actual del cursor.
    // Se usa para END_OF_FILE, donde inicio y deteccion coinciden.
    ComponenteLexico crearElemento(TipoComponente tipo, const std::string& lexema);

    FuenteCaracteres              entrada_;        // cursor sobre el fuente
    const IEspecificacionLenguaje& especificacion_; // tablas del lenguaje activo
    std::vector<ErrorLexico>      errores_;        // fallos acumulados (recovery)
};

#endif // ENGINE_AFD_SCANNER_H
