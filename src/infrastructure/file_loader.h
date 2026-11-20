#ifndef INFRA_FILE_LOADER_H
#define INFRA_FILE_LOADER_H

#include <string>

// Capa: INFRASTRUCTURE (adaptador de entrada/salida).
//
// Este modulo aisla el acceso al sistema de archivos para que el caso de
// uso solo reciba un std::string ya cargado. Contiene las validaciones de
// apertura y de archivo vacio; asi el nucleo puede analizar textos que no
// vienen de disco (pruebas unitarias, editor integrado, etc.).
struct ResultadoCarga {
    // True si el archivo se pudo abrir y leer (aunque este vacio).
    bool exito = false;

    // Contenido completo del archivo. Valido solo si exito es true.
    std::string fuente;

    // Mensaje de error legible. Valido solo si exito es false.
    std::string error;

    // True si el archivo se abrio pero no tenia contenido.
    // Es advertencia, no error fatal: el CLI retorna exito en ese caso.
    bool estaVacio = false;
};

// Lee el archivo indicado y retorna su contenido o el motivo del fallo.
// Nunca lanza excepciones: todo fallo se comunica en el resultado.
ResultadoCarga cargarArchivoFuente(const std::string& ruta);

#endif // INFRA_FILE_LOADER_H
