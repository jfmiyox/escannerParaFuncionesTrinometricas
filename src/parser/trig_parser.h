#ifndef PARSER_TRIG_PARSER_H
#define PARSER_TRIG_PARSER_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../core/token.h"
#include "parse_error.h"

enum class ModoAngulo {
    RADIANES,
    GRADOS
};

struct ResultadoSintaxis {
    bool exito = false;
    std::vector<std::string> variables;
    std::optional<ErrorAnalisis> error;
};

struct ResultadoEvaluacion {
    bool exito = false;
    double valor = 0.0;
    std::optional<ErrorAnalisis> error;
};

class ParserTrigonometrico {
public:
    explicit ParserTrigonometrico(const std::vector<ComponenteLexico>& tokens);

    ResultadoSintaxis validarYDetectarVariables();

    ResultadoEvaluacion evaluar(
        const std::unordered_map<std::string, double>& valoresVariables,
        ModoAngulo modo = ModoAngulo::RADIANES);

private:
    double parseE();
    double parseEPrima(double acumulado);
    double parseT();
    double parseTPrima(double acumulado);
    double parseU();
    double parseP();
    double parsePPrima(double base);
    double parseF();
    double parseParentesis();
    double parseFuncion();

    const ComponenteLexico& actual() const;
    const ComponenteLexico& avanzar();
    bool validarTokensEntrada();
    bool verificarFinDeEntrada();
    void registrarVariable(const std::string& nombre);

    double convertirNumero(const ComponenteLexico& token);
    double resolverVariable(const ComponenteLexico& token);
    double aplicarFuncion(const ComponenteLexico& fn, double argumento);
    double calcularPotencia(double base, double exponente, const ComponenteLexico& op);
    double dividir(double dividendo, double divisor, const ComponenteLexico& op);
    double validarFinito(double valor, const ComponenteLexico& token, const std::string& operacion);

    bool tieneError() const { return error_.has_value(); }
    void reportarError(FaseError fase, const std::string& mensaje, const ComponenteLexico& token);
    void reportarCierreEsperado(const std::string& descripcionApertura);
    static bool esNumeroDecimalValido(const std::string& lexema);

    const std::vector<ComponenteLexico>& tokens_;
    size_t indice_ = 0;
    int profundidad_ = 0;
    bool evaluando_ = false;
    ModoAngulo modo_ = ModoAngulo::RADIANES;
    const std::unordered_map<std::string, double>* valores_ = nullptr;
    std::vector<std::string> variables_;
    std::optional<ErrorAnalisis> error_;
};

#endif
