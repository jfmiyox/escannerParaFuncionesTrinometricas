#include "char_source.h"

// Inicializa el cursor al inicio del fuente: posicion 0, linea 1, columna 1.
FuenteCaracteres::FuenteCaracteres(const std::string& fuente)
    : fuente_(fuente), posicion_(0), linea_(1), columna_(1) {}

// Retorna el caracter bajo el cursor sin moverlo.
char FuenteCaracteres::verActual() const {
    if (llegoAlFin()) return '\0';
    return fuente_[posicion_];
}

// Retorna el caracter posterior al actual sin mover el cursor.
// Permite decidir operadores dobles (++, ==, ...) con un solo vistazo.
char FuenteCaracteres::verSiguiente() const {
    return verAdelante(1);
}

char FuenteCaracteres::verAdelante(size_t offset) const {
    if (posicion_ + offset >= fuente_.size()) return '\0';
    return fuente_[posicion_ + offset];
}

// Consume un caracter y actualiza la posicion logica. El salto de linea
// cuenta como fin de linea: sube la linea y reinicia la columna.
char FuenteCaracteres::avanzar() {
    if (llegoAlFin()) return '\0';
    char c = fuente_[posicion_++];
    if (c == '\n') {
        linea_++;
        columna_ = 1;
    } else {
        columna_++;
    }
    return c;
}

// El fin se alcanza cuando la posicion supera el ultimo indice valido.
bool FuenteCaracteres::llegoAlFin() const {
    return posicion_ >= fuente_.size();
}

// Solo digitos decimales ASCII. Los hexadecimales (a-f) se validan aparte
// en el analizador de numeros, donde corresponde.
bool FuenteCaracteres::esDigito(char c) {
    return c >= '0' && c <= '9';
}

// Letras ASCII mas el guion bajo, que en C inicia identificadores.
bool FuenteCaracteres::esLetra(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Un identificador admite letras y digitos despues del primer caracter.
bool FuenteCaracteres::esAlfanumerico(char c) {
    return esLetra(c) || esDigito(c);
}

// Los seis separadores reconocidos. '\r' se incluye para tolerar
// archivos con finales de linea Windows (\r\n).
bool FuenteCaracteres::esEspacio(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

// Avanza el cursor mientras haya separadores. Los comentarios NO se
// omiten aqui: los consume el escaner como estados propios del AFD.
void FuenteCaracteres::saltarEspacios() {
    while (!llegoAlFin()) {
        char c = verActual();
        if (esEspacio(c)) {
            avanzar();
        } else {
            break;
        }
    }
}
