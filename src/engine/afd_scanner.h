#ifndef ENGINE_AFD_SCANNER_H
#define ENGINE_AFD_SCANNER_H

#include <string>
#include <vector>
#include "../core/token.h"
#include "../core/lex_error.h"
#include "../core/language_spec.h"
#include "char_source.h"

class EscanerAFD {
public:
    EscanerAFD(const std::string& fuente, const IEspecificacionLenguaje& especificacion);

    std::vector<ComponenteLexico> tokenizar();

    bool tieneErrores() const { return !errores_.empty(); }

    const std::vector<ErrorLexico>& errores() const { return errores_; }

private:
    ComponenteLexico analizarIdentificador();

    ComponenteLexico analizarNumero();

    ComponenteLexico analizarCadena();

    ComponenteLexico analizarCaracter();

    void omitirComentarioLinea();

    void omitirComentarioBloque();

    ComponenteLexico analizarDirectiva();

    ComponenteLexico analizarOperadorOPuntuacion();

    ComponenteLexico siguienteElemento();

    void reportarError(const std::string& mensaje, const std::string& contexto);

    ComponenteLexico crearElemento(TipoComponente tipo, const std::string& lexema);

    FuenteCaracteres              entrada_;
    const IEspecificacionLenguaje& especificacion_;
    std::vector<ErrorLexico>      errores_;
};

#endif
