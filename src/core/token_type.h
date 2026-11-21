#ifndef CORE_TOKEN_TYPE_H
#define CORE_TOKEN_TYPE_H

#include <string>

enum class TipoComponente {
    INT, MAIN, VOID, BREAK, DO, ELSE, IF, WHILE, RETURN,
    READ,
    WRITE,
    AUTO, DOUBLE, LONG, STRUCT, SWITCH, CASE, ENUM,
    REGISTER, TYPEDEF, CHAR_KW, EXTERN, UNION, CONST, FLOAT,
    SHORT, UNSIGNED, CONTINUE, FOR, SIGNED, DEFAULT, GOTO,
    SIZEOF, VOLATILE, STATIC,

    FN_SIN,
    FN_COS,
    FN_TAN,

    DEFINE, ELIF, ENDIF, ERROR_DIR, IFDEF, IFNDEF,
    INCLUDE, MESSAGE, UNDEF,

    ID,
    INT_NUM,
    FLOAT_NUM,
    CHAR_LITERAL,
    STRING_LITERAL,

    PLUS,
    MINUS,
    MUL_OP,
    DIV_OP,
    MOD_OP,

    ASSIGN,
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,

    EQ,
    NOTEQ,
    LT,
    GT,
    LTEQ,
    GTEQ,

    ANDAND,
    OROR,
    NOT_OP,

    AND_OP,
    OR_OP,
    XOR_OP,  // ^ es potencia aqui, no xor
    TILDE_OP,
    SHL_OP,
    SHR_OP,

    DOT,
    ARROW,

    LBRACE,
    RBRACE,
    LSQUARE,
    RSQUARE,
    LPAR,
    RPAR,
    SEMI,
    COMMA,
    COLON,

    INC_OP,
    DEC_OP,

    HASH,
    ELLIPSIS,

    END_OF_FILE,

    UNKNOWN
};

#endif
