#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../src/application/evaluator_service.h"

namespace {

int pruebasEjecutadas = 0;

void verificarCerca(double obtenido, double esperado, double tol = 1e-9, const std::string& caso = "") {
    if (std::abs(obtenido - esperado) > tol) {
        std::cerr << "[FALLO] " << caso << ": esperado=" << esperado
                  << ", obtenido=" << obtenido << "\n";
        std::exit(1);
    }
    pruebasEjecutadas++;
}

void verificarValida(
    const ServicioEvaluador& srv,
    const std::string& expr,
    const std::unordered_map<std::string, double>& vars,
    double esperado,
    const std::vector<std::string>& varsEsperadas = {},
    ModoAngulo modo = ModoAngulo::RADIANES) {
    ResultadoAnalisisExpresion analisis = srv.analizar(expr);
    if (!analisis.exito) {
        std::cerr << "[FALLO] Se esperaba valida: " << expr << "\n";
        if (analisis.errorParser.has_value()) {
            std::cerr << "  Error: " << analisis.errorParser->mensaje << "\n";
        }
        std::exit(1);
    }
    if (!varsEsperadas.empty() && analisis.variables != varsEsperadas) {
        std::cerr << "[FALLO] Variables detectadas no coinciden para: " << expr << "\n";
        std::exit(1);
    }
    ResultadoEvaluacion eval = srv.evaluar(analisis.componentes, vars, modo);
    if (!eval.exito) {
        std::cerr << "[FALLO] Fallo evaluacion de: " << expr << " -> "
                  << (eval.error ? eval.error->mensaje : "") << "\n";
        std::exit(1);
    }
    verificarCerca(eval.valor, esperado, 1e-9, expr);
}

void verificarErrorFase(
    const ServicioEvaluador& srv,
    const std::string& expr,
    FaseError faseEsperada,
    const std::unordered_map<std::string, double>& vars = {},
    ModoAngulo modo = ModoAngulo::RADIANES) {
    ResultadoAnalisisExpresion analisis = srv.analizar(expr);

    if (faseEsperada == FaseError::LEXICO) {
        bool falloLexico = !analisis.erroresLexicos.empty() ||
            (analisis.errorParser.has_value() && analisis.errorParser->fase == FaseError::LEXICO);
        if (!falloLexico) {
            std::cerr << "[FALLO] Se esperaba error LEXICO en: " << expr << "\n";
            std::exit(1);
        }
        pruebasEjecutadas++;
        return;
    }

    if (faseEsperada == FaseError::SINTACTICO) {
        if (analisis.exito || !analisis.errorParser.has_value() ||
            analisis.errorParser->fase != FaseError::SINTACTICO) {
            std::cerr << "[FALLO] Se esperaba error SINTACTICO en: " << expr << "\n";
            std::exit(1);
        }
        pruebasEjecutadas++;
        return;
    }

    // FaseError::MATEMATICO
    if (!analisis.exito) {
        std::cerr << "[FALLO] Debio pasar sintaxis antes de error matematico: " << expr << "\n";
        std::exit(1);
    }
    ResultadoEvaluacion eval = srv.evaluar(analisis.componentes, vars, modo);
    if (eval.exito || !eval.error.has_value() || eval.error->fase != FaseError::MATEMATICO) {
        std::cerr << "[FALLO] Se esperaba error MATEMATICO en: " << expr << "\n";
        std::exit(1);
    }
    pruebasEjecutadas++;
}

void verificarParseoNumeroReal() {
    double val = 0.0;
    assert(ServicioEvaluador::parsearNumeroReal("3.14", val) && std::abs(val - 3.14) < 1e-12);
    assert(ServicioEvaluador::parsearNumeroReal("  -2.5  ", val) && std::abs(val + 2.5) < 1e-12);
    assert(ServicioEvaluador::parsearNumeroReal("+0", val) && val == 0.0);
    assert(ServicioEvaluador::parsearNumeroReal(".5", val) && std::abs(val - 0.5) < 1e-12);
    assert(ServicioEvaluador::parsearNumeroReal("3.", val) && std::abs(val - 3.0) < 1e-12);
    assert(ServicioEvaluador::parsearNumeroReal("-1.5e-3", val) && std::abs(val + 0.0015) < 1e-12);
    assert(ServicioEvaluador::parsearNumeroReal("+2E4", val) && std::abs(val - 20000.0) < 1e-12);
    assert(!ServicioEvaluador::parsearNumeroReal("", val));
    assert(!ServicioEvaluador::parsearNumeroReal("   ", val));
    assert(!ServicioEvaluador::parsearNumeroReal(".", val));
    assert(!ServicioEvaluador::parsearNumeroReal(".e5", val));
    assert(!ServicioEvaluador::parsearNumeroReal("1.5e", val));
    assert(!ServicioEvaluador::parsearNumeroReal("1.5e+", val));
    assert(!ServicioEvaluador::parsearNumeroReal("3445ea", val));
    assert(!ServicioEvaluador::parsearNumeroReal("abc", val));
    assert(!ServicioEvaluador::parsearNumeroReal("12abc", val));
    assert(!ServicioEvaluador::parsearNumeroReal("0x10", val));
    assert(!ServicioEvaluador::parsearNumeroReal("nan", val));
    assert(!ServicioEvaluador::parsearNumeroReal("inf", val));
    assert(!ServicioEvaluador::parsearNumeroReal("1.2.3", val));
    pruebasEjecutadas++;
}

} // namespace

