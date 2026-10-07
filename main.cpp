#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <locale>
#include <sstream>
#include <ctime>
#include <limits>
#include <algorithm>
#include <cctype>
#include <clocale>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

using namespace std;

enum class Color {
    Default,
    Cyan,
    Blue,
    Orange,
    Green,
    Red,
    Yellow
};

struct TermSize {
    int cols;
    int rows;
};

struct Empleado {
    int id;
    string nombre;
    string apellidoP;
    string apellidoM;
    string fechaNac;   // DD/MM/YYYY
    string direccion;
    string fechaAlta;  // DD/MM/YYYY
};

// ANSI things

string useColorAnsi(Color color) {
    switch (color) {
        case Color::Cyan:
            return "\033[36m";
        case Color::Blue:
            return "\033[34m";
        case Color::Orange:
            return "\033[38;5;208m"; // Color naranja de 256bit
        case Color::Green:
            return "\033[32m";
        case Color::Red:
            return "\033[31m";
        case Color::Yellow:
            return "\033[33m";
        case Color::Default:
        default:
            return "\033[0m";
    }
}

string setParpadeoAnsi(bool enabled) {
    return enabled ? "\033[5m" : "\033[25m";
}

string setCursorVisibleANSI(bool visible) {
    return visible ? "\033[?25h" : "\033[?25l";
}

struct AnsiStyleGuard {
    explicit AnsiStyleGuard(bool hide_cursor = false) {
        if (hide_cursor) {
            cout << setCursorVisibleANSI(false) << flush;
        }
    }

    ~AnsiStyleGuard() {
        cout << "\033[0m" << "\033[25m" << setCursorVisibleANSI(true) << flush;
    }
};

#ifdef _WIN32
bool enableSecuenciasVT() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) {
        return false;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, mode) != 0;
}
#endif

TermSize getTermSize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return { cols, rows };
    }
    return { 80, 25 };
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        return { static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row) };
    }
    return { 80, 25 };
#endif
}

void moverCursor(int col, int row) {
    cout << "\033[" << row << ";" << col << "H";
}

// Estilos de print

void imprimirCentrado(const string & text) {
    TermSize ts = getTermSize();
    int pad = (ts.cols - static_cast <int> (text.size())) / 2;
    if (pad < 0) {
        pad = 0;
    }
    cout << string(pad, ' ') << text << "\n";
}

void imprimirCentradoInline(const string & text) {
    TermSize ts = getTermSize();
    int pad = (ts.cols - static_cast <int> (text.size())) / 2;
    if (pad < 0) {
        pad = 0;
    }
    cout << string(pad, ' ') << text;
}

void renderLineaCentradaInline(const string & text, Color color, const string & estilo) {
    TermSize ts = getTermSize();
    int pad = (ts.cols - static_cast<int>(text.size())) / 2;
    if (pad < 0) {
        pad = 0;
    }
    cout << "\r" << string(pad, ' ') << estilo << useColorAnsi(color) << text
         << useColorAnsi(Color::Default) << "\033[K" << flush;
}

void limpiarLineaActual() {
    cout << "\r\033[K" << flush;
}

void limpiarPantalla() {
    cout << "\033[2J\033[H" << flush;
}

