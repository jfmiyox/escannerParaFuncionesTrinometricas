#include "trig_parser.h"
#include "../languages/trig/trig_language.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr int MAX_PROFUNDIDAD_RECURSION = 512;
constexpr double PI = 3.14159265358979323846;
}

ParserTrigonometrico::ParserTrigonometrico(const std::vector<ComponenteLexico>& tokens)
    : tokens_(tokens) {}

ResultadoSintaxis ParserTrigonometrico::validarYDetectarVariables() {
    indice_ = 0;
    profundidad_ = 0;
    evaluando_ = false;
    valores_ = nullptr;
    variables_.clear();
    error_.reset();

    if (!validarTokensEntrada()) {
        return ResultadoSintaxis{false, {}, error_};
    }

    parseE();
    if (!tieneError()) {
        verificarFinDeEntrada();
    }

    if (tieneError()) {
        return ResultadoSintaxis{false, {}, error_};
    }
    return ResultadoSintaxis{true, variables_, std::nullopt};
}

ResultadoEvaluacion ParserTrigonometrico::evaluar(
    const std::unordered_map<std::string, double>& valoresVariables,
    ModoAngulo modo) {
    indice_ = 0;
    profundidad_ = 0;
    evaluando_ = true;
    modo_ = modo;
    valores_ = &valoresVariables;
    error_.reset();

    if (!validarTokensEntrada()) {
        return ResultadoEvaluacion{false, 0.0, error_};
    }

    double resultado = parseE();
    if (!tieneError()) {
        verificarFinDeEntrada();
    }

    if (tieneError()) {
        return ResultadoEvaluacion{false, 0.0, error_};
    }
    return ResultadoEvaluacion{true, resultado, std::nullopt};
}

double ParserTrigonometrico::parseE() {
    if (++profundidad_ > MAX_PROFUNDIDAD_RECURSION) {
        reportarError(FaseError::SINTACTICO,
                      "Expresion demasiado anidada (supera el limite de recursion)",
                      actual());
        --profundidad_;
        return 0.0;
    }

    double izquierda = parseT();
    double resultado = tieneError() ? 0.0 : parseEPrima(izquierda);
    --profundidad_;
    return resultado;
}

double ParserTrigonometrico::parseEPrima(double acumulado) {
    TipoComponente tipo = actual().tipo;
    if (tipo != TipoComponente::PLUS && tipo != TipoComponente::MINUS) {
        return acumulado;
    }

    ComponenteLexico op = avanzar();
    double derecha = parseT();
    if (tieneError()) return 0.0;

    double siguiente = acumulado;
    if (evaluando_) {
        siguiente = (op.tipo == TipoComponente::PLUS) ? (acumulado + derecha) : (acumulado - derecha);
        siguiente = validarFinito(siguiente, op, "suma/resta");
        if (tieneError()) return 0.0;
    }
    return parseEPrima(siguiente);
}

double ParserTrigonometrico::parseT() {
    double izquierda = parseU();
    if (tieneError()) return 0.0;
    return parseTPrima(izquierda);
}

double ParserTrigonometrico::parseTPrima(double acumulado) {
    TipoComponente tipo = actual().tipo;
    if (tipo != TipoComponente::MUL_OP && tipo != TipoComponente::DIV_OP) {
        return acumulado;
    }

    ComponenteLexico op = avanzar();
    double derecha = parseU();
    if (tieneError()) return 0.0;

    double siguiente = acumulado;
    if (evaluando_) {
        if (op.tipo == TipoComponente::MUL_OP) {
            siguiente = validarFinito(acumulado * derecha, op, "multiplicacion");
        } else {
            siguiente = dividir(acumulado, derecha, op);
        }
        if (tieneError()) return 0.0;
    }
    return parseTPrima(siguiente);
}

double ParserTrigonometrico::parseU() {
    TipoComponente tipo = actual().tipo;
    if (tipo == TipoComponente::PLUS || tipo == TipoComponente::MINUS) {
        avanzar();
        double valor = parseU();
        if (tieneError()) return 0.0;
        if (!evaluando_) return 0.0;
        return (tipo == TipoComponente::MINUS) ? -valor : valor;
    }
    return parseP();
}

double ParserTrigonometrico::parseP() {
    double base = parseF();
    if (tieneError()) return 0.0;
    return parsePPrima(base);
}

double ParserTrigonometrico::parsePPrima(double base) {
    if (actual().tipo != TipoComponente::XOR_OP) {
        return base;
    }

    // el exponente baja por U: asi 2^3^2 asocia a la derecha y 2^-2 funciona
    ComponenteLexico op = avanzar();
    double exponente = parseU();
    if (tieneError()) return 0.0;
    if (!evaluando_) return 0.0;

    return calcularPotencia(base, exponente, op);
}