int main() {
    ServicioEvaluador srv;

    // 1. Casos explicitos del Trabajo Practico 3.
    verificarValida(srv, "sen(3.14 / 2) + cos(0) * 2^2", {},
                    std::sin(3.14 / 2.0) + std::cos(0.0) * 4.0);
    verificarValida(srv, "sen(x) + cos(y)", {{"x", 1.0}, {"y", 0.5}},
                    std::sin(1.0) + std::cos(0.5), {"x", "y"});
    verificarValida(srv, "sen(cos(x))+ tan(y)/cos(x) // comentario",
                    {{"x", 0.7}, {"y", 0.4}},
                    std::sin(std::cos(0.7)) + std::tan(0.4) / std::cos(0.7),
                    {"x", "y"});
    verificarValida(srv, "cos(x)^2 + sen(x)^2", {{"x", 1.2345}}, 1.0, {"x"});
    verificarValida(srv, "sin(x)", {{"x", 0.5}}, std::sin(0.5), {"x"});
    verificarValida(srv, "cos(x + y)", {{"x", 0.3}, {"y", 0.7}}, std::cos(1.0), {"x", "y"});
    verificarValida(srv, "tan(x^2 + sin(x))", {{"x", 0.6}},
                    std::tan(0.36 + std::sin(0.6)), {"x"});

    // 2. Distintos tipos de constantes reales y funciones trigonometricas anidadas.
    verificarValida(srv, ".5 + 3.", {}, 3.5);
    verificarValida(srv, "-.25 + +.75", {}, 0.5);
    verificarValida(srv, "1.5e-3 * 2E3", {}, 3.0);
    verificarValida(srv, ".5e2 + 3.e-1", {}, 50.3);
    verificarValida(srv, "sen(cos(tan(0.5)))", {}, std::sin(std::cos(std::tan(0.5))));
    verificarValida(srv, "tan(sin(cos(.5)) + 1.2e-2)", {},
                    std::tan(std::sin(std::cos(0.5)) + 1.2e-2));
    verificarValida(srv, "cos(sen(x) + tan(y * 1e-1))", {{"x", 1.0}, {"y", 0.5}},
                    std::cos(std::sin(1.0) + std::tan(0.05)), {"x", "y"});

    // 3. Precedencia, asociatividad y operadores unarios.
    verificarValida(srv, "2^3^2", {}, 512.0);               // asociatividad derecha: 2^(3^2)
    verificarValida(srv, "-2^2", {}, -4.0);                 // -(2^2)
    verificarValida(srv, "(-2)^2", {}, 4.0);                // (-2)^2
    verificarValida(srv, "2^-2", {}, 0.25);                 // exponente negativo
    verificarValida(srv, "10 - 3 - 2", {}, 5.0);            // asociatividad izquierda
    verificarValida(srv, "24 / 4 / 2", {}, 3.0);            // asociatividad izquierda
    verificarValida(srv, "+3 + -5 * -2", {}, 13.0);         // unarios combinados
    verificarValida(srv, "sin(z) * 2", {{"z", 1.0}}, std::sin(1.0) * 2.0, {"z"});

    // 4. Validaciones lexicas y de tokens no permitidos.
    verificarErrorFase(srv, "sin(x) + @", FaseError::LEXICO);
    verificarErrorFase(srv, "cos(x);", FaseError::LEXICO);
    verificarErrorFase(srv, "0x10 + 1", FaseError::LEXICO);
    verificarErrorFase(srv, "\"hola\"", FaseError::LEXICO);
    verificarErrorFase(srv, "3..1 + 1", FaseError::LEXICO);
    verificarErrorFase(srv, ". + 1", FaseError::LEXICO);

    // 5. Validaciones sintacticas LL(1) y numeros mal formados.
    verificarErrorFase(srv, "", FaseError::SINTACTICO);
    verificarErrorFase(srv, "   // solo comentario", FaseError::SINTACTICO);
    verificarErrorFase(srv, "()", FaseError::SINTACTICO);
    verificarErrorFase(srv, "sin()", FaseError::SINTACTICO);
    verificarErrorFase(srv, "sin(x + 1", FaseError::SINTACTICO);
    verificarErrorFase(srv, "x + 1)", FaseError::SINTACTICO);
    verificarErrorFase(srv, "sin x", FaseError::SINTACTICO);
    verificarErrorFase(srv, "log(x)", FaseError::SINTACTICO);
    verificarErrorFase(srv, "sin(log(x))", FaseError::SINTACTICO);
    verificarErrorFase(srv, "2 + * 3", FaseError::SINTACTICO);
    verificarErrorFase(srv, "5 ^", FaseError::SINTACTICO);
    verificarErrorFase(srv, "* 2", FaseError::SINTACTICO);
    verificarErrorFase(srv, "2 x", FaseError::SINTACTICO);
    verificarErrorFase(srv, "(1+2)(3+4)", FaseError::SINTACTICO);
    verificarErrorFase(srv, "3445ea", FaseError::SINTACTICO);
    verificarErrorFase(srv, "1.5e+", FaseError::SINTACTICO);
    verificarErrorFase(srv, "1.2.3", FaseError::SINTACTICO);

    // 6. Validaciones matematicas.
    verificarErrorFase(srv, "10 / (2 - 2)", FaseError::MATEMATICO);
    verificarErrorFase(srv, "1 / sin(x)", FaseError::MATEMATICO, {{"x", 0.0}});
    verificarErrorFase(srv, "0^-2", FaseError::MATEMATICO);
    verificarErrorFase(srv, "(-4)^0.5", FaseError::MATEMATICO);
    verificarErrorFase(srv, "10^400", FaseError::MATEMATICO);
    verificarErrorFase(srv, "1e400", FaseError::MATEMATICO);
    verificarErrorFase(srv, "x + 1", FaseError::MATEMATICO, {}); // variable faltante

    // 7. Modo angular GRADOS (sin cambiar la gramatica LL(1)).
    verificarValida(srv, "sen(a + c)", {{"a", 45.0}, {"c", 45.0}}, 1.0, {"a", "c"}, ModoAngulo::GRADOS);
    verificarValida(srv, "cos(60) + sen(30) * 2^2", {}, 2.5, {}, ModoAngulo::GRADOS);
    verificarValida(srv, "tan(45)", {}, 1.0, {}, ModoAngulo::GRADOS);
    verificarValida(srv, "cos(90)", {}, 0.0, {}, ModoAngulo::GRADOS);
    verificarErrorFase(srv, "tan(90)", FaseError::MATEMATICO, {}, ModoAngulo::GRADOS);
    verificarErrorFase(srv, "tan(-270)", FaseError::MATEMATICO, {}, ModoAngulo::GRADOS);
    verificarErrorFase(srv, "1 / cos(90)", FaseError::MATEMATICO, {}, ModoAngulo::GRADOS);

    // 8. Validacion de entrada numerica de variables.
    verificarParseoNumeroReal();

    std::cout << "[OK] Todas las pruebas unitarias pasaron (" << pruebasEjecutadas
              << " verificaciones exitosas).\n";
    return 0;
}
