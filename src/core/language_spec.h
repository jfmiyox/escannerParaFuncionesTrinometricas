#ifndef CORE_LANGUAGE_SPEC_H
#define CORE_LANGUAGE_SPEC_H

#include <string>
#include <unordered_map>
#include "token_type.h"

// Capa: CORE (puerto / interfaz).
//
// IEspecificacionLenguaje es EL punto de extension multi-lenguaje.
// El motor AFD (engine) solo habla contra esta interfaz; jamas incluye
// tablas concretas de C. Para soportar otro lenguaje basta crear una
// clase nueva que la implemente (ver src/languages/c/c_language.h).
//
// Contrato que debe cumplir cada implementacion:
// - palabrasReservadas(): tabla palabra -> categoria. Lo no encontrado
//   se clasifica como ID.
// - directivas(): tabla directiva (sin '#') -> categoria.
// - nombreDeTipo(): nombre imprimible de cada categoria en ese lenguaje.
class IEspecificacionLenguaje {
public:
    // Destructor virtual para permitir borrado polimorfico seguro.
    virtual ~IEspecificacionLenguaje() = default;

    // Retorna el nombre del lenguaje ("C", "MiniPascal"...).
    // Es solo informativo (mensajes y documentacion).
    virtual std::string nombreLenguaje() const = 0;

    // Retorna la tabla palabra -> categoria (ej. "int" -> INT).
    virtual const std::unordered_map<std::string, TipoComponente>& palabrasReservadas() const = 0;

    // Retorna la tabla directiva (sin '#') -> categoria
    // (ej. "include" -> INCLUDE).
    virtual const std::unordered_map<std::string, TipoComponente>& directivas() const = 0;

    // Retorna el nombre imprimible de una categoria en este lenguaje
    // (ej. TipoComponente::INT -> "INT").
    virtual std::string nombreDeTipo(TipoComponente t) const = 0;
};

#endif // CORE_LANGUAGE_SPEC_H
