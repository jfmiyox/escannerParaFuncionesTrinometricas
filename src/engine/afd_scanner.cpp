#include "afd_scanner.h"



EscanerAFD::EscanerAFD(const std::string& fuente, const IEspecificacionLenguaje& especificacion)
    : entrada_(fuente), especificacion_(especificacion) {}

// Recorre el fuente pidiendo componentes hasta agotar la entrada.
// Garantiza que la lista termine en END_OF_FILE aunque el fuente este
// vacio o el ultimo estado no lo haya emitido.
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

// Consume la secuencia alfanumerica completa y clasifica: si la palabra
// figura en la tabla del lenguaje es reservada, si no es identificador.
// La posicion guardada es la del primer caracter del lexema.
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

// Reconoce constantes numericas reales:
// 1) Hexadecimal: "0x" o "0X" seguido de [0-9a-fA-F]+ -> INT_NUM (filtrado luego en parser trig).
// 2) Flotante / real: digitos ("." digitos?)?, o "." digitos, con exponente cientifico opcional [eE][+-]?digitos.
// 3) Entero decimal: digitos -> INT_NUM.
ComponenteLexico EscanerAFD::analizarNumero() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;

    // Rama hexadecimal: prefijo 0x.
    if (entrada_.verActual() == '0' && (entrada_.verSiguiente() == 'x' || entrada_.verSiguiente() == 'X')) {
        lexema += entrada_.avanzar(); // '0'
        lexema += entrada_.avanzar(); // 'x' o 'X'
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

    // Mantisa: inicia con '.' seguido de digito (ej. ".5") o parte entera con punto opcional (ej. "3.14", "3.").
    if (entrada_.verActual() == '.') {
        esFlotante = true;
        lexema += entrada_.avanzar(); // '.'
        while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
            lexema += entrada_.avanzar();
        }
    } else {
        while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
            lexema += entrada_.avanzar();
        }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '.' && entrada_.verSiguiente() != '.') {
            esFlotante = true;
            lexema += entrada_.avanzar(); // '.'
            while (!entrada_.llegoAlFin() && FuenteCaracteres::esDigito(entrada_.verActual())) {
                lexema += entrada_.avanzar();
            }
        }
    }

    // Exponente cientifico opcional: [eE][+-]?[0-9]+
    char actual = entrada_.verActual();
    if (actual == 'e' || actual == 'E') {
        char despuesE = entrada_.verSiguiente();
        bool tieneExponenteValido = FuenteCaracteres::esDigito(despuesE) ||
            ((despuesE == '+' || despuesE == '-') && FuenteCaracteres::esDigito(entrada_.verAdelante(2)));

        if (tieneExponenteValido) {
            esFlotante = true;
            lexema += entrada_.avanzar(); // 'e' o 'E'
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

// Consume desde la comilla de apertura hasta la de cierre. Dentro admite
// secuencias de escape con '\' (validas: n 0 r t \ v f a " ').
// Casos de error con recovery:
// - Escape invalido: se reporta pero el analisis continua.
// - Salto de linea antes del cierre: se reporta y se corta la cadena.
// - Fin de archivo antes del cierre: se reporta al salir del bucle.
ComponenteLexico EscanerAFD::analizarCadena() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;
    lexema += entrada_.avanzar(); // comilla de apertura '"'

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
        lexema += entrada_.avanzar(); // comilla de cierre '"'
    }

    return ComponenteLexico(TipoComponente::STRING_LITERAL, lexema, lineaInicial, columnaInicial);
}

// Consume '\'' contenido '\''. El contenido es un caracter simple o una
// secuencia de escape ('\n', '\t', ...). Si falta el cierre o hay mas de
// un caracter, se reporta el error; aun asi se retorna el componente
// parcial para que el escaneo pueda continuar (recovery).
ComponenteLexico EscanerAFD::analizarCaracter() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    std::string lexema;
    lexema += entrada_.avanzar(); // comilla simple de apertura

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
        lexema += entrada_.avanzar(); // comilla simple de cierre
    }

    return ComponenteLexico(TipoComponente::CHAR_LITERAL, lexema, lineaInicial, columnaInicial);
}

// Se invoca con el cursor sobre el primer '/'. Consume ambos ('//')
// y descarta todo hasta el salto de linea (sin incluirlo) o hasta el
// fin del archivo. Solo mueve el cursor: el siguiente componente lo
// pedira el bucle de siguienteElemento, sin recursion.
void EscanerAFD::omitirComentarioLinea() {
    entrada_.avanzar(); // consume el primer '/'
    entrada_.avanzar(); // consume el segundo '/'
    while (!entrada_.llegoAlFin() && entrada_.verActual() != '\n') {
        entrada_.avanzar();
    }
}