double ParserTrigonometrico::parseF() {
    const ComponenteLexico& tok = actual();

    switch (tok.tipo) {
        case TipoComponente::LPAR:
            return parseParentesis();

        case TipoComponente::FN_SIN:
        case TipoComponente::FN_COS:
        case TipoComponente::FN_TAN:
            return parseFuncion();

        case TipoComponente::INT_NUM:
        case TipoComponente::FLOAT_NUM:
            return convertirNumero(avanzar());

        case TipoComponente::ID:
            return resolverVariable(avanzar());

        case TipoComponente::END_OF_FILE:
            reportarError(FaseError::SINTACTICO,
                          "Expresion incompleta: se esperaba un numero, variable, funcion o '('",
                          tok);
            return 0.0;

        case TipoComponente::RPAR:
            reportarError(FaseError::SINTACTICO,
                          "Se esperaba una expresion antes del parentesis de cierre ')'",
                          tok);
            return 0.0;

        default:
            reportarError(FaseError::SINTACTICO,
                          "Operador o token inesperado '" + tok.lexema + "': se esperaba un operando, funcion o '('",
                          tok);
            return 0.0;
    }
}

double ParserTrigonometrico::parseParentesis() {
    ComponenteLexico apertura = avanzar();
    if (actual().tipo == TipoComponente::RPAR) {
        reportarError(FaseError::SINTACTICO,
                      "Parentesis vacios '()': se esperaba una expresion",
                      actual());
        return 0.0;
    }

    double valor = parseE();
    if (tieneError()) return 0.0;

    if (actual().tipo != TipoComponente::RPAR) {
        reportarCierreEsperado("el parentesis abierto en linea " +
                               std::to_string(apertura.linea) + ", col " +
                               std::to_string(apertura.columna));
        return 0.0;
    }
    avanzar();
    return valor;
}

double ParserTrigonometrico::parseFuncion() {
    ComponenteLexico fn = avanzar();

    if (actual().tipo != TipoComponente::LPAR) {
        reportarError(FaseError::SINTACTICO,
                      "Se esperaba '(' despues de la funcion '" + fn.lexema + "'",
                      actual());
        return 0.0;
    }
    avanzar();

    if (actual().tipo == TipoComponente::RPAR) {
        reportarError(FaseError::SINTACTICO,
                      "Argumento vacio en llamada a '" + fn.lexema + "()'",
                      actual());
        return 0.0;
    }

    double argumento = parseE();
    if (tieneError()) return 0.0;

    if (actual().tipo != TipoComponente::RPAR) {
        reportarCierreEsperado("'" + fn.lexema + "(...)'");
        return 0.0;
    }
    avanzar();

    if (!evaluando_) return 0.0;
    return aplicarFuncion(fn, argumento);
}

const ComponenteLexico& ParserTrigonometrico::actual() const {
    if (indice_ < tokens_.size()) {
        return tokens_[indice_];
    }
    return tokens_.back();
}

const ComponenteLexico& ParserTrigonometrico::avanzar() {
    const ComponenteLexico& corriente = actual();
    if (indice_ + 1 < tokens_.size()) {
        indice_++;
    }
    return corriente;
}

bool ParserTrigonometrico::validarTokensEntrada() {
    if (tokens_.empty() || tokens_[0].tipo == TipoComponente::END_OF_FILE) {
        ComponenteLexico ref = tokens_.empty()
            ? ComponenteLexico(TipoComponente::END_OF_FILE, "EOF", 1, 1)
            : tokens_[0];
        reportarError(FaseError::SINTACTICO, "La expresion esta vacia", ref);
        return false;
    }

    for (const ComponenteLexico& tok : tokens_) {
        if (!EspecificacionTrigonometrica::esTokenPermitido(tok.tipo)) {
            std::string detalle = (tok.tipo == TipoComponente::UNKNOWN)
                ? "Caracter o lexema no reconocido '" + tok.lexema + "'"
                : "Token '" + tok.lexema + "' no permitido en expresiones trigonometricas";
            reportarError(FaseError::LEXICO, detalle, tok);
            return false;
        }
        if ((tok.tipo == TipoComponente::INT_NUM || tok.tipo == TipoComponente::FLOAT_NUM) &&
            !esNumeroDecimalValido(tok.lexema)) {
            reportarError(FaseError::LEXICO,
                          "Numero '" + tok.lexema + "' invalido; solo se permiten constantes reales decimales",
                          tok);
            return false;
        }
    }
    return true;
}

bool ParserTrigonometrico::verificarFinDeEntrada() {
    const ComponenteLexico& tok = actual();
    if (tok.tipo == TipoComponente::END_OF_FILE) {
        return true;
    }

    if (tok.tipo == TipoComponente::RPAR) {
        reportarError(FaseError::SINTACTICO,
                      "Parentesis de cierre ')' sin apertura correspondiente",
                      tok);
    } else if (tok.tipo == TipoComponente::LPAR && indice_ > 0 &&
               tokens_[indice_ - 1].tipo == TipoComponente::ID) {
        reportarError(FaseError::SINTACTICO,
                      "Funcion desconocida '" + tokens_[indice_ - 1].lexema +
                          "' (funciones soportadas: sin/sen, cos, tan)",
                      tokens_[indice_ - 1]);
    } else {
        reportarError(FaseError::SINTACTICO,
                      "Token inesperado '" + tok.lexema + "' (se esperaba un operador o fin de expresion)",
                      tok);
    }
    return false;
}

