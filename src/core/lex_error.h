#ifndef CORE_LEX_ERROR_H
#define CORE_LEX_ERROR_H

#include <string>

struct ErrorLexico {
    std::string mensaje;

    std::string contexto;

    int linea;

    int columna;
};

#endif
