#include "afd_scanner.h"



EscanerAFD::EscanerAFD(const std::string& fuente, const IEspecificacionLenguaje& especificacion)
    : entrada_(fuente), especificacion_(especificacion) {}

std::vector<ComponenteLexico> EscanerAFD::tokenizar() {
    std::vector<ComponenteLexico> componentes;
    while (!entrada_.llegoAlFin()) {
        ComponenteLexico actual = siguienteElemento();
        if (actual.tipo == TipoComponente::END_OF_FILE) {
            componentes.push_back(actual);
            break;
        }
        componentes.push_back(actual);
    }
    if (componentes.empty() || componentes.back().tipo != TipoComponente::END_OF_FILE) {
        componentes.push_back(crearElemento(TipoComponente::END_OF_FILE, "EOF"));
    }
    return componentes;
}

ComponenteLexico EscanerAFD::analizarIdentificador() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;

    while (!entrada_.llegoAlFin() && FuenteCaracteres::esAlfanumerico(entrada_.verActual())) {
        lexema += entrada_.avanzar();
    }

    const auto& tabla = especificacion_.palabrasReservadas();
    auto hallado = tabla.find(lexema);
    TipoComponente tipo = (hallado != tabla.end()) ? hallado->second : TipoComponente::ID;

    return ComponenteLexico(tipo, lexema, lineaInicial, columnaInicial);
}

ComponenteLexico EscanerAFD::analizarNumero() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;

    if (entrada_.verActual() == '0' && (entrada_.verSiguiente() == 'x' || entrada_.verSiguiente() == 'X')) {
        lexema += entrada_.avanzar();
        lexema += entrada_.avanzar();
        size_t digitos = 0;
        while (!entrada_.llegoAlFin() && (FuenteCaracteres::esDigito(entrada_.verActual()) ||
                             (entrada_.verActual() >= 'a' && entrada_.verActual() <= 'f') ||
                             (entrada_.verActual() >= 'A' && entrada_.verActual() <= 'F'))) {
            lexema += entrada_.avanzar();
            digitos++;
        }
        if (digitos == 0) {
            reportarError("Literal hexadecimal sin digitos despues de '0x'", lexema);
        }
        return ComponenteLexico(TipoComponente::INT_NUM, lexema, lineaInicial, columnaInicial);
    }

    bool esFlotante = false;

    if (entrada_.verActual() == '.') {
        esFlotante = true;
        lexema += entrada_.avanzar();
        while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
            lexema += entrada_.avanzar();
        }
    } else {
        while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
            lexema += entrada_.avanzar();
        }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '.' && entrada_.verSiguiente() != '.') {
            esFlotante = true;
            lexema += entrada_.avanzar();
            while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
                lexema += entrada_.avanzar();
            }
        }
    }

    char actual = entrada_.verActual();
    if (actual == 'e' || actual == 'E') {
        char despuesE = entrada_.verSiguiente();
        bool tieneExponenteValido = FuenteCaracteres::esDigito(despuesE) ||
            ((despuesE == '+' || despuesE == '-') && FuenteCaracteres::esDigito(entrada_.verAdelante(2)));

        if (tieneExponenteValido) {
            esFlotante = true;
            lexema += entrada_.avanzar();
            if (entrada_.verActual() == '+' || entrada_.verActual() == '-') {
                lexema += entrada_.avanzar();
            }
            while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
                lexema += entrada_.avanzar();
            }
        }
    }

    TipoComponente tipo = esFlotante ? TipoComponente::FLOAT_NUM : TipoComponente::INT_NUM;
    return ComponenteLexico(tipo, lexema, lineaInicial, columnaInicial);
}

ComponenteLexico EscanerAFD::analizarCadena() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;
    lexema += entrada_.avanzar();

    while (!entrada_.llegoAlFin() && entrada_.verActual() != '"') {
        char c = entrada_.avanzar();
        lexema += c;
        if (c == '\\') {
            if (entrada_.llegoAlFin()) {
                reportarError("Cadena sin cerrar: secuencia de escape incompleta", lexema);
                break;
            }
            char escape = entrada_.avanzar();
            lexema += escape;
            const std::string validos = "n0rt\\vfa\"'";
            if (validos.find(escape) == std::string::npos) {
                reportarError("Secuencia de escape invalida: \\" + std::string(1, escape), lexema);
            }
        }
        if (c == '\n') {
            reportarError("Cadena sin cerrar: salto de linea dentro de la cadena", lexema);
            break;
        }
    }

    if (entrada_.llegoAlFin() && (lexema.empty() || lexema.back() != '"')) {
        reportarError("Cadena sin cerrar al final del archivo", lexema);
    } else if (!entrada_.llegoAlFin() && entrada_.verActual() == '"') {
        lexema += entrada_.avanzar();
    }

    return ComponenteLexico(TipoComponente::STRING_LITERAL, lexema, lineaInicial, columnaInicial);
}