void fadeTextoCentrado(const string & text, Color color, int totalMs, bool fadeIn) {
    vector<string> estilos = { "\033[2m", "\033[22m", "\033[1m" };
    if (!fadeIn) {
        reverse(estilos.begin(), estilos.end());
    }

    int pasos = static_cast<int>(estilos.size());
    int delayMs = pasos > 0 ? totalMs / pasos : totalMs;
    for (const auto & estilo : estilos) {
        renderLineaCentradaInline(text, color, estilo);
        this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
    cout << "\033[22m" << flush;
}

void printLoSentimos(string mensaje){
    cout << "\n";
    cout << setParpadeoAnsi(true) << useColorAnsi(Color::Red);
    imprimirCentrado("--- Lo sentimos, " + mensaje + " ---");
    cout << setParpadeoAnsi(false) << useColorAnsi(Color::Default);
}

void printAviso(string mensaje){
    cout << "\n";
    cout << setParpadeoAnsi(true) << useColorAnsi(Color::Yellow);
    imprimirCentrado("--- Aviso: " + mensaje + " ---");
    cout << setParpadeoAnsi(false) << useColorAnsi(Color::Default);
}

void printSuccess(string mensaje){
    cout << "\n";
    cout << setParpadeoAnsi(true) << useColorAnsi(Color::Green);
    imprimirCentrado("--- Éxito! " + mensaje + " ---");
    cout << setParpadeoAnsi(false) << useColorAnsi(Color::Default);
}

// Funciones para leer tipos de datos de forma segura

bool leerShortSeguro(const string & prompt, short & valor) {
    imprimirCentradoInline(prompt);
    if (!(cin >> valor)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

bool leerIntSeguro(const string & prompt, int & valor) {
    imprimirCentradoInline(prompt);
    if (!(cin >> valor)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

string leerLineaSeguro(const string & prompt) {
    imprimirCentradoInline(prompt);
    string valor;
    getline(cin >> ws, valor);
    return valor;
}

string leerLineaPermitirVacio(const string & prompt) {
    imprimirCentradoInline(prompt);
    string valor;
    getline(cin, valor);
    return valor;
}

bool contienePipe(const string & texto) {
    return texto.find('|') != string::npos;
}

string leerLineaSeguroSinPipe(const string & prompt) {
    while (true) {
        string valor = leerLineaSeguro(prompt);
        if (!contienePipe(valor)) {
            return valor;
        }
        printAviso("El caracter '|' no esta permitido");
    }
}

string leerLineaPermitirVacioSinPipe(const string & prompt) {
    while (true) {
        string valor = leerLineaPermitirVacio(prompt);
        if (valor.empty() || !contienePipe(valor)) {
            return valor;
        }
        printAviso("El caracter '|' no esta permitido");
    }
}

bool asegurarArchivoExiste(const string & rutaArchivo) {
    ifstream entrada(rutaArchivo);
    if (entrada.is_open()) {
        return true;
    }

    ofstream salida(rutaArchivo, ios::app);
    return salida.is_open();
}

bool esFechaValidaDDMMYYYY(const string & fecha) {
    if (fecha.size() != 10 || fecha[2] != '/' || fecha[5] != '/') {
        return false;
    }

    for (size_t i = 0; i < fecha.size(); ++i) {
        if (i == 2 || i == 5) {
            continue;
        }
        if (!isdigit(static_cast<unsigned char>(fecha[i]))) {
            return false;
        }
    }

    int dia = stoi(fecha.substr(0, 2));
    int mes = stoi(fecha.substr(3, 2));
    int anio = stoi(fecha.substr(6, 4));

    if (anio < 1 || mes < 1 || mes > 12 || dia < 1) {
        return false;
    }

    int diasPorMes[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool esBisiesto = (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0);
    if (esBisiesto) {
        diasPorMes[1] = 29;
    }

    if (dia > diasPorMes[mes - 1]) {
        return false;
    }

    return true;
}

string leerFechaValida(const string & prompt, bool permitirVacio) {
    while (true) {
        imprimirCentradoInline(prompt);
        string entrada;
        getline(cin, entrada);
        if (permitirVacio && entrada.empty()) {
            return "";
        }
        if (esFechaValidaDDMMYYYY(entrada)) {
            return entrada;
        }
        printAviso("Fecha invalida, use DD/MM/YYYY");
    }
}

// Funciones de Leer empleados

bool parseEmpleadoLinea(const string & linea, Empleado & out) {
    if (linea.empty()) {
        return false;
    }

    vector<string> campos;
    string campo;
    stringstream ss(linea);

    while (getline(ss, campo, '|')) {
        campos.push_back(campo);
    }

    if (campos.size() != 7) {
        return false;
    }

    try {
        out.id = stoi(campos[0]);
    } catch (const exception&) {
        return false;
    }

    out.nombre = campos[1];
    out.apellidoP = campos[2];
    out.apellidoM = campos[3];
    out.fechaNac = campos[4];
    out.direccion = campos[5];
    out.fechaAlta = campos[6];
    return true;
}

vector<Empleado> leerEmpleados(const string & rutaArchivo) {
    vector<Empleado> empleados;
    ifstream file(rutaArchivo);
    if (!file.is_open()) {
        return empleados;
    }

    string linea;
    int numLinea = 0;
    while (getline(file, linea)) {
        ++numLinea;
        Empleado emp;
        if (parseEmpleadoLinea(linea, emp)) {
            empleados.push_back(emp);
        } else if (!linea.empty()) {
            printAviso("Linea invalida en archivo (linea " + to_string(numLinea) + ")");
        }
    }

    return empleados;
}

//Funciones para agregar empleados

string serializarEmpleado(const Empleado& emp) {
    return to_string(emp.id) + "|" + emp.nombre + "|" + emp.apellidoP + "|" +
           emp.apellidoM + "|" + emp.fechaNac + "|" + emp.direccion + "|" +
           emp.fechaAlta;
}

bool guardarEmpleados(const string & rutaArchivo, const vector<Empleado> & empleados) {
    ofstream file(rutaArchivo, ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    for (const auto & emp : empleados) {
        file << serializarEmpleado(emp) << "\n";
    }

    return true;
}

int obtenerSiguienteId(const vector<Empleado> & empleados) {
    int maxId = 0;
    for (const auto & emp : empleados) {
        if (emp.id > maxId) {
            maxId = emp.id;
        }
    }
    return maxId + 1;
}

string obtenerFechaLocal() {
    time_t ahora = time(nullptr);
    tm tiempoLocal{};

    #ifdef _WIN32
        localtime_s(&tiempoLocal, &ahora);
    #else
        localtime_r(&ahora, &tiempoLocal);
    #endif

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &tiempoLocal);
    return string(buffer);
}

void altaEmpleado(const string & rutaArchivo) {
    vector<Empleado> empleados = leerEmpleados(rutaArchivo);

    Empleado nuevo{};
    nuevo.id = obtenerSiguienteId(empleados);
    nuevo.fechaAlta = obtenerFechaLocal();

    cout << "\n";
    imprimirCentrado("--- Alta de empleado ---");
    nuevo.nombre = leerLineaSeguroSinPipe("Nombre: ");
    nuevo.apellidoP = leerLineaSeguroSinPipe("Apellido paterno: ");
    nuevo.apellidoM = leerLineaSeguroSinPipe("Apellido materno: ");

    nuevo.fechaNac = leerFechaValida("Fecha de nacimiento (DD/MM/YYYY): ", false);

    nuevo.direccion = leerLineaSeguroSinPipe("Direccion: ");

    empleados.push_back(nuevo);

    limpiarPantalla();

    if (guardarEmpleados(rutaArchivo, empleados)) {
        printSuccess("Empleado agregado con ID " + to_string(nuevo.id));
    } else {
        printLoSentimos("no se pudo guardar el archivo");
    }
}

string aMinusculas(string s) {
    for (char & c : s) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c))); //Convierte de forma segura
    }
    return s;
}

vector<Empleado> buscarPorNombre(const vector<Empleado> & empleados, const string & query) {
    vector<Empleado> resultados;
    string q = aMinusculas(query);

    for (const auto & emp : empleados) {
        string nombre = aMinusculas(emp.nombre);
        string apePat = aMinusculas(emp.apellidoP);
        string apeMat = aMinusculas(emp.apellidoM);

        if (nombre.find(q) != string::npos || apePat.find(q) != string::npos || apeMat.find(q) != string::npos) {
            resultados.push_back(emp);
        }
    }

    return resultados;
}

int buscarIndicePorId(const vector<Empleado> & empleados, int id) {
    for (size_t i = 0; i < empleados.size(); ++i) {
        if (empleados[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

vector<int> buscarIndicesPorTexto(const vector<Empleado> & empleados, const string & query) {
    vector<int> indices;
    string q = aMinusculas(query);

    for (size_t i = 0; i < empleados.size(); ++i) {
        const auto & emp = empleados[i];
        string nombre = aMinusculas(emp.nombre);
        string apePat = aMinusculas(emp.apellidoP);
        string apeMat = aMinusculas(emp.apellidoM);

        if (nombre.find(q) != string::npos || apePat.find(q) != string::npos || apeMat.find(q) != string::npos) {
            indices.push_back(static_cast<int>(i));
        }
    }

    return indices;
}

void mostrarEmpleados(const vector<Empleado> & empleados) {
    if (empleados.empty()) {
        printAviso("No hay empleados para mostrar");
        return;
    }

    cout << "\n";
    imprimirCentrado("--- Lista de empleados ---");
    for (const auto & emp : empleados) {
        imprimirCentrado("ID: " + to_string(emp.id));
        imprimirCentrado("Nombre: " + emp.nombre + " " + emp.apellidoP + " " + emp.apellidoM);
        imprimirCentrado("Fecha Nac.: " + emp.fechaNac);
        imprimirCentrado("Direccion: " + emp.direccion);
        imprimirCentrado("Fecha Alta: " + emp.fechaAlta);
        imprimirCentrado("-------------------------");
    }
}

void imprimirSubmenuCentrado(const string & titulo, const vector<string> & opciones) {
    cout << "\n";
    imprimirCentrado(titulo);
    for (const auto & opcion : opciones) {
        imprimirCentrado(opcion);
    }
}

void consultaEmpleados(const string & rutaArchivo) {
    while (true) {
        imprimirSubmenuCentrado("--- Consulta de empleados ---", {
            "1. Buscar por nombre/apellido",
            "2. Buscar por ID",
            "3. Mostrar todos",
            "0. Volver"
        });
        short opcion = 0;
        if (!leerShortSeguro("Opcion: ", opcion)) {
            limpiarPantalla();
            printLoSentimos("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);

        switch (opcion) {
            case 1: {
                string texto = leerLineaSeguro("Texto a buscar: ");
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                mostrarEmpleados(resultados);
                break;
            }
            case 2: {
                int id = 0;
                if (!leerIntSeguro("ID: ", id)) {
                    printLoSentimos("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx >= 0) {
                    mostrarEmpleados(vector<Empleado>{empleados[idx]});
                } else {
                    limpiarPantalla();
                    printLoSentimos("No se encontro empleado con ese ID");
                }
                break;
            }
            case 3:
                mostrarEmpleados(empleados);
                break;
            case 0:
                limpiarPantalla();
                return;
            default:
                limpiarPantalla();
                printLoSentimos("opcion invalida");
        }
    }
}

void bajaEmpleado(const string & rutaArchivo) {
    while (true) {
        imprimirSubmenuCentrado("--- Baja de empleado ---", {
            "1. Dar de baja por ID",
            "2. Dar de baja por nombre/apellido",
            "0. Volver"
        });
        short opcion = 0;
        if (!leerShortSeguro("Opcion: ", opcion)) {
            limpiarPantalla();
            printLoSentimos("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);

        switch (opcion) {
            case 1: {
                int id = 0;
                if (!leerIntSeguro("ID: ", id)) {
                    limpiarPantalla();
                    printLoSentimos("ID invalido");
                    break;
                }
                limpiarPantalla();
                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    printLoSentimos("No se encontro empleado con ese ID");
                    break;
                }

                short decision = 0;
                //Mostrar empleado a eliminar
                mostrarEmpleados(vector<Empleado>{empleados[idx]});
                if(!leerShortSeguro("Estas seguro de eliminar a este empleado (1 = Si, 0 = No): ", decision) || decision != 1) {
                    limpiarPantalla();
                    printAviso("Operacion cancelada");
                }else{
                    empleados.erase(empleados.begin() + idx);
                
                    if (guardarEmpleados(rutaArchivo, empleados)) {
                        printAviso("Empleado eliminado");
                    } else {
                        printLoSentimos("no se pudo guardar el archivo");
                    }
                }
                break;
            }
            case 2: {
                string texto = leerLineaSeguro("Texto a buscar: ");
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                if (indices.empty()) {
                    limpiarPantalla();
                    printAviso("No hay coincidencias");
                    break;
                }

                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                limpiarPantalla();
                mostrarEmpleados(resultados);

                int id = 0;
                if (!leerIntSeguro("ID a eliminar: ", id)) {
                    limpiarPantalla();
                    printLoSentimos("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    limpiarPantalla();
                    printLoSentimos("No se encontro empleado con ese ID");
                    break;
                }

                bool permitido = false;
                for (int idxPermitido : indices) {
                    if (idxPermitido == idx) {
                        permitido = true;
                        break;
                    }
                }

                if (!permitido) {
                    limpiarPantalla();
                    printAviso("Ese ID no esta en los resultados");
                    break;
                }

                empleados.erase(empleados.begin() + idx);
                limpiarPantalla();
                if (guardarEmpleados(rutaArchivo, empleados)) {
                    printAviso("Empleado eliminado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 0:
                limpiarPantalla();
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

void modificarEmpleado(const string & rutaArchivo) {
    while (true) {
        imprimirSubmenuCentrado("--- Modificacion de empleado ---", {
            "1. Modificar por ID",
            "2. Modificar por nombre/apellido",
            "0. Volver"
        });
        short opcion = 0;
        if (!leerShortSeguro("Opcion: ", opcion)) {
            limpiarPantalla();
            printLoSentimos("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);
        if (empleados.empty()) {
            limpiarPantalla();
            printAviso("No hay empleados registrados");
            continue;
        }

        auto editarCampos = [](Empleado & emp) {
            string entrada;
            limpiarPantalla();
            imprimirCentrado("--- Modificar empleado ID " + to_string(emp.id) + " ---");

            entrada = leerLineaPermitirVacioSinPipe("Nombre (" + emp.nombre + ") [dejar vacio para mantener]: ");
            if (!entrada.empty()) emp.nombre = entrada;

            entrada = leerLineaPermitirVacioSinPipe("Apellido paterno (" + emp.apellidoP + ") [dejar vacio para mantener]: ");
            if (!entrada.empty()) emp.apellidoP = entrada;

            entrada = leerLineaPermitirVacioSinPipe("Apellido materno (" + emp.apellidoM + ") [dejar vacio para mantener]: ");
            if (!entrada.empty()) emp.apellidoM = entrada;

            string nuevaFecha = leerFechaValida("Fecha de nacimiento (" + emp.fechaNac + ") [dejar vacio para mantener]: ", true);
            if (!nuevaFecha.empty()) emp.fechaNac = nuevaFecha;

            entrada = leerLineaPermitirVacioSinPipe("Direccion (" + emp.direccion + ") [dejar vacio para mantener]: ");
            if (!entrada.empty()) emp.direccion = entrada;
        };

        switch (opcion) {
            case 1: {
                int id = 0;
                if (!leerIntSeguro("ID: ", id)) {
                    limpiarPantalla();
                    printLoSentimos("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    limpiarPantalla();
                    printLoSentimos("No se encontro empleado con ese ID");
                    break;
                }

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                editarCampos(empleados[idx]);

                limpiarPantalla();
                if (guardarEmpleados(rutaArchivo, empleados)) {
                    printSuccess("Empleado actualizado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 2: {
                string texto = leerLineaSeguro("Texto a buscar: ");
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                if (indices.empty()) {
                    limpiarPantalla();
                    printAviso("No hay coincidencias");
                    break;
                }

                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                mostrarEmpleados(resultados);

                int id = 0;
                if (!leerIntSeguro("ID a modificar: ", id)) {
                    limpiarPantalla();
                    printLoSentimos("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    limpiarPantalla();
                    printAviso("No se encontro empleado con ese ID");
                    break;
                }

                bool permitido = false;
                for (int idxPermitido : indices) {
                    if (idxPermitido == idx) {
                        permitido = true;
                        break;
                    }
                }

                if (!permitido) {
                    limpiarPantalla();
                    printAviso("Ese ID no esta en los resultados");
                    break;
                }

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                editarCampos(empleados[idx]);
                limpiarPantalla();
                if (guardarEmpleados(rutaArchivo, empleados)) {
                    printSuccess("Empleado actualizado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 0:
                limpiarPantalla();
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

//Interfaz

void barraDeCarga() {
    cout << "\n";
    AnsiStyleGuard style_guard(true);
    for (int i = 0; i <= 100; ++i) {
        int pos = i / 2;
        string barra = "[";
        for (int j = 0; j < 50; ++j) {
            if (j < pos) barra += "=";
            else if (j == pos) barra += ">";
            else barra += " ";
        }
        barra += "] " + to_string(i) + "%";

        renderLineaCentradaInline(barra, Color::Cyan, "");
        
        // Simular trabajo
        this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    fadeTextoCentrado("CARGA COMPLETA!", Color::Green, 600, true);
    fadeTextoCentrado("CARGA COMPLETA!", Color::Green, 600, false);
    limpiarLineaActual();
}

string textBienvenida(){
    return "\n\t\t\t\tBIENVENIDO AL\n\t\t\tSISTEMA DE GESTION DE EMPLEADOS\n\n";
}

void imprimirMenuCentrado(){
    cout << "\n";
    imprimirCentrado("1. Dar de alta empleado");
    imprimirCentrado("2. Dar de baja empleado");
    imprimirCentrado("3. Consulta");
    imprimirCentrado("4. Modificacion");
    imprimirCentrado("0. Salir");
}

void menuCiclado(const string & rutaArchivo){
    while(true){
        imprimirMenuCentrado();
        short opcion = 0;
        if (!leerShortSeguro("Opcion: ", opcion)) {
            printLoSentimos("Entrada invalida");
            continue;
        }
        switch(opcion){
            case 1:
                limpiarPantalla();
                altaEmpleado(rutaArchivo);
                break;
            case 2:
                limpiarPantalla();
                bajaEmpleado(rutaArchivo);
                break;
            case 3:
                limpiarPantalla();
                consultaEmpleados(rutaArchivo);
                break;
            case 4:
                limpiarPantalla();
                modificarEmpleado(rutaArchivo);
                break;
            case 0:
                limpiarPantalla();
                fadeTextoCentrado("Gracias, gracias, por usar mi sistema.", Color::Green, 600, true);
                fadeTextoCentrado("Gracias, gracias, por usar mi sistema.", Color::Green, 600, false);
                fadeTextoCentrado("Nos vemos!", Color::Green, 600, true);
                fadeTextoCentrado("Nos vemos!", Color::Green, 600, false);
                limpiarPantalla();
                return;
            default:
                printLoSentimos("opcion invalida");
                opcion = -1; //Con esto evitamos un ciclo sin fin
        }
    }
}

int main(){
    #ifdef _WIN32
        enableSecuenciasVT();
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    setlocale(LC_ALL, "es_MX.UTF-8");

    AnsiStyleGuard global_style_guard;

    limpiarPantalla();

    barraDeCarga();
    cout << useColorAnsi(Color::Cyan);
    imprimirCentrado("BIENVENIDO AL");
    imprimirCentrado("SISTEMA DE GESTION DE EMPLEADOS");
    cout << useColorAnsi(Color::Default) << "\n";
    
    const string rutaArchivo = "empleados.txt";
    if (!asegurarArchivoExiste(rutaArchivo)) {
        printLoSentimos("no se pudo abrir el archivo");
        return 1;
    }

    menuCiclado(rutaArchivo);
}