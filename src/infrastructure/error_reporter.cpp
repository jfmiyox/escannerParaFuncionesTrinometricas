#include "error_reporter.h"
#include <iostream>
#include <iomanip>

// Imprime el cierre del analisis: exito o lista numerada de errores.
// Los numeros de linea/columna se alinean con setw para que el bloque
// quede en columnas legibles aunque haya posiciones de 1 o 3 digitos.
void imprimirResumen(const std::vector<ErrorLexico>& errores) {
    if (errores.empty()) {
        std::cout << "\n[OK] Escaneo completado sin errores lexicos.\n";
        return;
    }
    const std::string separador(50, '=');
    std::cerr << "\n" << separador << "\n";
    std::cerr << "  RESUMEN: " << errores.size() << " error(es) lexico(s)\n";
    std::cerr << separador << "\n";
    for (size_t i = 0; i < errores.size(); i++) {
        const auto& e = errores[i];
        std::cerr << "  [" << (i+1) << "] Linea " << std::setw(3) << e.linea
                  << ", Col " << std::setw(3) << e.columna
                  << " : " << e.mensaje;
        if (!e.contexto.empty())
            std::cerr << " -> \"" << e.contexto << "\"";
        std::cerr << "\n";
    }
    std::cerr << separador << "\n";
}