ComponenteLexico EscanerAFD::analizarCaracter() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;
    lexema += entrada_.avanzar();

    if (entrada_.llegoAlFin()) {
        reportarError("Literal de caracter vacio o sin cerrar", lexema);
        return ComponenteLexico(TipoComponente::UNKNOWN, lexema, lineaInicial, columnaInicial);
    }

    char c = entrada_.avanzar();
    lexema += c;

    if (c == '\\') {
        if (entrada_.llegoAlFin()) {
            reportarError("Secuencia de escape incompleta en literal de caracter", lexema);
            return ComponenteLexico(TipoComponente::UNKNOWN, lexema, lineaInicial, columnaInicial);
        }
        char escape = entrada_.avanzar();
        lexema += escape;
        const std::string validos = "n0rt\\vfa\"'";
        if (validos.find(escape) == std::string::npos) {
            reportarError("Secuencia de escape invalida en caracter: \\" + std::string(1, escape), lexema);
        }
    }

    if (entrada_.llegoAlFin() || entrada_.verActual() != '\'') {
        reportarError("Literal de caracter sin cerrar o con multiples caracteres", lexema);
    } else {
        lexema += entrada_.avanzar();
    }

    return ComponenteLexico(TipoComponente::CHAR_LITERAL, lexema, lineaInicial, columnaInicial);
}

void EscanerAFD::omitirComentarioLinea() {
    entrada_.avanzar();
    entrada_.avanzar();
    while (!entrada_.llegoAlFin() && entrada_.verActual() != '\n') {
        entrada_.avanzar();
    }
}

void EscanerAFD::omitirComentarioBloque() {
    int lineaApertura = entrada_.linea();
    int columnaApertura = entrada_.columna();
    entrada_.avanzar();
    entrada_.avanzar();

    while (!entrada_.llegoAlFin()) {
        char c = entrada_.avanzar();
        if (c == '*' && !entrada_.llegoAlFin() && entrada_.verActual() == '/') {
            entrada_.avanzar();
            return;
        }
    }

    reportarError("Comentario de bloque sin cerrar (abierto en linea " + std::to_string(lineaApertura) + ")", "/*");
    errores_.back().linea = lineaApertura;
    errores_.back().columna = columnaApertura;
}

ComponenteLexico EscanerAFD::analizarDirectiva() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    entrada_.avanzar();
    entrada_.saltarEspacios();

    std::string nombre;
    while (!entrada_.llegoAlFin() && FuenteCaracteres::esLetra(entrada_.verActual())) {
        nombre += entrada_.avanzar();
    }

    if (nombre.empty()) {
        reportarError("Directiva de preprocesador vacia despues de '#'", "#");
        errores_.back().linea = lineaInicial;
        errores_.back().columna = columnaInicial;
        return ComponenteLexico(TipoComponente::UNKNOWN, "#", lineaInicial, columnaInicial);
    }

    const auto& tabla = especificacion_.directivas();
    auto hallada = tabla.find(nombre);
    if (hallada == tabla.end()) {
        reportarError("Directiva de preprocesador desconocida: #" + nombre, "#" + nombre);
        return ComponenteLexico(TipoComponente::UNKNOWN, "#" + nombre, lineaInicial, columnaInicial);
    }
    return ComponenteLexico(hallada->second, "#" + nombre, lineaInicial, columnaInicial);
}

