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


void creaMnemonicos() {
    strcpy(mnemonicos[0x00], "SYS");
    strcpy(mnemonicos[0x01], "JMP");
    strcpy(mnemonicos[0x02], "JZ");
    strcpy(mnemonicos[0x03], "JP");
    strcpy(mnemonicos[0x04], "JN");
    strcpy(mnemonicos[0x05], "JNZ");
    strcpy(mnemonicos[0x06], "JNP");
    strcpy(mnemonicos[0x07], "JNN");
    strcpy(mnemonicos[0x08], "NOT");
    strcpy(mnemonicos[0x0F], "STOP");

    strcpy(mnemonicos[0x10], "MOV");
    strcpy(mnemonicos[0x11], "ADD");
    strcpy(mnemonicos[0x12], "SUB");
    strcpy(mnemonicos[0x13], "MUL");
    strcpy(mnemonicos[0x14], "DIV");
    strcpy(mnemonicos[0x15], "CMP");
    strcpy(mnemonicos[0x16], "SHL");
    strcpy(mnemonicos[0x17], "SHR");
    strcpy(mnemonicos[0x18], "SAR");
    strcpy(mnemonicos[0x19], "AND");
    strcpy(mnemonicos[0x1A], "OR");
    strcpy(mnemonicos[0x1B], "XOR");
    strcpy(mnemonicos[0x1C], "SWAP");
    strcpy(mnemonicos[0x1D], "LDL");
    strcpy(mnemonicos[0x1E], "LDH");
    strcpy(mnemonicos[0x1F], "RND");
}

char* getNombreOperando(Tmv mv, int ip, int bytes) // sin testear/incompleto
{
    char* valor;
    switch (bytes)
    {
    case 0:
    { // nulo
        break;
    }

    case 1:
    { // registro
        valor = mv.memoria[++ip];
        break;
    }

    case 2:
    { // inmediato
        unsigned char high = mv.memoria[++ip];
        unsigned char low  = mv.memoria[++ip];
        valor = (high << 8) | low;
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


void disassembler(Tmv mv){
    int base = obtenerHigh(mv.tablaSegmentos[0]);
    int ip = 0;
    int j;
    int tam = obtenerLow(mv.tablaSegmentos[0]);
    char *nom1,*nom2;
    unsigned char opc,top1,top2,ins,aux;

    do{
        printf("[%04x] ",base + ip);
        ins = mv.memoria[ip];
        printf("%02x ",ins);

        opc = (ins & 0x1F);
        if(opc == 0x0F){
            printf("| STOP");
            
        }
        else if(opc >= 0x00 && opc <= 0x08){ // 1 operando
            top1 = (ins >> 6) & (0x03);
            for (j = 0; j < top1; j++){
                printf("%02x ", (unsigned char) mv.memoria[++ip]);
            }
            ip -= top1;
            printf("| %s %s",mnemonicos[opc]);
        }
        else if(opc >= 0x10 && opc <= 0x1F){ // 2 operandos
            top2 = (ins >> 6) & (0x03);
            top1 = (ins >> 4) & (0x03);
            aux = top1 + top2;

            for(j = 0; j < aux; j++){
                printf("%02x ", (unsigned char) mv.memoria[++ip]);
            }
            ip -= aux;
            //strcpy(&nom1));
            //strcpy(&nom2, getNombreOperando());
            printf("| %s %s, %s",mnemonicos[opc]);

        }
        else{
            printf("Operando invalido");
            opc = 0x0F;
        }    

    }while ((ip <= tam) && (opc != 0x0F));
}