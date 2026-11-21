#ifndef LANG_TRIG_LANGUAGE_H
#define LANG_TRIG_LANGUAGE_H

#include "../../core/language_spec.h"

class EspecificacionTrigonometrica : public IEspecificacionLenguaje {
public:
    static const EspecificacionTrigonometrica& instancia();

    std::string nombreLenguaje() const override { return "Expresiones Trigonometricas"; }

    const std::unordered_map<std::string, TipoComponente>& palabrasReservadas() const override;

    const std::unordered_map<std::string, TipoComponente>& directivas() const override;

    std::string nombreDeTipo(TipoComponente t) const override;

    static bool esTokenPermitido(TipoComponente t);

private:
    EspecificacionTrigonometrica() = default;
};

#endif
