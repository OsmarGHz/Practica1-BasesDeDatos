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
    Red
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

void printLoSentimos(string mensaje){
    cout << setParpadeoAnsi(true) << useColorAnsi(Color::Red)
             << "\n\t\t--- Lo sentimos, " + mensaje + "---\n"
             << setParpadeoAnsi(false) << useColorAnsi(Color::Default);
}

string printAviso(string mensaje){
    return "\n\t\t--- Aviso: " + mensaje + " ---\n";
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
            cout << printAviso("Linea invalida en archivo (linea " + to_string(numLinea) + ")");
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
    if (empleados.empty()) {
        return 1;
    }
    return empleados.back().id + 1;
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

    cout << "\n--- Alta de empleado ---\n";
    cout << "Nombre: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); //Limpiar el buffer antes de usar getline
    getline(cin, nuevo.nombre);

    cout << "Apellido paterno: ";
    getline(cin, nuevo.apellidoP);

    cout << "Apellido materno: ";
    getline(cin, nuevo.apellidoM);

    cout << "Fecha de nacimiento (DD/MM/YYYY): ";
    getline(cin, nuevo.fechaNac);

    cout << "Direccion: ";
    getline(cin, nuevo.direccion);

    empleados.push_back(nuevo);

    if (guardarEmpleados(rutaArchivo, empleados)) {
        cout << printAviso("Empleado agregado con ID " + to_string(nuevo.id));
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
        cout << printAviso("No hay empleados para mostrar");
        return;
    }

    cout << "\n--- Lista de empleados ---\n";
    for (const auto & emp : empleados) {
        cout << "ID: " << emp.id << "\n";
        cout << "Nombre: " << emp.nombre << " " << emp.apellidoP << " " << emp.apellidoM << "\n";
        cout << "Fecha Nac.: " << emp.fechaNac << "\n";
        cout << "Direccion: " << emp.direccion << "\n";
        cout << "Fecha Alta: " << emp.fechaAlta << "\n";
        cout << "-------------------------\n";
    }
}

