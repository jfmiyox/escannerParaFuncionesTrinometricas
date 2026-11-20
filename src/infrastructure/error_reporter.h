#ifndef INFRA_ERROR_REPORTER_H
#define INFRA_ERROR_REPORTER_H

#include <vector>
#include "../core/lex_error.h"

// Capa: INFRASTRUCTURE (presentacion de errores).
//
// Muestra el resumen final de errores lexicos. Sin errores imprime la
// confirmacion en stdout; con errores imprime el bloque "RESUMEN" en
// stderr con indice, posicion, mensaje y contexto de cada fallo.
// El motor solo acumula ErrorLexico; todo el formato vive aqui.
void imprimirResumen(const std::vector<ErrorLexico>& errores);

#endif // INFRA_ERROR_REPORTER_H
