#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include "Md5hash.h"

struct ArchivoHasheado {
    std::wstring nombre;
    std::string hash;
};

// Función para listar los archivos y calcular sus hashes recursivamente
void listar_y_hashear(const std::wstring& directorio, std::vector<ArchivoHasheado>& resultado, int nivel = 0) {
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
            listar_y_hashear(directorio + L"\\" + nombreArchivo, resultado, nivel + 1);
        }
        else {
            // Convertir el nombre del archivo a una cadena estándar
            std::string nombreUTF8(nombreArchivo.begin(), nombreArchivo.end());

            // Calcular el hash MD5 del nombre del archivo
            std::vector<uint8_t> entrada(nombreUTF8.begin(), nombreUTF8.end());
            std::string hash = md5(entrada);

            // Agregar el nombre y el hash a la lista de resultados
            resultado.push_back({ nombreArchivo, hash });
        }

    } while (FindNextFileW(hFind, &datosArchivo) != 0);

    FindClose(hFind);
}

int main() {
    std::wstring rutaInicial;
    std::wcout << L"Ingrese la ubicación de la carpeta a hashear: ";
    std::getline(std::wcin, rutaInicial);

    std::vector<ArchivoHasheado> listaHashes;

    // Llamar a la función para listar y calcular hashes
    listar_y_hashear(rutaInicial, listaHashes);

    // Mostrar resultados
    std::wcout << L"\nResultados del hash:\n";
    for (const auto& archivo : listaHashes) {
        std::wcout << L"Archivo: " << archivo.nombre << L" | MD5: " << archivo.hash.c_str() << L'\n';
    }

    return 0;
}
