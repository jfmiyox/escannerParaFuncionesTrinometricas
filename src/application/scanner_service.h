#ifndef APP_SCANNER_SERVICE_H
#define APP_SCANNER_SERVICE_H

#include <string>
#include <vector>
#include "../core/token.h"
#include "../core/lex_error.h"
#include "../core/language_spec.h"

// Capa: APPLICATION (caso de uso).
//
// ServicioEscaner es el orquestador: recibe el fuente como texto y la
// especificacion del lenguaje, ejecuta el motor AFD y devuelve el
// resultado empaquetado. El CLI y futuros clientes (parser, pruebas,
// otros frontends) hablan contra esta clase, nunca contra el AFD
// directamente. Cambiar de lenguaje es solo inyectar otra
// IEspecificacionLenguaje en el constructor.
struct ResultadoAnalisis {
    // Componentes reconocidos, terminados en END_OF_FILE.
    std::vector<ComponenteLexico> componentes;

    // Errores lexicos hallados, en orden de aparicion.
    std::vector<ErrorLexico> errores;

    // Indica si el analisis produjo al menos un error lexico.
    bool tieneErrores() const { return !errores.empty(); }
};

class ServicioEscaner {
public:
    // Construye el servicio fijando el lenguaje a reconocer.
    // La especificacion debe vivir mas que el servicio.
    explicit ServicioEscaner(const IEspecificacionLenguaje& especificacion);

    // Analiza el fuente completo y retorna componentes y errores.
    // No toca disco ni imprime: es reutilizable desde tests y parser.
    ResultadoAnalisis analizar(const std::string& fuente) const;

private:
    // Lenguaje activo, inyectado por el cliente (ej. EspecificacionC).
    const IEspecificacionLenguaje& especificacion_;
};

#endif // APP_SCANNER_SERVICE_H
