#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../application/evaluator_service.h"
#include "../infrastructure/error_reporter.h"
#include "../infrastructure/file_loader.h"
#include "../infrastructure/token_printer.h"
#include "../languages/trig/trig_language.h"

namespace {

struct OpcionesCLI {
    bool mostrarTokens = false;
    bool modoLote = false;
    bool tieneExpresionInline = false;
    bool modoEspecificadoPorCLI = false;
    ModoAngulo modoAngulo = ModoAngulo::RADIANES;
    std::string expresionInline;
    std::string rutaArchivo;
    std::unordered_map<std::string, double> variablesPredefinidas;
};

void imprimirUso(const char* prog) {
    std::cout << "Evaluador Trigonometrico LL(1)\n"
              << "Uso:\n"
              << "  " << prog << "                          Modo interactivo (REPL)\n"
              << "  " << prog << " -e \"<expr>\" [-g] [-t] [-v ...] Evalua una expresion\n"
              << "  " << prog << " <archivo> [-g] [-t] [-v ...]     Evalua un archivo\n"
              << "  " << prog << " --lote <archivo> [-g] [-v ...]   Evalua cada linea de un archivo\n\n"
              << "Opciones:\n"
              << "  -e, --expr <texto>   Expresion a evaluar directamente\n"
              << "  -v, --var x=val      Asigna valor numerico a una variable (repetible)\n"
              << "  -g, --grados, --deg  Interpreta argumentos de sin/sen/cos/tan en grados\n"
              << "  -r, --radianes       Interpreta argumentos trigonometricos en radianes (por defecto)\n"
              << "  -t, --tokens         Muestra los tokens reconocidos por el AFD\n"
              << "  --lote               Procesa el archivo linea por linea\n"
              << "  -h, --help           Muestra esta ayuda\n";
}

bool parsearAsignacionVariable(
    const std::string& arg, std::unordered_map<std::string, double>& destino) {
    size_t eq = arg.find('=');
    if (eq == std::string::npos || eq == 0 || eq + 1 >= arg.size()) {
        return false;
    }
    std::string nombre = arg.substr(0, eq);
    double valor = 0.0;
    if (!ServicioEvaluador::parsearNumeroReal(arg.substr(eq + 1), valor)) {
        return false;
    }
    destino[nombre] = valor;
    return true;
}

bool parsearArgumentos(int argc, char* argv[], OpcionesCLI& opts) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            imprimirUso(argv[0]);
            std::exit(EXIT_SUCCESS);
        } else if (arg == "-t" || arg == "--tokens") {
            opts.mostrarTokens = true;
        } else if (arg == "--lote") {
            opts.modoLote = true;
        } else if (arg == "-g" || arg == "--grados" || arg == "--deg") {
            opts.modoAngulo = ModoAngulo::GRADOS;
            opts.modoEspecificadoPorCLI = true;
        } else if (arg == "-r" || arg == "--radianes" || arg == "--rad") {
            opts.modoAngulo = ModoAngulo::RADIANES;
            opts.modoEspecificadoPorCLI = true;
        } else if (arg == "-e" || arg == "--expr") {
            if (i + 1 >= argc) {
                std::cerr << "Error: falta la expresion despues de " << arg << "\n";
                return false;
            }
            opts.tieneExpresionInline = true;
            opts.expresionInline = argv[++i];
        } else if (arg == "-v" || arg == "--var") {
            if (i + 1 >= argc || !parsearAsignacionVariable(argv[++i], opts.variablesPredefinidas)) {
                std::cerr << "Error: formato invalido en " << arg << " (use nombre=numero)\n";
                return false;
            }
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Error: opcion desconocida '" << arg << "'\n";
            return false;
        } else if (opts.rutaArchivo.empty()) {
            opts.rutaArchivo = arg;
        } else {
            std::cerr << "Error: argumento inesperado '" << arg << "'\n";
            return false;
        }
    }
    return true;
}

void imprimirErrorParser(const ErrorAnalisis& err) {
    std::cerr << "[ERROR] " << err.nombreFase()
              << " (linea " << err.linea << ", col " << err.columna << "): "
              << err.mensaje;
    if (!err.contexto.empty()) {
        std::cerr << " [cerca de \"" << err.contexto << "\"]";
    }
    std::cerr << "\n";
}

bool solicitarVariables(
    const std::vector<std::string>& variables,
    const std::unordered_map<std::string, double>& predefinidas,
    ModoAngulo modo,
    std::unordered_map<std::string, double>& valores) {
    valores = predefinidas;
    const char* etiquetaUnidad = (modo == ModoAngulo::GRADOS) ? "grados si es angulo" : "radianes si es angulo";
    for (const std::string& nombre : variables) {
        if (valores.find(nombre) != valores.end()) {
            continue;
        }
        while (true) {
            std::cout << "  Ingrese valor para '" << nombre << "' (" << etiquetaUnidad << "): ";
            std::string linea;
            if (!std::getline(std::cin, linea)) {
                std::cerr << "\n[ERROR] Fin de entrada al esperar el valor de '" << nombre << "'.\n";
                return false;
            }
            double numero = 0.0;
            if (ServicioEvaluador::parsearNumeroReal(linea, numero)) {
                valores[nombre] = numero;
                break;
            }
            std::cerr << "  Valor invalido. Ingrese un numero real (ej: 0, 3.14, -1.5, .5, 1.5e-3).\n";
        }
    }
    return true;
}

