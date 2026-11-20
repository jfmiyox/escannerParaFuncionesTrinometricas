#ifndef CORE_LEX_ERROR_H
#define CORE_LEX_ERROR_H

#include <string>

// Capa: CORE (entidad de dominio).
//
// ErrorLexico es el dato puro de un fallo durante el escaneo: que paso,
// con que texto problematico y en que posicion del fuente. El motor lo
// acumula sin detenerse (modo recovery) y el formateo visual para el
// usuario vive en infrastructure (imprimirResumen), no aqui.
struct ErrorLexico {
    // Descripcion del problema (ej. "Caracter no reconocido: '@'").
    std::string mensaje;

    // Fragmento problematico que origino el error (ej. "@").
    // Puede estar vacio si no hay un lexema concreto que mostrar.
    std::string contexto;

    // Linea del fuente donde se detecto el error (base 1).
    int linea;

    // Columna del fuente donde se detecto el error (base 1).
    int columna;
};

#endif // CORE_LEX_ERROR_H
