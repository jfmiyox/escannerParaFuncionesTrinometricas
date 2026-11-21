#include "error_reporter.h"
#include <iostream>
#include <iomanip>

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
