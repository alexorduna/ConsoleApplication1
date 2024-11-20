#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include "Md5hash.h"

// Estructura para representar un nodo en la lista enlazada
struct Nodo {
    std::string hash;
    std::wstring nombreOriginal; // Nombre del archivo original
    std::wstring ubicacion;      // Ubicación del archivo
    bool repetido;               // Indicador para hashes repetidos
    Nodo* siguiente;
};

// Clase ListaEnlazada para manejar los nodos y detectar duplicados
class ListaEnlazada {
private:
    Nodo* cabeza;

public:
    ListaEnlazada() : cabeza(nullptr) {}

    // Agregar un nuevo hash a la lista
    void agregar(const std::string& hash, const std::wstring& nombre, const std::wstring& ubicacion) {
        bool repetido = false;
        Nodo* actual = cabeza;

        // Verificar si el hash ya existe
        while (actual) {
            if (actual->hash == hash) {
                repetido = true;
                break;
            }
            actual = actual->siguiente;
        }

        // Agregar el nuevo nodo con la información
        Nodo* nuevo = new Nodo{ hash, nombre, ubicacion, repetido, cabeza };
        cabeza = nuevo;
    }

    // Mostrar los hashes en la lista sin los duplicados, solo mostrando el nombre y ubicación si es duplicado
    void mostrar() {
        Nodo* actual = cabeza;
        std::cout << "\nLista de hashes MD5:\n";
        while (actual) {
            if (actual->repetido) {
                // Solo mostramos los duplicados con nombre y ubicación
                std::wcout << L"Archivo: " << actual->nombreOriginal
                    << L" | Ubicacion: " << actual->ubicacion
                    << L" | Hash: " << actual->hash.c_str() << L" (REPETIDO)" << std::endl;
            }
            else {
                // Si no es duplicado, solo mostramos el hash
                std::cout << "Hash: " << actual->hash.c_str() << std::endl;
            }
            actual = actual->siguiente;
        }
    }

    // Mostrar el resumen de archivos duplicados (solo los nombres)
    void mostrarDuplicados() {
        Nodo* actual = cabeza;
        std::cout << "\nResumen de archivos duplicados:\n";
        bool hayDuplicados = false;
        while (actual) {
            if (actual->repetido) {
                hayDuplicados = true;
                std::wcout << L"Archivo duplicado: " << actual->nombreOriginal
                    << L" | Ubicacion: " << actual->ubicacion
                    << L" | Hash: " << actual->hash.c_str() << L"\n";
            }
            actual = actual->siguiente;
        }
        if (!hayDuplicados) {
            std::cout << "No se encontraron archivos duplicados.\n";
        }
    }

    // Liberar memoria al destruir la lista
    ~ListaEnlazada() {
        while (cabeza) {
            Nodo* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
        }
    }
};

// Función para listar archivos, calcular hashes y detectar duplicados
void listar_y_hashear(const std::wstring& directorio, ListaEnlazada& lista) {
    std::wstring rutaBusqueda = directorio + L"\\*";

    WIN32_FIND_DATAW datosArchivo;
    HANDLE hFind = FindFirstFileW(rutaBusqueda.c_str(), &datosArchivo);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Error al abrir el directorio: " << directorio << L'\n';
        return;
    }

    do {
        const std::wstring nombreArchivo = datosArchivo.cFileName;

        // Ignorar directorios "." y ".."
        if (nombreArchivo == L"." || nombreArchivo == L"..") {
            continue;
        }

        // Comprobar si es un directorio
        if (datosArchivo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            listar_y_hashear(directorio + L"\\" + nombreArchivo, lista);
        }
        else {
            // Convertir el nombre del archivo a una cadena estándar
            std::string nombreUTF8(nombreArchivo.begin(), nombreArchivo.end());

            // Calcular el hash MD5 del nombre del archivo
            std::vector<uint8_t> entrada(nombreUTF8.begin(), nombreUTF8.end());
            std::string hash = md5(entrada);

            // Agregar el hash, nombre y ubicación a la lista
            lista.agregar(hash, nombreArchivo, directorio);
        }

    } while (FindNextFileW(hFind, &datosArchivo) != 0);

    FindClose(hFind);
}

int main() {
    std::wstring rutaInicial;
    std::wcout << L"Ingrese la ubicacion de la carpeta a hashear: ";
    std::getline(std::wcin, rutaInicial);

    ListaEnlazada listaHashes;

    // Llamar a la función para listar y calcular hashes
    listar_y_hashear(rutaInicial, listaHashes);

    // Mostrar la lista de hashes sin los duplicados, y mostrar solo nombre y ubicación cuando sean duplicados
    listaHashes.mostrar();

    // Mostrar el resumen de duplicados (solo los nombres)
    listaHashes.mostrarDuplicados();

    return 0;
}
