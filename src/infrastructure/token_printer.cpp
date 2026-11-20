#include "token_printer.h"
#include <iostream>

// Recorre los componentes y escribe una linea por cada uno en stdout.
// Los nombres de tipo salen de la especificacion del lenguaje activo,
// asi que esta funcion sirve para cualquier idioma sin cambios.
void imprimirComponentes(const std::vector<ComponenteLexico>& componentes,
                         const IEspecificacionLenguaje& especificacion) {
    for (const ComponenteLexico& actual : componentes) {
        if (actual.tipo == TipoComponente::END_OF_FILE) break;
        std::cout << "Token: " << especificacion.nombreDeTipo(actual.tipo)
                  << " \"" << actual.lexema << "\""
                  << " (línea " << actual.linea << ", col " << actual.columna << ")\n";
    }
}
