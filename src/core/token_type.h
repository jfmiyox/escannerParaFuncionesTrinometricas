#ifndef CORE_TOKEN_TYPE_H
#define CORE_TOKEN_TYPE_H

#include <string>

// Capa: CORE (dominio).
//
// TipoComponente es el vocabulario de categorias que el motor AFD puede
// emitir. Es intencionalmente un "superconjunto" generico: cada lenguaje
// (C, expresiones trigonometricas, etc.) usa el subconjunto que necesita
// a traves de su especificacion (palabras reservadas, directivas y nombres).
// El motor nunca asocia un lenguaje concreto a estas categorias.
enum class TipoComponente {
    // Palabras reservadas de C.
    INT, MAIN, VOID, BREAK, DO, ELSE, IF, WHILE, RETURN,
    READ,           // scanf
    WRITE,          // printf
    AUTO, DOUBLE, LONG, STRUCT, SWITCH, CASE, ENUM,
    REGISTER, TYPEDEF, CHAR_KW, EXTERN, UNION, CONST, FLOAT,
    SHORT, UNSIGNED, CONTINUE, FOR, SIGNED, DEFAULT, GOTO,
    SIZEOF, VOLATILE, STATIC,

    // Funciones trigonometricas (lenguaje de expresiones trigonometricas).
    FN_SIN,         // sin / sen
    FN_COS,         // cos
    FN_TAN,         // tan

    // Directivas de preprocesador.
    DEFINE, ELIF, ENDIF, ERROR_DIR, IFDEF, IFNDEF,
    INCLUDE, MESSAGE, UNDEF,

    // Identificadores y literales.
    ID,
    INT_NUM,
    FLOAT_NUM,
    CHAR_LITERAL,
    STRING_LITERAL,

    // Operadores aritmeticos.
    PLUS,           // +
    MINUS,          // -
    MUL_OP,         // *
    DIV_OP,         // /
    MOD_OP,         // %

    // Operadores de asignacion.
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    MUL_ASSIGN,     // *=
    DIV_ASSIGN,     // /=
    MOD_ASSIGN,     // %=

    // Operadores relacionales.
    EQ,             // ==
    NOTEQ,          // !=
    LT,             // <
    GT,             // >
    LTEQ,           // <=
    GTEQ,           // >=

    // Operadores logicos.
    ANDAND,         // &&
    OROR,           // ||
    NOT_OP,         // !

    // Operadores de bits / potencia.
    AND_OP,         // &
    OR_OP,          // |
    XOR_OP,         // ^ (potencia en expresiones matematicas)
    TILDE_OP,       // ~
    SHL_OP,         // <<
    SHR_OP,         // >>

    // Operadores de puntero y miembro.
    DOT,            // .
    ARROW,          // ->

    // Agrupacion y puntuacion.
    LBRACE,         // {
    RBRACE,         // }
    LSQUARE,        // [
    RSQUARE,        // ]
    LPAR,           // (
    RPAR,           // )
    SEMI,           // ;
    COMMA,          // ,
    COLON,          // :

    // Incremento y decremento.
    INC_OP,         // ++
    DEC_OP,         // --

    // Especiales.
    HASH,           // # (inicio de directiva)
    ELLIPSIS,       // ...

    // Fin de archivo / fin de expresion.
    END_OF_FILE,

    // Error (lexema no reconocido por ninguna regla).
    UNKNOWN
};

#endif // CORE_TOKEN_TYPE_H
