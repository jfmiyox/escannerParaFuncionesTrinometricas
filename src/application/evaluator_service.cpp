#include "evaluator_service.h"
#include "../languages/trig/trig_language.h"
#include "scanner_service.h"
#include <cctype>
#include <cmath>
#include <cstdlib>

ResultadoAnalisisExpresion ServicioEvaluador::analizar(const std::string& expresion) const {
    ResultadoAnalisisExpresion salida;

    const EspecificacionTrigonometrica& spec = EspecificacionTrigonometrica::instancia();
    ServicioEscaner escaner(spec);
    ResultadoAnalisis lex = escaner.analizar(expresion);

    salida.componentes = std::move(lex.componentes);
    salida.erroresLexicos = std::move(lex.errores);

    if (!salida.erroresLexicos.empty()) {
        return salida;
    }

    ParserTrigonometrico parser(salida.componentes);
    ResultadoSintaxis sintaxis = parser.validarYDetectarVariables();
    if (!sintaxis.exito) {
        salida.errorParser = sintaxis.error;
        return salida;
    }

    salida.exito = true;
    salida.variables = std::move(sintaxis.variables);
    return salida;
}

ResultadoEvaluacion ServicioEvaluador::evaluar(
    const std::vector<ComponenteLexico>& componentes,
    const std::unordered_map<std::string, double>& valores,
    ModoAngulo modo) const {
    ParserTrigonometrico parser(componentes);
    return parser.evaluar(valores, modo);
}

bool ServicioEvaluador::parsearNumeroReal(const std::string& texto, double& salida) {
    size_t inicio = 0;
    while (inicio < texto.size() && std::isspace(static_cast<unsigned char>(texto[inicio]))) {
        inicio++;
    }
    size_t fin = texto.size();
    while (fin > inicio && std::isspace(static_cast<unsigned char>(texto[fin - 1]))) {
        fin--;
    }
    if (inicio == fin) return false;

    std::string limpio = texto.substr(inicio, fin - inicio);

    size_t i = 0;
    if (limpio[i] == '+' || limpio[i] == '-') i++;
    if (i >= limpio.size()) return false;

    bool tieneDigitos = false;
    bool tienePunto = false;
    for (; i < limpio.size(); i++) {
        char c = limpio[i];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            tieneDigitos = true;
        } else if (c == '.' && !tienePunto) {
            tienePunto = true;
        } else {
            break;
        }
    }
    if (!tieneDigitos) return false;

    if (i < limpio.size()) {
        if (limpio[i] != 'e' && limpio[i] != 'E') return false;
        i++;
        if (i < limpio.size() && (limpio[i] == '+' || limpio[i] == '-')) i++;
        bool tieneDigitosExp = false;
        for (; i < limpio.size(); i++) {
            if (!std::isdigit(static_cast<unsigned char>(limpio[i]))) return false;
            tieneDigitosExp = true;
        }
        if (!tieneDigitosExp) return false;
    }

    char* finPtr = nullptr;
    double valor = std::strtod(limpio.c_str(), &finPtr);
    if (finPtr != limpio.c_str() + limpio.size() || !std::isfinite(valor)) {
        return false;
    }

    salida = valor;
    return true;
}
