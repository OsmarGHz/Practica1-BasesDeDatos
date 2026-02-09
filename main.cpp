#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <locale>

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

void imprimirCentrado(const string & text) {
    TermSize ts = getTermSize();
    int pad = (ts.cols - static_cast <int> (text.size())) / 2;
    if (pad < 0) {
        pad = 0;
    }
    cout << string(pad, ' ') << text << "\n";
}

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

void printLoSentimos(string mensaje){
    cout << setParpadeoAnsi(true) << useColorAnsi(Color::Red)
             << "\n\t\t--- Lo sentimos, " + mensaje + "---\n"
             << setParpadeoAnsi(false) << useColorAnsi(Color::Default);
}

string printAviso(string mensaje){
    return "\n\t\t--- Aviso: " + mensaje + " ---\n";
}

string printBienvenida(){
    return "\n\t\t\t\tBIENVENIDO AL\n\t\t\tSISTEMA DE GESTION DE EMPLEADOS\n\n";
}

string printMenu(){
    return "1. Agregar empleado\n2. Eliminar empleado\n3. Modificar empleado\n4. Listar empleados\n5. Salir\n";
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
    fstream file("C:\\empleados.txt");
    if(!file.is_open()){
        printLoSentimos("no se pudo abrir el archivo");
    }
}