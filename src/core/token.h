#ifndef CORE_TOKEN_H
#define CORE_TOKEN_H

#include <string>
#include "token_type.h"

// Capa: CORE (entidad de dominio).
//
// ComponenteLexico es la unidad minima que produce el escaner: una
// categoria (tipo), el texto exacto leido del fuente (lexema) y la
// posicion donde empieza (linea y columna, ambas base 1).
//
// Es agnostica al lenguaje: no sabe imprimir, no lee archivos y no
// conoce palabras reservadas. Solo transporta el dato puro desde el
// motor (engine) hacia la presentacion (infrastructure) o hacia un
// futuro analizador sintactico (parser).
struct ComponenteLexico {
    // Categoria del componente (ver TipoComponente).
    TipoComponente tipo;

    // Texto exacto leido del fuente. Incluye comillas en los literales
    // de cadena y de caracter (ej. "\"hola\"", "'\n'").
    std::string lexema;

    // Linea del fuente donde empieza el lexema (base 1).
    int linea;

    // Columna del fuente donde empieza el lexema (base 1).
    int columna;

    // Construye un componente con su categoria, lexema y posicion inicial.
    ComponenteLexico(TipoComponente t, std::string lex, int lin, int col)
        : tipo(t), lexema(std::move(lex)), linea(lin), columna(col) {}
};

#endif // CORE_TOKEN_H
