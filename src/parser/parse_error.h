#ifndef PARSER_PARSE_ERROR_H
#define PARSER_PARSE_ERROR_H

#include <string>

// Capa: PARSER (entidad de error de analisis y evaluacion).
//
// Representa un fallo detectado durante la validacion de tokens, el
// analisis sintactico LL(1) o la evaluacion matematica de la expresion.
enum class FaseError {
    LEXICO,
    SINTACTICO,
    MATEMATICO
};

struct ErrorAnalisis {
    FaseError fase;
    std::string mensaje;
    std::string contexto;
    int linea;
    int columna;

    std::string nombreFase() const {
        switch (fase) {
            case FaseError::LEXICO:     return "Error lexico";
            case FaseError::SINTACTICO: return "Error sintactico";
            case FaseError::MATEMATICO: return "Error matematico";
        }
        return "Error";
    }
};

#endif // PARSER_PARSE_ERROR_H
