#ifndef INFRA_TOKEN_PRINTER_H
#define INFRA_TOKEN_PRINTER_H

#include <vector>
#include "../core/token.h"
#include "../core/language_spec.h"

void imprimirComponentes(const std::vector<ComponenteLexico>& componentes,
                         const IEspecificacionLenguaje& especificacion);

#endif
