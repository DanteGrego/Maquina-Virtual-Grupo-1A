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
        strcpy(fileName, vectorArgumentos[1]);
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

void leerArch(Tmv mv, char *nomArch)
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

            i = obtenerHigh(mv.tablaSegmentos[0]);

            while (fread(&x, sizeof(char), 1, arch) != 0)
            {
                mv.memoria[i] = x;
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

void cargarTablaSegmentos(Tmv mv, int tamCodigo)
{
    mv.tablaSegmentos[0] = combinarHighLow(0, tamCodigo);
    mv.tablaSegmentos[1] = combinarHighLow(tamCodigo, TAM_MEMORIA - tamCodigo);
}

void inicializarRegistros(Tmv mv)
{
    mv.registros[CS] = 0x00000000;   // 0x0000 0000
    mv.registros[DS] = 0x00010000; // 0x0001 0000
    mv.registros[IP] = mv.registros[CS];
}

int leerValOperando(Tmv mv, int top, int posOp){
    int op = 0;

    if(top > 0){
        op = mv.memoria[posOp];
        op <<= 24;
        op >>= 24;//escopeta goes brr

        top--;
        for(int i = 0; i < top; i++){
            op <<= 8;
            op |= mv.memoria[posOp + 1];
        }
    }

    return op;
}

void leerInstruccion(Tmv mv){
    int posFisInstruccion = obtenerDirFisica(mv, mv.registros[IP]);
    char instruccion = mv.memoria[posFisInstruccion];
    char top2 = (instruccion >> 6) & 0x03;
    char top1 = (instruccion >> 4) & 0x03;
    char opc = instruccion & 0x1F;
    
    posFisInstruccion++;//me pongo en posicion para leer op2
    int valOp2 = leerValOperando(mv, top2, posFisInstruccion);
    posFisInstruccion += top2;//me pongo en posicion para leer op1
    int valOp1 = leerValOperando(mv, top1, posFisInstruccion);
    

    if(top1 == 0){
        top1 = top2;
        valOp1 = valOp2;
        top2 = valOp2 = 0;//TODO preguntar si cuando hay un solo parametro op2 tiene que ser 0 o no
    }

    mv.registros[OPC] = opc;
    mv.registros[OP1] = ((int)top1 << 24) | (valOp1 & 0x00FFFFFF);//maskeado por si era negativo, sino me tapa el top en el primer byte
    mv.registros[OP2] = ((int)top2 << 24) | (valOp2 & 0x00FFFFFF);
    mv.registros[IP] += 1 + top1 + top2;
}

char obtengoTipoOperando(int bytes) // sin testear
{
    bytes &= 0xC0000000;
    bytes >>= 30;
    return bytes;
}

int getValor(Tmv mv, int bytes) // sin testear/incompleto
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
        valor = mv.registros[bytes];
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
        leerMemoria(mv, obtenerDirLogica(mv, bytes));
        valor = mv.registros[MBR];
        break;
    }
    }

    return valor;
}
int setValor(Tmv mv, int bytes, int valor) // sin testear/incompleto
{
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
        valor = mv.registros[bytes];
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
        leerMemoria(mv, obtenerDirLogica(mv, bytes));
        valor = mv.registros[MBR];
        break;
    }
    }

    return valor;
}
int setValor(Tmv mv, int bytes, int valor) // sin testear/incompleto
{
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
        valor = mv.registros[bytes];
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
        leerMemoria(mv, bytes);
        valor = mv.registros[MBR];
        break;
    }
    }

    return valor;
}


char* getMnemonic(int code) {
    switch (code) {
        case 0x00: return "SYS";
        case 0x01: return "JMP";
        case 0x02: return "JZ";
        case 0x03: return "JP";
        case 0x04: return "JN";
        case 0x05: return "JNZ";
        case 0x06: return "JNP";
        case 0x07: return "JNN";
        case 0x08: return "NOT";
        case 0x0F: return "STOP";

        case 0x10: return "MOV";
        case 0x11: return "ADD";
        case 0x12: return "SUB";
        case 0x13: return "MUL";
        case 0x14: return "DIV";
        case 0x15: return "CMP";
        case 0x16: return "SHL";
        case 0x17: return "SHR";
        case 0x18: return "SAR";
        case 0x19: return "AND";
        case 0x1A: return "OR";
        case 0x1B: return "XOR";
        case 0x1C: return "SWAP";
        case 0x1D: return "LDL";
        case 0x1E: return "LDH";
        case 0x1F: return "RND";

        default: return "UNKNOWN";
    }
}

void disassembler(Tmv mv){
    int aux = obtenerHigh(mv.tablaSegmentos[0]);
    int i = aux;
    int j;
    int tam = obtenerLow(mv.tablaSegmentos[0]);
    char *nombre;
    char opc,top1,top2,ins;

    while(i <= tam){
        printf("[%x] ",aux + i);
        ins = mv.memoria[i];
        printf("%02x ",ins);

        opc = (ins & 0x1F);
        if (opc == 0x0F)
        {
            printf("| STOP");
            
        }
        else if(opc >= 0x00 && opc <= 0x08){ // 1 operando
            top1 = (opc >> 6) & (0x03);
            for (j = 0; j < top1; j++){

            }
            ip -= top1;
            printf("| %s %s",mnemonicos[opc]);
        }
        else if(opc >= 0x10 && opc <= 0x1F){ // 2 operandos
            top2 = (opc >> 6) & (0x03);
            top1 = (opc >> 4) & (0x03);
        }
        else{
            printf("Operando invalido");
    }
}

int obtenerDirFisica(Tmv mv, int dirLogica)
{
    int segmento = obtenerHigh(dirLogica);
    int offset = obtenerLow(dirLogica);

    int base = obtenerHigh(mv.tablaSegmentos[segmento]);
    return base + offset;
}

void leerMemoria(Tmv mv, int valor)
{
    char codRegistro = (valor & 0x001F0000) >> 24; //indice para vector de registros
    int offsetOp = obtenerLow(valor);

    int valRegistro = mv.registros[codRegistro];
    int baseDS = obtenerDirFisica(mv, DS);
    int tamDS = obtenerLow(mv.tablaSegmentos[mv.registros[DS]]);

    // la direccion logica del LAR es la direccion logica del registro + el offset del operando
    mv.registros[LAR] = combinarHighLow(obtenerHigh(valRegistro), obtenerLow(valRegistro) + offsetOp);
    int offsetFisico = obtenerDirFisica(mv, LAR);

    if (offsetFisico >= baseDS && offsetFisico < baseDS + tamDS)
    {
        mv.registros[MAR] = combinarHighLow(4, offsetFisico);
        mv.registros[MBR] = mv.memoria[offsetFisico];
    }
    else
    {
        printf("Error: Desbordamiento de segmento\n");
        exit(-1);
    }
}
