#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <locale>


using namespace std;

void barraDeCarga() {
    cout << "\n\n";
    for (int i = 0; i <= 100; ++i) {
        cout << "\r"; // Retorno de carro: vuelve al inicio de la linea sin bajar
        
        cout << "\033[36m" << "["; // Color Cyan
        
        int pos = i / 2;
        for (int j = 0; j < 50; ++j) {
            if (j < pos) cout << "=";
            else if (j == pos) cout << ">";
            else cout << " ";
        }
        
        cout << "] " << i << "%" << "\033[0m"; // Cerrar color
        cout << flush;
        
        // Simular trabajo
        this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    cout << "\n\n\033[1;32m" << "CARGA COMPLETA!" << "\033[0m" << "\n";
}

string printLoSentimos(string mensaje){
    return "\n\t\t--- Lo sentimos, " + mensaje + "---\n";
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
    barraDeCarga();
    cout << printBienvenida();
    cout << "\033[1;32m" << "Operacion exitosa :)" << "\033[0m" << std::endl;
    fstream file("C:\\empleados.txt");
    if(!file.is_open()){
        cout << printLoSentimos("no se pudo abrir el archivo");
    }
}