void ParserTrigonometrico::registrarVariable(const std::string& nombre) {
    if (std::find(variables_.begin(), variables_.end(), nombre) == variables_.end()) {
        variables_.push_back(nombre);
    }
}

double ParserTrigonometrico::convertirNumero(const ComponenteLexico& token) {
    if (!evaluando_) return 0.0;
    try {
        double valor = std::stod(token.lexema);
        return validarFinito(valor, token, "conversion numerica");
    } catch (const std::out_of_range&) {
        reportarError(FaseError::MATEMATICO,
                      "Constante numerica fuera del rango de precision double",
                      token);
        return 0.0;
    }
}

double ParserTrigonometrico::resolverVariable(const ComponenteLexico& token) {
    registrarVariable(token.lexema);
    if (!evaluando_) return 0.0;

    auto it = valores_->find(token.lexema);
    if (it == valores_->end()) {
        reportarError(FaseError::MATEMATICO,
                      "No se proporciono valor para la variable '" + token.lexema + "'",
                      token);
        return 0.0;
    }
    return validarFinito(it->second, token, "sustitucion de variable");
}

double ParserTrigonometrico::aplicarFuncion(const ComponenteLexico& fn, double argumento) {
    double rad = (modo_ == ModoAngulo::GRADOS) ? (argumento * PI / 180.0) : argumento;

    if (fn.tipo == TipoComponente::FN_TAN && modo_ == ModoAngulo::GRADOS) {
        double resto = std::fmod(std::abs(argumento), 180.0);
        if (std::abs(resto - 90.0) < 1e-12) {
            reportarError(FaseError::MATEMATICO,
                          "Indeterminacion matematica: tan(" + std::to_string(argumento) +
                              " grados) no esta definida",
                          fn);
            return 0.0;
        }
    }

    double resultado = 0.0;
    switch (fn.tipo) {
        case TipoComponente::FN_SIN: resultado = std::sin(rad); break;
        case TipoComponente::FN_COS: resultado = std::cos(rad); break;
        case TipoComponente::FN_TAN: resultado = std::tan(rad); break;
        default: break;
    }
    if (modo_ == ModoAngulo::GRADOS && std::fmod(std::abs(argumento), 90.0) == 0.0 &&
        std::abs(resultado) < 1e-12) {
        resultado = 0.0;
    }
    return validarFinito(resultado, fn, "funcion " + fn.lexema);
}

double ParserTrigonometrico::calcularPotencia(
    double base, double exponente, const ComponenteLexico& op) {
    if (base == 0.0 && exponente < 0.0) {
        reportarError(FaseError::MATEMATICO,
                      "Division por cero implicita: 0 elevado a exponente negativo",
                      op);
        return 0.0;
    }
    if (base < 0.0 && std::floor(exponente) != exponente) {
        reportarError(FaseError::MATEMATICO,
                      "Potencia indefinida en los reales: base negativa con exponente no entero",
                      op);
        return 0.0;
    }
    double resultado = std::pow(base, exponente);
    return validarFinito(resultado, op, "potencia");
}

double ParserTrigonometrico::dividir(
    double dividendo, double divisor, const ComponenteLexico& op) {
    if (divisor == 0.0) {
        reportarError(FaseError::MATEMATICO,
                      "Division por cero: el divisor evalua a 0",
                      op);
        return 0.0;
    }
    return validarFinito(dividendo / divisor, op, "division");
}

double ParserTrigonometrico::validarFinito(
    double valor, const ComponenteLexico& token, const std::string& operacion) {
    if (!std::isfinite(valor)) {
        reportarError(FaseError::MATEMATICO,
                      "Resultado indefinido o desbordamiento numerico en " + operacion,
                      token);
        return 0.0;
    }
    return valor;
}

void ParserTrigonometrico::reportarError(
    FaseError fase, const std::string& mensaje, const ComponenteLexico& token) {
    if (error_.has_value()) return;
    error_ = ErrorAnalisis{fase, mensaje, token.lexema, token.linea, token.columna};
}

void ParserTrigonometrico::reportarCierreEsperado(const std::string& descripcionApertura) {
    const ComponenteLexico& tok = actual();
    if (tok.tipo == TipoComponente::LPAR && indice_ > 0 &&
        tokens_[indice_ - 1].tipo == TipoComponente::ID) {
        reportarError(FaseError::SINTACTICO,
                      "Funcion desconocida '" + tokens_[indice_ - 1].lexema +
                          "' (funciones soportadas: sin/sen, cos, tan)",
                      tokens_[indice_ - 1]);
        return;
    }
    reportarError(FaseError::SINTACTICO,
                  "Se esperaba ')' para cerrar " + descripcionApertura +
                      ", se encontro '" + tok.lexema + "'",
                  tok);
}

bool ParserTrigonometrico::esNumeroDecimalValido(const std::string& lexema) {
    if (lexema.empty()) return false;
    if (lexema.size() >= 2 && lexema[0] == '0' && (lexema[1] == 'x' || lexema[1] == 'X')) {
        return false;
    }
    return true;
}
