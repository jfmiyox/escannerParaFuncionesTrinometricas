#ifndef INFRA_TOKEN_PRINTER_H
#define INFRA_TOKEN_PRINTER_H

#include <vector>
#include "../core/token.h"
#include "../core/language_spec.h"

// Capa: INFRASTRUCTURE (presentacion).
//
// Imprime la lista de componentes en salida estandar con el formato
// oficial del proyecto:
//   Token: TIPO "lexema" (línea X, col Y)
// El marcador END_OF_FILE no se imprime: solo es centinela interno.
void imprimirComponentes(const std::vector<ComponenteLexico>& componentes,
                         const IEspecificacionLenguaje& especificacion);

#endif // INFRA_TOKEN_PRINTER_H
