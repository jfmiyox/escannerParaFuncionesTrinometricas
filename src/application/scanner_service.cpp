#include "scanner_service.h"
#include "../engine/afd_scanner.h"

// Guarda la referencia al lenguaje; el trabajo real ocurre en analizar().
ServicioEscaner::ServicioEscaner(const IEspecificacionLenguaje& especificacion)
    : especificacion_(especificacion) {}

// Crea un motor AFD con el lenguaje inyectado, tokeniza el fuente y
// empaqueta componentes mas errores en un unico resultado.
ResultadoAnalisis ServicioEscaner::analizar(const std::string& fuente) const {
    EscanerAFD motor(fuente, especificacion_);
    std::vector<ComponenteLexico> componentes = motor.tokenizar();
    return ResultadoAnalisis{std::move(componentes), motor.errores()};
}