void consultaEmpleados(const string & rutaArchivo) {
    while (true) {
        cout << "\n--- Consulta de empleados ---\n";
        cout << "1. Buscar por nombre/apellido\n";
        cout << "2. Buscar por ID\n";
        cout << "3. Mostrar todos\n";
        cout << "0. Volver\n";
        cout << "Opcion: ";

        short opcion = 0;
        cin >> opcion;
        if (!cin) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << printAviso("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);

        switch (opcion) {
            case 1: {
                cout << "Texto a buscar: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                string texto;
                getline(cin, texto);
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                mostrarEmpleados(resultados);
                break;
            }
            case 2: {
                cout << "ID: ";
                int id = 0;
                cin >> id;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << printAviso("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx >= 0) {
                    mostrarEmpleados(vector<Empleado>{empleados[idx]});
                } else {
                    cout << printAviso("No se encontro empleado con ese ID");
                }
                break;
            }
            case 3:
                mostrarEmpleados(empleados);
                break;
            case 0:
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

void bajaEmpleado(const string & rutaArchivo) {
    while (true) {
        cout << "\n--- Baja de empleado ---\n";
        cout << "1. Dar de baja por ID\n";
        cout << "2. Dar de baja por nombre/apellido\n";
        cout << "0. Volver\n";
        cout << "Opcion: ";

        short opcion = 0;
        cin >> opcion;
        if (!cin) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << printAviso("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);

        switch (opcion) {
            case 1: {
                cout << "ID: ";
                int id = 0;
                cin >> id;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << printAviso("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    cout << printAviso("No se encontro empleado con ese ID");
                    break;
                }

                empleados.erase(empleados.begin() + idx);
                if (guardarEmpleados(rutaArchivo, empleados)) {
                    cout << printAviso("Empleado eliminado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 2: {
                cout << "Texto a buscar: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                string texto;
                getline(cin, texto);
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                if (indices.empty()) {
                    cout << printAviso("No hay coincidencias");
                    break;
                }

                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                mostrarEmpleados(resultados);

                cout << "ID a eliminar: ";
                int id = 0;
                cin >> id;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << printAviso("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    cout << printAviso("No se encontro empleado con ese ID");
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
                    cout << printAviso("Ese ID no esta en los resultados");
                    break;
                }

                empleados.erase(empleados.begin() + idx);
                if (guardarEmpleados(rutaArchivo, empleados)) {
                    cout << printAviso("Empleado eliminado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 0:
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

void modificarEmpleado(const string & rutaArchivo) {
    while (true) {
        cout << "\n--- Modificacion de empleado ---\n";
        cout << "1. Modificar por ID\n";
        cout << "2. Modificar por nombre/apellido\n";
        cout << "0. Volver\n";
        cout << "Opcion: ";

        short opcion = 0;
        cin >> opcion;
        if (!cin) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << printAviso("Entrada invalida");
            continue;
        }

        vector<Empleado> empleados = leerEmpleados(rutaArchivo);
        if (empleados.empty()) {
            cout << printAviso("No hay empleados registrados");
            continue;
        }

        auto editarCampos = [](Empleado & emp) {
            string entrada;
            cout << "\n--- Modificar empleado ID " << emp.id << " ---\n";

            cout << "Nombre (" << emp.nombre << "): ";
            getline(cin, entrada);
            if (!entrada.empty()) emp.nombre = entrada;

            cout << "Apellido paterno (" << emp.apellidoP << "): ";
            getline(cin, entrada);
            if (!entrada.empty()) emp.apellidoP = entrada;

            cout << "Apellido materno (" << emp.apellidoM << "): ";
            getline(cin, entrada);
            if (!entrada.empty()) emp.apellidoM = entrada;

            cout << "Fecha de nacimiento (" << emp.fechaNac << "): ";
            getline(cin, entrada);
            if (!entrada.empty()) emp.fechaNac = entrada;

            cout << "Direccion (" << emp.direccion << "): ";
            getline(cin, entrada);
            if (!entrada.empty()) emp.direccion = entrada;
        };

        switch (opcion) {
            case 1: {
                cout << "ID: ";
                int id = 0;
                cin >> id;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << printAviso("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    cout << printAviso("No se encontro empleado con ese ID");
                    break;
                }

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                editarCampos(empleados[idx]);

                if (guardarEmpleados(rutaArchivo, empleados)) {
                    cout << printAviso("Empleado actualizado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 2: {
                cout << "Texto a buscar: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                string texto;
                getline(cin, texto);
                vector<int> indices = buscarIndicesPorTexto(empleados, texto);
                if (indices.empty()) {
                    cout << printAviso("No hay coincidencias");
                    break;
                }

                vector<Empleado> resultados;
                for (int idx : indices) {
                    resultados.push_back(empleados[idx]);
                }
                mostrarEmpleados(resultados);

                cout << "ID a modificar: ";
                int id = 0;
                cin >> id;
                if (!cin) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << printAviso("ID invalido");
                    break;
                }

                int idx = buscarIndicePorId(empleados, id);
                if (idx < 0) {
                    cout << printAviso("No se encontro empleado con ese ID");
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
                    cout << printAviso("Ese ID no esta en los resultados");
                    break;
                }

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                editarCampos(empleados[idx]);

                if (guardarEmpleados(rutaArchivo, empleados)) {
                    cout << printAviso("Empleado actualizado");
                } else {
                    printLoSentimos("no se pudo guardar el archivo");
                }
                break;
            }
            case 0:
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

//Interfaz

void barraDeCarga() {
    cout << "\n\n";
    AnsiStyleGuard style_guard(true);
    for (int i = 0; i <= 100; ++i) {
        cout << "\r"; // Retorno de carro que vuelve al inicio de la linea sin bajar

        cout << useColorAnsi(Color::Cyan) << "["; // Color Cyan
        
        int pos = i / 2;
        for (int j = 0; j < 50; ++j) {
            if (j < pos) cout << "=";
            else if (j == pos) cout << ">";
            else cout << " ";
        }
        
        cout << "] " << i << "%" << useColorAnsi(Color::Default); // Cerrar color
        cout << flush;
        
        // Simular trabajo
        this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    cout << "\n\n" << "\033[1m" << useColorAnsi(Color::Green) << "CARGA COMPLETA!"
         << useColorAnsi(Color::Default) << "\n";
}

string printBienvenida(){
    return "\n\t\t\t\tBIENVENIDO AL\n\t\t\tSISTEMA DE GESTION DE EMPLEADOS\n\n";
}

string printMenu(){
    return "1. Dar de alta empleado\n2. Dar de baja empleado\n3. Consulta\n4. Modificacion\n0. Salir\n";
}

void menuCiclado(const string & rutaArchivo){
    while(true){
        cout << printMenu();
        short opcion;
        cin >> opcion;
        switch(opcion){
            case 1:
                altaEmpleado(rutaArchivo);
                break;
            case 2:
                bajaEmpleado(rutaArchivo);
                break;
            case 3:
                consultaEmpleados(rutaArchivo);
                break;
            case 4:
                modificarEmpleado(rutaArchivo);
                break;
            case 0:
                cout << useColorAnsi(Color::Green) << "\nGracias, gracias cuate, por usar mi sistema.\nNos vemos!" 
                     << useColorAnsi(Color::Default) << "\n";
                return;
            default:
                printLoSentimos("opcion invalida");
        }
    }
}

int main(){
    #ifdef _WIN32
        enableSecuenciasVT();
    #endif

    AnsiStyleGuard global_style_guard;

    barraDeCarga();
    cout << useColorAnsi(Color::Blue);
    imprimirCentrado("BIENVENIDO AL");
    imprimirCentrado("SISTEMA DE GESTION DE EMPLEADOS");
    cout << useColorAnsi(Color::Default) << "\n";

    cout << useColorAnsi(Color::Green);
    imprimirCentrado("Operacion exitosa :)");
    cout << useColorAnsi(Color::Default);
    const string rutaArchivo = "C:\\empleados.txt";
    fstream file(rutaArchivo);
    // if(!file.is_open()){
    //     printLoSentimos("no se pudo abrir el archivo");
    // }

    menuCiclado(rutaArchivo);
}