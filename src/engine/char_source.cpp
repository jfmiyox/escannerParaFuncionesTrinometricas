#include "char_source.h"

FuenteCaracteres::FuenteCaracteres(const std::string& fuente)
    : fuente_(fuente), posicion_(0), linea_(1), columna_(1) {}

char FuenteCaracteres::verActual() const {
    if (llegoAlFin()) return '\0';
    return fuente_[posicion_];
}

char FuenteCaracteres::verSiguiente() const {
    return verAdelante(1);
}

char FuenteCaracteres::verAdelante(size_t offset) const {
    if (posicion_ + offset >= fuente_.size()) return '\0';
    return fuente_[posicion_ + offset];
}

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

bool FuenteCaracteres::llegoAlFin() const {
    return posicion_ >= fuente_.size();
}

bool FuenteCaracteres::esDigito(char c) {
    return c >= '0' && c <= '9';
}

bool FuenteCaracteres::esLetra(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool FuenteCaracteres::esAlfanumerico(char c) {
    return esLetra(c) || esDigito(c);
}

bool FuenteCaracteres::esEspacio(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';  // \r por si el archivo viene de windows
}

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
