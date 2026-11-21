#include "trig_language.h"


static const std::unordered_map<std::string, TipoComponente> FUNCIONES_TRIG = {
    {"sin", TipoComponente::FN_SIN},
    {"sen", TipoComponente::FN_SIN},  // alias de sin
    {"cos", TipoComponente::FN_COS},
    {"tan", TipoComponente::FN_TAN},
};

// Este lenguaje no maneja directivas de preprocesador (#).
static const std::unordered_map<std::string, TipoComponente> SIN_DIRECTIVAS = {};

const EspecificacionTrigonometrica& EspecificacionTrigonometrica::instancia() {
    static EspecificacionTrigonometrica unica;
    return unica;
}

const std::unordered_map<std::string, TipoComponente>&
EspecificacionTrigonometrica::palabrasReservadas() const {
    return FUNCIONES_TRIG;
}

const std::unordered_map<std::string, TipoComponente>&
EspecificacionTrigonometrica::directivas() const {
    return SIN_DIRECTIVAS;
}

std::string EspecificacionTrigonometrica::nombreDeTipo(TipoComponente t) const {
    switch (t) {
        case TipoComponente::FN_SIN:      return "FUNC_SIN";
        case TipoComponente::FN_COS:      return "FUNC_COS";
        case TipoComponente::FN_TAN:      return "FUNC_TAN";
        case TipoComponente::ID:          return "VAR";
        case TipoComponente::INT_NUM:     return "INT_NUM";
        case TipoComponente::FLOAT_NUM:   return "FLOAT_NUM";
        case TipoComponente::PLUS:        return "PLUS";
        case TipoComponente::MINUS:       return "MINUS";
        case TipoComponente::MUL_OP:      return "MUL_OP";
        case TipoComponente::DIV_OP:      return "DIV_OP";
        case TipoComponente::XOR_OP:      return "POW_OP";
        case TipoComponente::LPAR:        return "LPAR";
        case TipoComponente::RPAR:        return "RPAR";
        case TipoComponente::END_OF_FILE: return "END_OF_FILE";
        case TipoComponente::UNKNOWN:     return "UNKNOWN";
        default:                          return "TOKEN_NO_SOPORTADO";
    }
}

bool EspecificacionTrigonometrica::esTokenPermitido(TipoComponente t) {
    switch (t) {
        case TipoComponente::FN_SIN:
        case TipoComponente::FN_COS:
        case TipoComponente::FN_TAN:
        case TipoComponente::ID:
        case TipoComponente::INT_NUM:
        case TipoComponente::FLOAT_NUM:
        case TipoComponente::PLUS:
        case TipoComponente::MINUS:
        case TipoComponente::MUL_OP:
        case TipoComponente::DIV_OP:
        case TipoComponente::XOR_OP:
        case TipoComponente::LPAR:
        case TipoComponente::RPAR:
        case TipoComponente::END_OF_FILE:
            return true;
        default:
            return false;
    }
}
