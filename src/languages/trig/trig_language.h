#ifndef LANG_TRIG_LANGUAGE_H
#define LANG_TRIG_LANGUAGE_H

#include "../../core/language_spec.h"

// Capa: LANGUAGES/trig (plugin concreto para expresiones trigonometricas).
//
// Implementa IEspecificacionLenguaje para reutilizar exactamente el mismo
// motor AFD de scanner_c sin modificarlo. Define las funciones reservadas
// (sin, sen, cos, tan) y los nombres legibles de cada token permitido en
// expresiones aritmetico-trigonometricas.
class EspecificacionTrigonometrica : public IEspecificacionLenguaje {
public:
    // Retorna la instancia unica compartida (singleton).
    static const EspecificacionTrigonometrica& instancia();

    std::string nombreLenguaje() const override { return "Expresiones Trigonometricas"; }

    const std::unordered_map<std::string, TipoComponente>& palabrasReservadas() const override;

    const std::unordered_map<std::string, TipoComponente>& directivas() const override;

    std::string nombreDeTipo(TipoComponente t) const override;

    // Indica si un token emitido por el AFD pertenece al subconjunto valido
    // del lenguaje de expresiones trigonometricas.
    static bool esTokenPermitido(TipoComponente t);

private:
    EspecificacionTrigonometrica() = default;
};

#endif // LANG_TRIG_LANGUAGE_H
