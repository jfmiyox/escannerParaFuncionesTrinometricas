#ifndef APP_EVALUATOR_SERVICE_H
#define APP_EVALUATOR_SERVICE_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../core/lex_error.h"
#include "../core/token.h"
#include "../parser/parse_error.h"
#include "../parser/trig_parser.h"

struct ResultadoAnalisisExpresion {
    bool exito = false;
    std::vector<ComponenteLexico> componentes;
    std::vector<ErrorLexico> erroresLexicos;
    std::vector<std::string> variables;
    std::optional<ErrorAnalisis> errorParser;
};

class ServicioEvaluador {
public:
    ResultadoAnalisisExpresion analizar(const std::string& expresion) const;

    ResultadoEvaluacion evaluar(
        const std::vector<ComponenteLexico>& componentes,
        const std::unordered_map<std::string, double>& valores,
        ModoAngulo modo = ModoAngulo::RADIANES) const;

    static bool parsearNumeroReal(const std::string& texto, double& salida);
};

#endif
