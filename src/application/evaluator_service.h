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

// Resultado consolidado de las fases lexica y sintactica para una expresion.
struct ResultadoAnalisisExpresion {
    bool exito = false;
    std::vector<ComponenteLexico> componentes;
    std::vector<ErrorLexico> erroresLexicos;
    std::vector<std::string> variables;
    std::optional<ErrorAnalisis> errorParser;
};

// Capa: APPLICATION (caso de uso principal).
//
// Orquesta el escaner AFD (reutilizado de scanner_c con EspecificacionTrigonometrica)
// y el parser descendente LL(1) con evaluacion directa.
class ServicioEvaluador {
public:
    // Ejecuta analisis lexico y validacion sintactica LL(1), detectando
    // las variables simbolicas presentes en orden de aparicion.
    ResultadoAnalisisExpresion analizar(const std::string& expresion) const;

    // Evalua una expresion previamente analizada sustituyendo cada variable
    // por su valor numerico en `valores`, usando la unidad angular indicada.
    ResultadoEvaluacion evaluar(
        const std::vector<ComponenteLexico>& componentes,
        const std::unordered_map<std::string, double>& valores,
        ModoAngulo modo = ModoAngulo::RADIANES) const;

    // Parsea y valida estrictamente una entrada numerica real del usuario
    // (rechaza texto vacio, hexadecimales, NaN, Inf y basura al final).
    static bool parsearNumeroReal(const std::string& texto, double& salida);
};

#endif // APP_EVALUATOR_SERVICE_H
