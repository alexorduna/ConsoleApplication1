#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include "Md5hash.h"
using namespace std;

// Estructura para representar un nodo en la lista enlazada
struct Nodo {

    string Nombreh;
    string Tamanioh;
    wstring NombreA;
    wstring ubicacion;
    uint64_t tamanio;
    bool DuplicadoN;
    bool DuplicadoT;
    Nodo* nodo;

};

// Clase para manejar los nodos y detectar duplicados
class ListaEnlazada {
private:

    Nodo* cabeza;

public:

    ListaEnlazada() : cabeza(nullptr) {}

    // Agregar un nuevo nodo a la lista
    void agregar(const string& Nombreh, const string& Tamanioh,

        const wstring& nombre, const wstring& ubicacion, uint64_t tamanio) {
        bool DuplicadoN = false;
        bool DuplicadoT = false;
        Nodo* a = cabeza; 

        // Verificar si ya existen hashes repetidos
        while (a) {
            if (a->Nombreh == Nombreh) {
                DuplicadoN = true;
            }
            if (a->Tamanioh == Tamanioh) {
                DuplicadoT = true;
            }
            a = a->nodo;
        }

        // Agregar el nuevo nodo con la informacion
        Nodo* nuevo = new Nodo{ Nombreh, Tamanioh, nombre, ubicacion, tamanio, DuplicadoN, DuplicadoT, cabeza };
        cabeza = nuevo;
    }

    // Mostrar los hashes en la lista
    void mostrar() {

        Nodo* a = cabeza;

        cout << "\nLista de hashes:\n";

        while (a) {

            wcout << L"Archivo: " << a->NombreA << L"\n" << L" | Ubicacion: " << a->ubicacion << L"\n" << L" | Tamano: " << a->tamanio << L" bytes" << L"\n"
                << L" | Hash Nombre: " << a->Nombreh.c_str() << L"\n" << L" | Hash Tamano: " << a->Tamanioh.c_str();

            if (a->DuplicadoN || a->DuplicadoT) { 
                wcout << L" (duplicado)";
            }

            wcout << endl;

            a = a->nodo;
        }
    }

    // Mostrar el resumen de archivos duplicados
    void mostrarDuplicados() {
        Nodo* a = cabeza;

        cout << "\nResumen de archivos duplicados:\n";

        bool hayDuplicados = false;

        while (a) { 
              
            if (a->DuplicadoN || a->DuplicadoT) { 
                hayDuplicados = true;

                wcout << L"Archivo: " << a->NombreA << L"\n" << L" | Ubicacion: " << a->ubicacion << L"\n" << L" | Tamano: " << a->tamanio << L" bytes" << L"\n"  
                    << L" | Hash Nombre: " << a->Nombreh.c_str() << L"\n" << L" | Hash Tamano: " << a->Tamanioh.c_str(); 
            }

            a = a->nodo; 
        }

        if (!hayDuplicados) {

            cout << "No se encontraron archivos duplicados.\n";
        }
    }

    void eliminarDuplicados() { 
        Nodo* actual = cabeza; 
        int decision;

        cout << "\nDesea eliminar los archivos duplicados?\n";
        cin >> decision; 

        while (actual) {
            if (actual->DuplicadoN || actual->DuplicadoT) { 
                if (DeleteFileW((actual->ubicacion + L"\\" + actual->NombreA).c_str())) { 
                    wcout << actual->NombreA << L" en " << actual->ubicacion << L"\nArchivo eliminado exitosamente! " << L"\n";
                }
                else {
                    wcout << L"Error al intentar eliminar: " << actual->NombreA << L" en " << actual->ubicacion << L"\n";
                }
            }
            actual = actual->nodo;
        }
    }


    // Liberar memoria al destruir la lista
    ~ListaEnlazada() {

        while (cabeza) {
            Nodo* temp = cabeza;
            cabeza = cabeza->nodo;
            delete temp;
        }
    }
};

// Funcion para calcular el tamanio del archivo
uint64_t getTamaniofile(const WIN32_FIND_DATAW& datosArchivo) { 

    LARGE_INTEGER tamanio;

    tamanio.HighPart = datosArchivo.nFileSizeHigh; 
    tamanio.LowPart = datosArchivo.nFileSizeLow;

    return static_cast<uint64_t>(tamanio.QuadPart); 
}

// Funcion para listar archivos, calcular hashes y detectar duplicados
void listar_y_hashear(const wstring& directorio, ListaEnlazada& lista) {

    wstring ruta = directorio + L"\\*"; 
    WIN32_FIND_DATAW datosFile;

    HANDLE hFind = FindFirstFileW(ruta.c_str(), &datosFile);

    if (hFind == INVALID_HANDLE_VALUE) {

        wcout << L"Error al abrir el directorio: " << directorio << L'\n';
        return;
    }

    do {

        const wstring nombreFile = datosFile.cFileName;

        // Ignorar directorios "." y ".."
        if (nombreFile == L"." || nombreFile == L"..") {
            continue;
        }

        // Comprobar si es un directorio
        if (datosFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { 

            listar_y_hashear(directorio + L"\\" + nombreFile, lista);
        }
        else
        {
            // Calcular el tamanio del archivo
            uint64_t tamanioFile = getTamaniofile(datosFile); 

            // Convertir el nombre del archivo a una cadena estandar
            string nombreUTF8(nombreFile.begin(), nombreFile.end());

            // Calcular hash para el nombre del archivo
            vector<uint8_t> entradaNombre(nombreUTF8.begin(), nombreUTF8.end());
            string Nombreh = md5(entradaNombre);

            // Calcular hash para el tamanio del archivo
            vector<uint8_t> entradaTamano(8); // Crear un vector de 8 bytes (64 bits) 
            for (size_t i = 0; i < 8; ++i) {

                entradaTamano[i] = (tamanioFile >> (i * 8)) & 0xFF; // Extraer byte por byte 
            }

            string Tamanioh = md5(entradaTamano);


            // Agregar los hashes, nombre, ubicacion y tamanio a la lista
            lista.agregar(Nombreh, Tamanioh, nombreFile, directorio, tamanioFile);
        }

    }

    while (FindNextFileW(hFind, &datosFile) != 0);

    FindClose(hFind);
}

int main() {

    wstring rutaInicial;
    wcout << L"Ingrese la ubicacion de la carpeta a hashear: ";
    getline(wcin, rutaInicial);

    ListaEnlazada listaHashes;

    listar_y_hashear(rutaInicial, listaHashes);
   
    listaHashes.mostrar();

    listaHashes.mostrarDuplicados();

    listaHashes.eliminarDuplicados();

    return 0;
}
