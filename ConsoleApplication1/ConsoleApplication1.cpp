#define NOMINMAX
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <windows.h>
#include "Md5hash.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <map>
using namespace std;

// Función para normalizar el nombre del archivo
wstring normalizarNombre(const wstring& nombre) {
    wstring nombreBase = nombre;

    // Obtener la extensión
    size_t ultimoPunto = nombreBase.find_last_of(L'.');
    wstring extension = L"";
    if (ultimoPunto != wstring::npos) {
        extension = nombreBase.substr(ultimoPunto);
        nombreBase = nombreBase.substr(0, ultimoPunto);
    }

    // Lista de patrones a eliminar
    vector<wstring> patrones = {
        L" \\(\\d+\\)",     // (1), (2), etc.
        L"\\(\\d+\\)",      // (1), (2) sin espacio
        L" - copia",        // - copia
        L"-copia",          // -copia sin espacio
        L" \\(copia\\)",    // (copia)
        L"\\(copia\\)",     // (copia) sin espacio
        L" \\(ver \\d+\\)", // (ver 1), (ver 2), etc.
        L"\\(ver \\d+\\)",  // (ver 1) sin espacio
        L" - Copy",         // - Copy
        L"-Copy",           // -Copy sin espacio
        L" Copy",           // Copy
        L" copia",          // copia
        L"_copy",           // _copy
        L"_copia"           // _copia
    };

    // Eliminar cada patrón
    for (const auto& patron : patrones) {
        wregex expresion(patron + L"$"); // $ asegura que el patrón está al final
        nombreBase = regex_replace(nombreBase, expresion, L"");
    }

    return nombreBase + extension;
}

struct Nodo {
    string Nombreh;
    string Tamanioh;
    wstring NombreA;
    wstring NombreOriginal; // Guardamos también el nombre normalizado
    wstring ubicacion;
    uint64_t tamanio;
    bool esDuplicado;
    Nodo* nodo;

    Nodo(string nh, string th, wstring na, wstring no, wstring ub, uint64_t tam, Nodo* siguiente) :
        Nombreh(nh), Tamanioh(th), NombreA(na), NombreOriginal(no), ubicacion(ub),
        tamanio(tam), esDuplicado(false), nodo(siguiente) {}
};

class ListaEnlazada {
private:
    Nodo* cabeza;
    map<string, vector<Nodo*>> gruposPorTamanio;

public:
    ListaEnlazada() : cabeza(nullptr) {}

    void agregar(const wstring& nombreArchivo, const wstring& ubicacion, uint64_t tamanio) {
        // Normalizar el nombre
        wstring nombreNormalizado = normalizarNombre(nombreArchivo);

        // Convertir el nombre normalizado a UTF-8 para el hash
        string nombreUTF8(nombreNormalizado.begin(), nombreNormalizado.end());

        // Calcular hash del nombre normalizado
        vector<uint8_t> entradaNombre(nombreUTF8.begin(), nombreUTF8.end());
        string Nombreh = md5(entradaNombre);

        // Calcular hash del tamaño
        vector<uint8_t> entradaTamano(8);
        for (size_t i = 0; i < 8; ++i) {
            entradaTamano[i] = (tamanio >> (i * 8)) & 0xFF;
        }
        string Tamanioh = md5(entradaTamano);

        // Crear nuevo nodo
        Nodo* nuevo = new Nodo(Nombreh, Tamanioh, nombreArchivo, nombreNormalizado,
            ubicacion, tamanio, cabeza);
        cabeza = nuevo;

        // Verificar duplicados
        gruposPorTamanio[Tamanioh].push_back(nuevo);

        // Si hay más de un archivo en el grupo, verificar duplicados
        if (gruposPorTamanio[Tamanioh].size() > 1) {
            vector<Nodo*>& grupo = gruposPorTamanio[Tamanioh];

            // Comparar el nuevo archivo con todos los del mismo tamaño
            for (size_t i = 0; i < grupo.size() - 1; i++) {
                // Si tienen el mismo hash de nombre normalizado
                if (grupo[i]->Nombreh == nuevo->Nombreh) {
                    grupo[i]->esDuplicado = true;
                    nuevo->esDuplicado = true;
                }
            }
        }
    }

    void mostrar() {
        Nodo* actual = cabeza;
        cout << "\nLista de archivos:\n";

        while (actual) {
            wcout << L"\nNombre Original: " << actual->NombreA
                << L"\nNombre Normalizado: " << actual->NombreOriginal
                << L"\nUbicacion: " << actual->ubicacion
                << L"\nTamano: " << actual->tamanio << L" bytes"
                << L"\nHash Nombre Normalizado: " << actual->Nombreh.c_str()
                << L"\nHash Tamano: " << actual->Tamanioh.c_str();

            if (actual->esDuplicado) {
                wcout << L"\n** ARCHIVO DUPLICADO **";
            }
            wcout << L"\n----------------------------------------\n";
            actual = actual->nodo;
        }
    }

    void mostrarDuplicados() {
        cout << "\nArchivos duplicados encontrados:\n";
        bool hayDuplicados = false;

        for (const auto& grupo : gruposPorTamanio) {
            const vector<Nodo*>& archivos = grupo.second;

            bool grupoDuplicados = false;
            for (const Nodo* archivo : archivos) {
                if (archivo->esDuplicado) {
                    if (!grupoDuplicados) {
                        cout << "\nGrupo de archivos duplicados:\n";
                        grupoDuplicados = true;
                        hayDuplicados = true;
                    }
                    wcout << L"\nNombre Original: " << archivo->NombreA
                        << L"\nNombre Normalizado: " << archivo->NombreOriginal
                        << L"\nUbicacion: " << archivo->ubicacion
                        << L"\nTamano: " << archivo->tamanio << L" bytes"
                        << L"\n----------------------------------------\n";
                }
            }
        }

        if (!hayDuplicados) {
            cout << "No se encontraron archivos duplicados.\n";
        }
    }

    ~ListaEnlazada() {
        while (cabeza) {
            Nodo* temp = cabeza;
            cabeza = cabeza->nodo;
            delete temp;
        }
    }
};

// La función getTamaniofile permanece igual
uint64_t getTamaniofile(const WIN32_FIND_DATAW& datosArchivo) {
    LARGE_INTEGER tamanio;
    tamanio.HighPart = datosArchivo.nFileSizeHigh;
    tamanio.LowPart = datosArchivo.nFileSizeLow;
    return static_cast<uint64_t>(tamanio.QuadPart);
}

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

        if (nombreFile == L"." || nombreFile == L"..") {
            continue;
        }

        if (datosFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            listar_y_hashear(directorio + L"\\" + nombreFile, lista);
        }
        else {
            uint64_t tamanioFile = getTamaniofile(datosFile);
            lista.agregar(nombreFile, directorio, tamanioFile);
        }
    } while (FindNextFileW(hFind, &datosFile) != 0);

    FindClose(hFind);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    while (true) {
        wstring rutaInicial;
        wcout << L"\nIngrese la ubicacion de la carpeta a hashear (o escriba 'salir' para terminar): ";
        getline(wcin, rutaInicial);

        if (rutaInicial == L"salir") {
            wcout << L"Programa finalizado.\n";
            break;
        }

        ListaEnlazada listaHashes;
        wcout << L"Procesando la carpeta: " << rutaInicial << L"...\n";
        listar_y_hashear(rutaInicial, listaHashes);

        listaHashes.mostrar();
        listaHashes.mostrarDuplicados();
    }

    return 0;
}