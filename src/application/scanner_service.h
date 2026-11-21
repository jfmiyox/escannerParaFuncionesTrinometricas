#ifndef APP_SCANNER_SERVICE_H
#define APP_SCANNER_SERVICE_H

#include <string>
#include <vector>
#include "../core/token.h"
#include "../core/lex_error.h"
#include "../core/language_spec.h"

struct ResultadoAnalisis {
    std::vector<ComponenteLexico> componentes;

    std::vector<ErrorLexico> errores;

    bool tieneErrores() const { return !errores.empty(); }
};

class ServicioEscaner {
public:
    explicit ServicioEscaner(const IEspecificacionLenguaje& especificacion);

    ResultadoAnalisis analizar(const std::string& fuente) const;

private:
    const IEspecificacionLenguaje& especificacion_;
};

#endif