ComponenteLexico EscanerAFD::analizarOperadorOPuntuacion() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    char c = entrada_.avanzar();

    switch (c) {
    case '{': return ComponenteLexico(TipoComponente::LBRACE,  "{", lineaInicial, columnaInicial);
    case '}': return ComponenteLexico(TipoComponente::RBRACE,  "}", lineaInicial, columnaInicial);
    case '[': return ComponenteLexico(TipoComponente::LSQUARE, "[", lineaInicial, columnaInicial);
    case ']': return ComponenteLexico(TipoComponente::RSQUARE, "]", lineaInicial, columnaInicial);
    case '(': return ComponenteLexico(TipoComponente::LPAR,    "(", lineaInicial, columnaInicial);
    case ')': return ComponenteLexico(TipoComponente::RPAR,    ")", lineaInicial, columnaInicial);
    case ';': return ComponenteLexico(TipoComponente::SEMI,    ";", lineaInicial, columnaInicial);
    case ',': return ComponenteLexico(TipoComponente::COMMA,   ",", lineaInicial, columnaInicial);
    case ':': return ComponenteLexico(TipoComponente::COLON,   ":", lineaInicial, columnaInicial);
    case '~': return ComponenteLexico(TipoComponente::TILDE_OP,"~", lineaInicial, columnaInicial);
    case '^': return ComponenteLexico(TipoComponente::XOR_OP,  "^", lineaInicial, columnaInicial);

    case '+':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '+') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::INC_OP,      "++", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::PLUS_ASSIGN,  "+=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::PLUS, "+", lineaInicial, columnaInicial);

    case '-':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '-') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::DEC_OP,       "--", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MINUS_ASSIGN,  "-=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '>') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::ARROW,         "->", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MINUS, "-", lineaInicial, columnaInicial);

    case '*':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MUL_ASSIGN, "*=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MUL_OP, "*", lineaInicial, columnaInicial);

    case '/':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::DIV_ASSIGN, "/=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::DIV_OP, "/", lineaInicial, columnaInicial);

    case '%':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MOD_ASSIGN, "%=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MOD_OP, "%", lineaInicial, columnaInicial);

    case '=':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::EQ,     "==", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::ASSIGN, "=", lineaInicial, columnaInicial);

    case '!':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::NOTEQ,  "!=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::NOT_OP, "!", lineaInicial, columnaInicial);

    case '<':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::LTEQ,   "<=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '<') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::SHL_OP, "<<", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::LT, "<", lineaInicial, columnaInicial);

    case '>':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::GTEQ,   ">=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '>') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::SHR_OP, ">>", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::GT, ">", lineaInicial, columnaInicial);

    case '&':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '&') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::ANDAND, "&&", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::AND_OP, "&", lineaInicial, columnaInicial);

    case '|':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '|') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::OROR,   "||", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::OR_OP, "|", lineaInicial, columnaInicial);

    case '.':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '.' && entrada_.verSiguiente() == '.') {
            entrada_.avanzar(); entrada_.avanzar();
            return ComponenteLexico(TipoComponente::ELLIPSIS, "...", lineaInicial, columnaInicial);
        }
        return ComponenteLexico(TipoComponente::DOT, ".", lineaInicial, columnaInicial);

    default:
        reportarError("Caracter no reconocido: '" + std::string(1, c) + "'", std::string(1, c));
        return ComponenteLexico(TipoComponente::UNKNOWN, std::string(1, c), lineaInicial, columnaInicial);
    }
}

ComponenteLexico EscanerAFD::siguienteElemento() {
    while (true) {
        entrada_.saltarEspacios();

        if (entrada_.llegoAlFin()) {
            return crearElemento(TipoComponente::END_OF_FILE, "EOF");
        }

        char c = entrada_.verActual();

        if (FuenteCaracteres::esLetra(c)) {
            return analizarIdentificador();
        }

        if (FuenteCaracteres::esDigito(c) ||
            (c == '.' && FuenteCaracteres::esDigito(entrada_.verSiguiente()))) {
            return analizarNumero();
        }

        if (c == '"') {
            return analizarCadena();
        }

        if (c == '\'') {
            return analizarCaracter();
        }

        if (c == '#') {
            return analizarDirectiva();
        }

        if (c == '/' && entrada_.verSiguiente() == '/') {
            omitirComentarioLinea();
            continue;
        }
        if (c == '/' && entrada_.verSiguiente() == '*') {
            size_t tamAntes = errores_.size();
            omitirComentarioBloque();
            if (errores_.size() == tamAntes) {
                continue;
            }
            const ErrorLexico& ultimo = errores_.back();
            return ComponenteLexico(TipoComponente::UNKNOWN, "/*", ultimo.linea, ultimo.columna);
        }

        return analizarOperadorOPuntuacion();
    }
}

void EscanerAFD::reportarError(const std::string& mensaje, const std::string& contexto) {
    errores_.push_back({mensaje, contexto, entrada_.linea(), entrada_.columna()});
}

ComponenteLexico EscanerAFD::crearElemento(TipoComponente tipo, const std::string& lexema) {
    return ComponenteLexico(tipo, lexema, entrada_.linea(), entrada_.columna());
}
