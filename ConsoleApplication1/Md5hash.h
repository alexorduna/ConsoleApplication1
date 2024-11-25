#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>  



// Constantes de rotacion y la tabla K para las 64 rondas de MD5
const uint32_t s[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

const uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// Variables de estado A, B, C, D usadas en el procesamiento de los bloques
uint32_t A, B, C, D;

/*Funcion que realiza la rotacion a la izquierda de x, n veces.
Es utilizada para los desplazamientos durante el proceso de cada bloque*/
uint32_t RotarIzq(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

/* Funcion que aplica el padding al mensaje original para asegurarse de que
su longitud sea un multiplo de 512 bits. Agrega un bit "1" seguido de ceros
y luego el tamaño original del mensaje.*/
std::vector<uint8_t> padMensaje(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> padded = input;
    padded.push_back(0x80); // Anadir bit "1"

    // Rellenar con ceros hasta alcanzar 448 bits (o 56 bytes)
    while ((padded.size() * 8) % 512 != 448) {
        padded.push_back(0);
    }

    // Convertir la longitud del mensaje original a bits y agregarla en 64 bits
    uint64_t LargoBits = input.size() * 8;
    for (int i = 0; i < 8; i++) {
        padded.push_back(LargoBits >> (8 * i));
    }

    return padded;
}

/* Procesa un bloque de 512 bits a traves de varias rondas de operaciones
 bit a bit, aplicando diferentes funcio nes de mezcla */
void Bloque(const uint8_t* bloque) {

    // Asignamos las variables temporales a los valores actuales de A, B, C y D
    uint32_t a = A, b = B, c = C, d = D;

    // Convertimos los 64 bytes del bloque a 16 enteros de 32 bits
    uint32_t M[16];
    for (int i = 0; i < 16; i++) {
        M[i] = (bloque[i * 4]) | (bloque[i * 4 + 1] << 8) | (bloque[i * 4 + 2] << 16) | (bloque[i * 4 + 3] << 24);
    }

    // Realizamos 64 operaciones divididas en 4 grupos de 16
    for (int i = 0; i < 64; i++) {
        uint32_t F, g;
        if (i < 16) {
            F = (b & c) | (~b & d);
            g = i;
        }
        else if (i < 32) {
            F = (d & b) | (~d & c);                         //TRANSFORMACION Y COMBINACION
            g = (5 * i + 1) % 16;
        }
        else if (i < 48) {
            F = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        }
        else {
            F = c ^ (b | ~d);
            g = (7 * i) % 16;
        }

        // Actualizamos los valores de a, b, c y d
        F = F + a + K[i] + M[g]; //perdida de datos solo si sobrepasa el tamaño de la variable
        a = d;
        d = c;
        c = b;
        b = b + RotarIzq(F, s[i]);
    }

    // Sumamos los resultados de las rondas a las variables globales
    A += a;
    B += b;
    C += c;
    D += d; // se genera un desbordamiento de bits por lo tanto tambien hay perdida
}

// Funcion principal que aplica MD5 a un mensaje de entrada y retorna el hash final
std::string md5(const std::vector<uint8_t>& input) {

    //Inicializar las variables de estado
    A = 0x67452301;
    B = 0xefcdab89;
    C = 0x98badcfe;
    D = 0x10325476;

    std::vector<uint8_t> paddedMensaje = padMensaje(input);

    /* Bucle for que procesa el mensaje relleno en bloques de 512 bits(64 bytes).
    Se recorre el mensaje en incrementos de 64 bytes para procesar cada bloque. */
    for (size_t i = 0; i < paddedMensaje.size(); i += 64) {
        Bloque(&paddedMensaje[i]);
    }

    std::ostringstream resultado;
    /* Configuramos el flujo para que convierta los valores en formato hexadecimal
     y que llene con ceros (0) a la izquierda cuando sea necesario. */
    resultado << std::hex << std::setfill('0');

    // Este bucle recorre las variables de estado A, B, C y D que contienen el resultado final del hash.
    for (auto valor : { A, B, C, D }) {
        resultado << std::setw(8) << valor;
    }

    return resultado.str();
}