#ifndef CORE_LANGUAGE_SPEC_H
#define CORE_LANGUAGE_SPEC_H

#include <string>
#include <unordered_map>
#include "token_type.h"

class IEspecificacionLenguaje {
public:
    virtual ~IEspecificacionLenguaje() = default;

    virtual std::string nombreLenguaje() const = 0;

    virtual const std::unordered_map<std::string, TipoComponente>& palabrasReservadas() const = 0;

    virtual const std::unordered_map<std::string, TipoComponente>& directivas() const = 0;

    virtual std::string nombreDeTipo(TipoComponente t) const = 0;
};

#endif