bool procesarExpresion(
    const ServicioEvaluador& servicio,
    const std::string& expresion,
    const OpcionesCLI& opts) {
    ResultadoAnalisisExpresion analisis = servicio.analizar(expresion);

    if (opts.mostrarTokens) {
        imprimirComponentes(analisis.componentes, EspecificacionTrigonometrica::instancia());
    }

    if (!analisis.erroresLexicos.empty()) {
        imprimirResumen(analisis.erroresLexicos);
        return false;
    }

    if (analisis.errorParser.has_value()) {
        imprimirErrorParser(*analisis.errorParser);
        return false;
    }

    std::unordered_map<std::string, double> valores;
    if (!solicitarVariables(analisis.variables, opts.variablesPredefinidas, opts.modoAngulo, valores)) {
        return false;
    }

    ResultadoEvaluacion eval = servicio.evaluar(analisis.componentes, valores, opts.modoAngulo);
    if (!eval.exito) {
        if (eval.error.has_value()) {
            imprimirErrorParser(*eval.error);
        }
        return false;
    }

    std::cout << std::setprecision(15) << "Resultado: " << eval.valor << "\n";
    return true;
}

ModoAngulo seleccionarUnidadInicial() {
    while (true) {
        std::cout << "Seleccione unidad angular [1 = Radianes (por defecto), 2 = Grados]: ";
        std::string opcion;
        if (!std::getline(std::cin, opcion)) {
            return ModoAngulo::RADIANES;
        }
        if (opcion.empty() || opcion == "1" || opcion == "rad" || opcion == "radianes") {
            return ModoAngulo::RADIANES;
        }
        if (opcion == "2" || opcion == "g" || opcion == "deg" || opcion == "grados") {
            return ModoAngulo::GRADOS;
        }
        std::cout << "Opcion invalida. Escriba 1 (Radianes) o 2 (Grados).\n";
    }
}

int ejecutarInteractivo(const ServicioEvaluador& servicio, OpcionesCLI opts) {
    std::cout << "====================================================\n"
              << "  Evaluador Trigonometrico LL(1) (precision double)\n"
              << "  Funciones: sin, sen, cos, tan  | Ops: + - * / ^\n"
              << "  Comandos: 'rad' / 'grados' para cambiar unidad\n"
              << "            'salir' o Ctrl+D para terminar\n"
              << "====================================================\n";
    if (!opts.modoEspecificadoPorCLI) {
        opts.modoAngulo = seleccionarUnidadInicial();
    }

    std::string linea;
    while (true) {
        std::string prefijo = (opts.modoAngulo == ModoAngulo::GRADOS) ? "[DEG]" : "[RAD]";
        std::cout << "\n" << prefijo << " > ";
        if (!std::getline(std::cin, linea)) {
            std::cout << "\n";
            break;
        }
        if (linea == "salir" || linea == "exit" || linea == "quit") {
            break;
        }
        if (linea == "rad" || linea == "radianes") {
            opts.modoAngulo = ModoAngulo::RADIANES;
            std::cout << "Modo cambiado a RADIANES.\n";
            continue;
        }
        if (linea == "deg" || linea == "grados" || linea == "g") {
            opts.modoAngulo = ModoAngulo::GRADOS;
            std::cout << "Modo cambiado a GRADOS.\n";
            continue;
        }
        if (linea.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        procesarExpresion(servicio, linea, opts);
    }
    return EXIT_SUCCESS;
}

int ejecutarLote(
    const ServicioEvaluador& servicio,
    const std::string& contenido,
    const OpcionesCLI& opts) {
    std::istringstream stream(contenido);
    std::string linea;
    int numeroLinea = 0;
    int fallos = 0;

    while (std::getline(stream, linea)) {
        numeroLinea++;
        size_t primero = linea.find_first_not_of(" \t\r\n");
        if (primero == std::string::npos) continue;
        if (linea.compare(primero, 2, "//") == 0) continue;

        std::cout << "\n[" << numeroLinea << "] Expresion: " << linea << "\n";
        if (!procesarExpresion(servicio, linea, opts)) {
            fallos++;
        }
    }
    return (fallos == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

}

int main(int argc, char* argv[]) {
    OpcionesCLI opts;
    if (!parsearArgumentos(argc, argv, opts)) {
        return EXIT_FAILURE;
    }

    ServicioEvaluador servicio;

    if (opts.tieneExpresionInline) {
        return procesarExpresion(servicio, opts.expresionInline, opts)
            ? EXIT_SUCCESS
            : EXIT_FAILURE;
    }

    if (!opts.rutaArchivo.empty()) {
        ResultadoCarga carga = cargarArchivoFuente(opts.rutaArchivo);
        if (!carga.exito) {
            std::cerr << carga.error << "\n";
            return EXIT_FAILURE;
        }
        if (carga.estaVacio) {
            std::cerr << "Advertencia: El archivo esta vacio.\n";
            return EXIT_SUCCESS;
        }
        if (opts.modoLote) {
            return ejecutarLote(servicio, carga.fuente, opts);
        }
        return procesarExpresion(servicio, carga.fuente, opts)
            ? EXIT_SUCCESS
            : EXIT_FAILURE;
    }

    return ejecutarInteractivo(servicio, opts);
}