// Se invoca con el cursor sobre '/' (de "/*"). Consume la apertura y
// descarta hasta el cierre "*/". Si el archivo termina antes, reporta
// "bloque sin cerrar" ubicado en la posicion de apertura y deja que
// siguienteElemento emita el UNKNOWN de evidencia.
// Solo mueve el cursor (sin recursion).
void EscanerAFD::omitirComentarioBloque() {
    int lineaApertura = entrada_.linea();
    int columnaApertura = entrada_.columna();
    entrada_.avanzar(); // consume '/'
    entrada_.avanzar(); // consume '*'

    while (!entrada_.llegoAlFin()) {
        char c = entrada_.avanzar();
        if (c == '*' && !entrada_.llegoAlFin() && entrada_.verActual() == '/') {
            entrada_.avanzar(); // consume '/'
            return;
        }
    }

    reportarError("Comentario de bloque sin cerrar (abierto en linea " + std::to_string(lineaApertura) + ")", "/*");
    // Reubica el error en la apertura real: al llegar aqui el cursor
    // esta en el fin del archivo, pero al usuario le sirve saber donde
    // abrio el comentario que nunca se cerro.
    errores_.back().linea = lineaApertura;
    errores_.back().columna = columnaApertura;
}

// Consume '#', omite espacios intermedios y lee el nombre con letras.
// Si el nombre figura en la tabla del lenguaje retorna su categoria;
// si no, reporta "directiva desconocida" y retorna UNKNOWN.
// Un '#' sin nombre (fin de linea o fin de archivo) tambien es error.
ComponenteLexico EscanerAFD::analizarDirectiva() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    entrada_.avanzar(); // consume '#'
    entrada_.saltarEspacios();

    std::string nombre;
    while (!entrada_.llegoAlFin() && FuenteCaracteres::esLetra(entrada_.verActual())) {
        nombre += entrada_.avanzar();
    }

    // Caso borde: '#' seguido de fin de linea, de fin de archivo o de un
    // caracter no alfabetico (ej. "#123"). No hay nombre de directiva.
    // El error se ubica en el '#' inicial, no donde quedo el cursor tras
    // omitir los espacios (que pudo cruzar saltos de linea).
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

// Despacha por el primer caracter y, cuando aplica, confirma el segundo
// (y tercero para "...") con lookahead. Cubre agrupacion, aritmetica,
// asignacion compuesta, relacionales, logicos, bits, miembro, incremento,
// decremento y elipsis. El caso por defecto reporta el caracter
// no reconocido como UNKNOWN y el escaneo continua.
ComponenteLexico EscanerAFD::analizarOperadorOPuntuacion() {
    int lineaInicial = entrada_.linea(), columnaInicial = entrada_.columna();
    char c = entrada_.avanzar();

    switch (c) {
    // Agrupacion y puntuacion simple.
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

    // Signo mas: incremento (++), asignacion compuesta (+=) o suma (+).
    case '+':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '+') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::INC_OP,      "++", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::PLUS_ASSIGN,  "+=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::PLUS, "+", lineaInicial, columnaInicial);

    // Signo menos: decremento (--), compuesta (-=), flecha (->) o resta (-).
    case '-':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '-') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::DEC_OP,       "--", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MINUS_ASSIGN,  "-=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '>') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::ARROW,         "->", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MINUS, "-", lineaInicial, columnaInicial);

    // Asterisco: multiplicacion compuesta (*=) o producto/desreferencia (*).
    case '*':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MUL_ASSIGN, "*=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MUL_OP, "*", lineaInicial, columnaInicial);

    // Barra: division compuesta (/=) o division (/).
    // NOTA: los comentarios (// y /*) nunca llegan aqui porque el bucle
    // de siguienteElemento los intercepta y omite antes del reparto.
    case '/':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::DIV_ASSIGN, "/=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::DIV_OP, "/", lineaInicial, columnaInicial);

    // Porcentaje: modulo compuesto (%=) o modulo (%).
    case '%':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::MOD_ASSIGN, "%=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::MOD_OP, "%", lineaInicial, columnaInicial);

    // Igual: comparacion (==) o asignacion (=).
    case '=':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::EQ,     "==", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::ASSIGN, "=", lineaInicial, columnaInicial);

    // Exclamacion: distinto (!=) o negacion logica (!).
    case '!':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::NOTEQ,  "!=", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::NOT_OP, "!", lineaInicial, columnaInicial);

    // Menor: menor-igual (<=), desplazamiento (<<) o menor (<).
    case '<':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::LTEQ,   "<=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '<') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::SHL_OP, "<<", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::LT, "<", lineaInicial, columnaInicial);

    // Mayor: mayor-igual (>=), desplazamiento (>>) o mayor (>).
    case '>':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '=') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::GTEQ,   ">=", lineaInicial, columnaInicial); }
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '>') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::SHR_OP, ">>", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::GT, ">", lineaInicial, columnaInicial);

    // Ampersand: AND logico (&&) o AND de bits / direccion (&).
    case '&':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '&') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::ANDAND, "&&", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::AND_OP, "&", lineaInicial, columnaInicial);

    // Pleca: OR logico (||) u OR de bits (|).
    case '|':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '|') { entrada_.avanzar(); return ComponenteLexico(TipoComponente::OROR,   "||", lineaInicial, columnaInicial); }
        return ComponenteLexico(TipoComponente::OR_OP, "|", lineaInicial, columnaInicial);

    // Punto: elipsis (...) si hay dos puntos mas, o acceso a miembro (.).
    case '.':
        if (!entrada_.llegoAlFin() && entrada_.verActual() == '.' && entrada_.verSiguiente() == '.') {
            // Ya se consumio el primer punto; faltan los otros dos.
            entrada_.avanzar(); entrada_.avanzar();
            return ComponenteLexico(TipoComponente::ELLIPSIS, "...", lineaInicial, columnaInicial);
        }
        return ComponenteLexico(TipoComponente::DOT, ".", lineaInicial, columnaInicial);

    // Sin regla asociada: error lexico con recovery.
    default:
        reportarError("Caracter no reconocido: '" + std::string(1, c) + "'", std::string(1, c));
        return ComponenteLexico(TipoComponente::UNKNOWN, std::string(1, c), lineaInicial, columnaInicial);
    }
}

