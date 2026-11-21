# Evaluador de Expresiones Trigonométricas LL(1) en C++17

Implementación del **Trabajo Práctico 3 de Compiladores**: diseño de una gramática LL(1), analizador sintáctico descendente recursivo y evaluador semántico directo para expresiones aritméticas y trigonométricas con precisión `double`, reutilizando el motor léxico AFD de `scanner_c`.

## Características

- **Reutilización del analizador léxico (`scanner_c`)**: se conserva íntegro el motor `EscanerAFD` (`src/engine/`) e interfaces `core/` y `application/`, inyectando únicamente el nuevo plugin `EspecificacionTrigonometrica` (`src/languages/trig/`).
- **Gramática LL(1) estricta**:
  - Precedencia matemática estándar: `()` y `FUNC(...)` > `^` (asociativa a derecha) > unario `+`/`-` > `*`, `/` > `+`, `-`.
  - Funciones soportadas (incluso anidadas, ej. `sen(cos(tan(0.5)))`): `sin` (con alias `sen`), `cos`, `tan`, con selector opcional de **Radianes** (por defecto) o **Grados** (`-g` / `--grados`, o `rad`/`grados` en el modo interactivo) sin alterar la gramática LL(1).
  - Operadores: `+`, `-`, `*`, `/`, `^`.
  - Constantes numéricas reales en todos sus formatos: enteros (`42`), decimales (`3.14`, `.5`, `3.`) y notación científica (`1.5e-3`, `2E3`), además de variables simbólicas (`x`, `y`, `z`, etc.).
- **Evaluación semántica directa durante el descenso recursivo**: solicita automáticamente al usuario el valor numérico de cada variable simbólica detectada (en orden de aparición, sin duplicados) y evalúa en precisión `double`.
- **Validaciones completas**:
  - Léxicas: caracteres no reconocidos, tokens de C no permitidos (`;`, `=`, etc.), literales hexadecimales (`0x...`), puntos aislados (`.`, `3..1`).
  - Sintácticas LL(1): expresión vacía, paréntesis desbalanceados o vacíos, funciones desconocidas (`log(x)`), función sin `(`, números mal formados (`3445ea`, `1.5e+`, `1.2.3`), operadores faltantes o consecutivos, límite de profundidad de recursión.
  - De entrada de variables: valida reales en formato entero, decimal o notación científica al leer del usuario.
  - Matemáticas: división por cero, `0` elevado a exponente negativo, base negativa con exponente fraccionario, desbordamiento (`1e400`) o resultado no finito (`NaN`/`Inf`).

## Compilación y Pruebas

El repositorio incluye el binario `./evaluador` compilado de forma **estática** para Linux x86_64 / WSL (funciona en distintas versiones de distribución sin depender de `libstdc++` o `glibc`). Si necesitas recompilarlo en tu equipo (o en macOS / Windows con MinGW):

```bash
make            # compila ./evaluador para tu sistema
make static     # compila ./evaluador estático (portable entre versiones de Linux)
make test       # compila y ejecuta 60 verificaciones unitarias + lote de expresiones válidas
make clean      # elimina build/ y binarios
```

## Modos de Uso

```bash
# Modo interactivo (REPL: permite elegir Radianes o Grados)
./evaluador

# Evaluar una expresión directa (por defecto en radianes; -g para grados, -t para tokens, -v para variables)
./evaluador -e "sen(3.14 / 2) + cos(0) * 2^2"
./evaluador -g -e "sen(a + c)" -v a=45 -v c=45
./evaluador -e "sen(cos(x)) + tan(y)/cos(x)" -t
./evaluador -e "sen(x) + cos(y)" -v x=1.0 -v y=0.5

# Evaluar archivo (una expresión o en modo lote línea por línea)
./evaluador --lote tests/expresiones_validas.txt -v x=1.0 -v y=0.5 -v z=2.0
```

## Estructura del Proyecto

```
evaluador_trigonometrico/
├── docs/
│   ├── Informe_Tecnico.tex         # Fuente LaTeX del informe técnico
│   └── Informe_Tecnico.pdf         # Informe técnico en PDF
├── src/
│   ├── core/                       # Entidades e interfaz IEspecificacionLenguaje (de scanner_c)
│   ├── engine/                     # Motor AFD genérico (de scanner_c)
│   ├── languages/trig/             # EspecificacionTrigonometrica (sin/sen, cos, tan)
│   ├── parser/                     # ParserTrigonometrico LL(1) + validaciones + evaluación
│   ├── application/                # ServicioEscaner y ServicioEvaluador
│   ├── infrastructure/             # Carga de archivos e impresión de tokens/errores
│   └── cli/main.cpp                # Interfaz CLI (interactivo, -e, archivo, --lote)
├── tests/
│   ├── test_unitarios.cpp          # Pruebas unitarias e integración
│   ├── expresiones_validas.txt     # Casos válidos (incluye todos los del enunciado)
│   └── expresiones_invalidas.txt   # Casos de error léxico, sintáctico y matemático
├── output/
│   └── resultados_pruebas.txt      # Resultados y validación matemática contra Python math
└── Makefile
```
