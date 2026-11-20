#ifndef ENGINE_CHAR_SOURCE_H
#define ENGINE_CHAR_SOURCE_H

#include <string>

// Capa: ENGINE (detalle del AFD, agnostico al lenguaje).
//
// FuenteCaracteres encapsula el cursor sobre el codigo fuente: posicion
// actual, linea y columna, con lookahead de 2 caracteres (verActual y
// verSiguiente). Extraerlo del escaner original elimina el acoplamiento
// entre "como me muevo por el texto" y "que componente reconozco".
//
// Invariantes:
// - linea y columna empiezan en 1.
// - avanzar() sobre '\n' incrementa la linea y reinicia la columna a 1.
// - verActual() y verSiguiente() nunca consumen: retornan '\0' si no hay
//   caracter que mostrar (fin del fuente).
class FuenteCaracteres {
public:
    // Construye el cursor sobre el fuente completo ya cargado en memoria.
    explicit FuenteCaracteres(const std::string& fuente);

    // Mira el caracter actual sin consumirlo ('\0' si llego al fin).
    char verActual() const;

    // Mira el caracter siguiente sin consumirlo ('\0' si no existe).
    char verSiguiente() const;

    // Mira el caracter `offset` posiciones adelante sin consumirlo ('\0' si no existe).
    char verAdelante(size_t offset) const;

    // Consume el caracter actual, actualiza linea/columna y lo retorna.
    // Retorna '\0' si ya se llego al fin (no avanza en ese caso).
    char avanzar();

    // Indica si el cursor ya consumio todo el fuente.
    bool llegoAlFin() const;

    // Linea actual del cursor (base 1).
    int linea() const { return linea_; }

    // Columna actual del cursor (base 1).
    int columna() const { return columna_; }

    // Clasificadores de caracteres (estaticos: no dependen del estado).
    // Retorna true si c es un digito decimal ('0'-'9').
    static bool esDigito(char c);

    // Retorna true si c puede iniciar un identificador (letra o '_').
    static bool esLetra(char c);

    // Retorna true si c puede ir dentro de un identificador.
    static bool esAlfanumerico(char c);

    // Retorna true si c es separador (espacio, tab, salto de linea, etc.).
    static bool esEspacio(char c);

    // Consume todos los espacios consecutivos desde el cursor.
    void saltarEspacios();

private:
    std::string fuente_;  // texto completo del programa a escanear
    size_t      posicion_; // indice del caracter actual en fuente_
    int         linea_;    // linea actual (base 1)
    int         columna_;  // columna actual (base 1)
};

#endif // ENGINE_CHAR_SOURCE_H
