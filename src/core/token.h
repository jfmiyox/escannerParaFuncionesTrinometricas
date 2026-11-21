#ifndef CORE_TOKEN_H
#define CORE_TOKEN_H

#include <string>
#include "token_type.h"

struct ComponenteLexico {
    TipoComponente tipo;

    std::string lexema;

    int linea;

    int columna;

    ComponenteLexico(TipoComponente t, std::string lex, int lin, int col)
        : tipo(t), lexema(std::move(lex)), linea(lin), columna(col) {}
};

#endif
