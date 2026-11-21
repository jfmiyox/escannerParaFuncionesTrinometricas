#ifndef INFRA_FILE_LOADER_H
#define INFRA_FILE_LOADER_H

#include <string>

struct ResultadoCarga {
    bool exito = false;

    std::string fuente;

    std::string error;

    bool estaVacio = false;
};

ResultadoCarga cargarArchivoFuente(const std::string& ruta);

#endif
