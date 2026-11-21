#ifndef PARSER_PARSE_ERROR_H
#define PARSER_PARSE_ERROR_H

#include <string>

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

#endif
