#include "token_printer.h"
#include <iostream>

void imprimirComponentes(const std::vector<ComponenteLexico>& componentes,
                         const IEspecificacionLenguaje& especificacion) {
    for (const ComponenteLexico& actual : componentes) {
        if (actual.tipo == TipoComponente::END_OF_FILE) break;
        std::cout << "Token: " << especificacion.nombreDeTipo(actual.tipo)
                  << " \"" << actual.lexema << "\""
                  << " (línea " << actual.linea << ", col " << actual.columna << ")\n";
    }
}
