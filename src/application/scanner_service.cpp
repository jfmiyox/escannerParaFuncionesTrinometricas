#include "scanner_service.h"
#include "../engine/afd_scanner.h"

ServicioEscaner::ServicioEscaner(const IEspecificacionLenguaje& especificacion)
    : especificacion_(especificacion) {}

ResultadoAnalisis ServicioEscaner::analizar(const std::string& fuente) const {
    EscanerAFD motor(fuente, especificacion_);
    std::vector<ComponenteLexico> componentes = motor.tokenizar();
    return ResultadoAnalisis{std::move(componentes), motor.errores()};
}
