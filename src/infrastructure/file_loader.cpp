#include "file_loader.h"
#include <fstream>
#include <sstream>

// Abre el archivo en modo texto, vuelca su contenido a un string y marca
// si venia vacio. Si no se puede abrir, retorna exito=false con el error.
ResultadoCarga cargarArchivoFuente(const std::string& ruta) {
    ResultadoCarga resultado;
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        resultado.error = "Error: No se puede abrir el archivo '" + ruta + "'";
        return resultado;
    }
    std::ostringstream acumulador;
    acumulador << archivo.rdbuf();
    resultado.fuente = acumulador.str();
    resultado.exito = true;
    resultado.estaVacio = resultado.fuente.empty();
    return resultado;
}
