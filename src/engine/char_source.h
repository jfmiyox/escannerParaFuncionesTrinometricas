#ifndef ENGINE_CHAR_SOURCE_H
#define ENGINE_CHAR_SOURCE_H

#include <string>

class FuenteCaracteres {
public:
    explicit FuenteCaracteres(const std::string& fuente);

    char verActual() const;

    char verSiguiente() const;

    char verAdelante(size_t offset) const;

    char avanzar();

    bool llegoAlFin() const;

    int linea() const { return linea_; }

    int columna() const { return columna_; }

    static bool esDigito(char c);

    static bool esLetra(char c);

    static bool esAlfanumerico(char c);

    static bool esEspacio(char c);

    void saltarEspacios();

private:
    std::string fuente_;
    size_t      posicion_;
    int         linea_;
    int         columna_;
};

#endif
