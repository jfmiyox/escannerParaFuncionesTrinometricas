#ifndef PARSER_TRIG_PARSER_H
#define PARSER_TRIG_PARSER_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../core/token.h"
#include "parse_error.h"

// Unidad angular aplicada a los argumentos de sin/sen, cos y tan.
enum class ModoAngulo {
    RADIANES,
    GRADOS
};

// Resultado de la fase de validacion sintactica LL(1) y deteccion de variables.
struct ResultadoSintaxis {
    bool exito = false;
    std::vector<std::string> variables; // Variables unicas en orden de aparicion.
    std::optional<ErrorAnalisis> error;
};

// Resultado de la fase de evaluacion semantica con precision double.
struct ResultadoEvaluacion {
    bool exito = false;
    double valor = 0.0;
    std::optional<ErrorAnalisis> error;
};

// Capa: PARSER (Reconocedor Descendente Recursivo LL(1) y Evaluador Directo).
//
// Gramatica LL(1) implementada:
//   E  -> T E'
//   E' -> + T E' | - T E' | epsilon
//   T  -> U T'
//   T' -> * U T' | / U T' | epsilon
//   U  -> + U | - U | P
//   P  -> F P'
//   P' -> ^ U | epsilon
//   F  -> ( E ) | FUNC ( E ) | NUM | VAR
//
// Precedencia resultante (de mayor a menor):
//   1. Agrupacion () y funciones (sin/sen, cos, tan)
//   2. Potencia ^ (asociativa por la derecha)
//   3. Signo unario + y -
//   4. Multiplicacion * y division / (asociativas por la izquierda)
//   5. Suma + y resta - (asociativas por la izquierda)
class ParserTrigonometrico {
public:
    explicit ParserTrigonometrico(const std::vector<ComponenteLexico>& tokens);

    // Valida el vocabulario de tokens y la gramatica LL(1) sin evaluar
    // aritmetica, recolectando las variables simbolicas detectadas.
    ResultadoSintaxis validarYDetectarVariables();

    // Evalua la expresion durante el descenso recursivo sustituyendo cada
    // variable simbolica por su valor en `valoresVariables`.
    ResultadoEvaluacion evaluar(
        const std::unordered_map<std::string, double>& valoresVariables,
        ModoAngulo modo = ModoAngulo::RADIANES);

private:
    // Reglas no terminales LL(1).
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

    // Utilidades del cursor y validaciones.
    const ComponenteLexico& actual() const;
    const ComponenteLexico& avanzar();
    bool validarTokensEntrada();
    bool verificarFinDeEntrada();
    void registrarVariable(const std::string& nombre);

    // Operaciones semanticas con validacion de dominio matematico.
    double convertirNumero(const ComponenteLexico& token);
    double resolverVariable(const ComponenteLexico& token);
    double aplicarFuncion(const ComponenteLexico& fn, double argumento);
    double calcularPotencia(double base, double exponente, const ComponenteLexico& op);
    double dividir(double dividendo, double divisor, const ComponenteLexico& op);
    double validarFinito(double valor, const ComponenteLexico& token, const std::string& operacion);

    // Manejo de errores.
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

#endif // PARSER_TRIG_PARSER_H