// Reparte segun el primer caracter no blanco hacia el estado que toca.
// Los comentarios se omiten dentro del mismo bucle, por lo que una
// seguidilla larga de comentarios no consume pila de llamadas.
ComponenteLexico EscanerAFD::siguienteElemento() {
    while (true) {
        entrada_.saltarEspacios();

        if (entrada_.llegoAlFin()) {
            return crearElemento(TipoComponente::END_OF_FILE, "EOF");
        }

        char c = entrada_.verActual();

        // ESTADO: identificador o palabra reservada.
        if (FuenteCaracteres::esLetra(c)) {
            return analizarIdentificador();
        }

        // ESTADO: numero entero, real decimal, .fraccion o notacion cientifica.
        if (FuenteCaracteres::esDigito(c) ||
            (c == '.' && FuenteCaracteres::esDigito(entrada_.verSiguiente()))) {
            return analizarNumero();
        }

        // ESTADO: literal de cadena.
        if (c == '"') {
            return analizarCadena();
        }

        // ESTADO: literal de caracter.
        if (c == '\'') {
            return analizarCaracter();
        }

        // ESTADO: directiva de preprocesador.
        if (c == '#') {
            return analizarDirectiva();
        }

        // Comentarios: se omiten y el bucle pide el siguiente real.
        if (c == '/' && entrada_.verSiguiente() == '/') {
            omitirComentarioLinea();
            continue;
        }
        if (c == '/' && entrada_.verSiguiente() == '*') {
            // Guarda la posicion de apertura para el posible error de
            // bloque sin cerrar (omitirComentarioBloque la reporta).
            size_t tamAntes = errores_.size();
            omitirComentarioBloque();
            // Si el bloque cerro bien, seguir buscando el componente.
            // Si no cerro, emitir el UNKNOWN de evidencia y continuar.
            if (errores_.size() == tamAntes) {
                continue;
            }
            // Recupera la posicion de apertura desde el error recien
            // registrado para ubicar el componente UNKNOWN.
            const ErrorLexico& ultimo = errores_.back();
            return ComponenteLexico(TipoComponente::UNKNOWN, "/*", ultimo.linea, ultimo.columna);
        }

        // OPERADORES Y PUNTUACION: cualquier otro caso.
        return analizarOperadorOPuntuacion();
    }
}

// Guarda el error con la linea y columna actuales del cursor, que en el
// momento de la llamada corresponden al punto de deteccion.
void EscanerAFD::reportarError(const std::string& mensaje, const std::string& contexto) {
    errores_.push_back({mensaje, contexto, entrada_.linea(), entrada_.columna()});
}

// Fabrica un componente en la posicion actual del cursor.
// Solo se usa para END_OF_FILE, cuyo inicio coincide con la deteccion.
ComponenteLexico EscanerAFD::crearElemento(TipoComponente tipo, const std::string& lexema) {
    return ComponenteLexico(tipo, lexema, entrada_.linea(), entrada_.columna());
}
