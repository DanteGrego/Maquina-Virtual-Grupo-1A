#include <stdio.h>
#include "mv.h"
#include <string.h>

// TODO una funcion que devuelva los ultimos 5 bits (para el opc)
// dissasembler

int main(int numeroArgumentos, char *vectorArgumentos[])
{
    char *fileName;            // nombre del archivo.vmx
    char imprimoDesensamblado; // condicion booleana que decide mostrar el codigo desensamblado
    if (numeroArgumentos < 2)
    {
        println("Numero insuficiente de argumentos");
        return -1;
    }
    else
    {
        fileName = vectorArgumentos[1];
        if (numeroArgumentos > 2 && strcmp(vectorArgumentos[2], "-d") == 0)
            imprimoDesensamblado = 1;
    }

    return 0;
}

int combinarHighLow(int bytesHigh, int bytesLow)
{
    return (bytesHigh << 16) | (bytesLow & 0x0000FFFF);
}

int obtenerHigh(int bytes)
{
    int res = 0;
    bytes >> 16;
    res = bytes & 0x0000FFFF;
    return res;
}

int obtenerLow(int bytes)
{
    return bytes & 0x0000FFFF;
}

char obtenerOPC(char x)
{
    return x & 0x1F;
}

void leerArch(Tmv *mv, char *nomArch)
{
    char x;
    int i;
    char cabecera[5], version, tamCodigo[2];
    FILE *arch;
    arch = fopen(nomArch, "rb");

    if (arch != NULL)
    {
        fread(cabecera, sizeof(char), TAM_IDENTIFICADOR, arch);

        if (strcmp(cabecera, "VMX25") == 0)
        {
            fread(&version, sizeof(char), 1, arch);
            fread(tamCodigo, sizeof(char), 2, arch);

            cargarTablaSegmentos(mv, tamCodigo[0] + tamCodigo[1] * 256);

            i = obtenerHigh(mv->tablaSegmentos[0]);

            while (fread(&x, sizeof(char), 1, arch) != 0)
            {
                mv->memoria[i] = x;
                i++;
            }
        }
        else
            printf("Error: El archivo no es valido\n");

        fclose(arch);
    }
    else
    {
        printf("Error: El archivo no pudo abrirse\n");
        exit(-1);
    }
}

void cargarTablaSegmentos(Tmv *mv, int tamCodigo)
{
    mv->tablaSegmentos[0] = combinarHighLow(0, tamCodigo);
    mv->tablaSegmentos[1] = combinarHighLow(tamCodigo, TAM_MEMORIA - tamCodigo);
}

void inicializarRegistros(Tmv *mv)
{
    mv->registros[CS] = 0;   // 0x0000 0000
    mv->registros[DS] = 256; // 0x0001 0000
    mv->registros[IP] = mv->registros[CS];
}
char obtengoTipoOperando(int bytes) // sin testear
{
    bytes &= 0xC0000000;
    bytes >>= 30;
    return bytes;
}
int getValor(Tmv *mv, int bytes) // sin testear/incompleto
{
    int valor = 0;
    char tipoOperando = obtengoTipoOperando(bytes);
    switch (tipoOperando)
    {
    case 0:
    { // nulo
        break;
    }

    case 1:
    { // registro
        bytes &= 0x000000FF;
        valor = mv->registros[bytes];
        break;
    }

    case 2:
    { // inmediato
        bytes &= 0x0000FFFF;
        valor = bytes;
        break;
    }

    case 3:
    { // memoria
        bytes &= 0x00FFFFFF;
        break;
    }
    }

    return valor;
}
