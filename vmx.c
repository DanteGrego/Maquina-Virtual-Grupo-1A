#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mv.h"

// TODO una funcion que devuelva los ultimos 5 bits (para el opc)
// dissasembler

int main(int numeroArgumentos, char *vectorArgumentos[])
{
    char *fileName;            // nombre del archivo.vmx
    char imprimoDesensamblado; // condicion booleana que decide mostrar el codigo desensamblado
    Tmv mv;
    if (numeroArgumentos < 2)
    {
        printf("Numero insuficiente de argumentos");
        return -1;
    }
    else
    {
        fileName = vectorArgumentos[1];

        if (numeroArgumentos > 2 && strcmp(vectorArgumentos[2], "-d") == 0)
            imprimoDesensamblado = 1;

        leerArch(&mv, fileName);
        //if(imprimoDesensamblado)
         //   disassembler(mv);

        inicializarRegistros(&mv);
        while(seguirEjecutando(&mv)){
            leerInstruccion(&mv);
        }
    }

    return 0;
}

int seguirEjecutando(Tmv* mv){
    int tabla = mv->tablaSegmentos[obtenerHigh(mv->registros[IP])];
    int baseCS = obtenerHigh(tabla);
    int tamCS = obtenerLow(tabla);
    int dirFisicaIp = obtenerDirFisica(mv, mv->registros[IP]);
    dirFisicaIp -= baseCS;
    return dirFisicaIp >= 0 && dirFisicaIp <= tamCS;

}

int combinarHighLow(int bytesHigh, int bytesLow)
{
    printf("high: %d\nlow: %d\n", bytesHigh, bytesLow);
    return (bytesHigh << 16) | (bytesLow & 0x0000FFFF);
}

int obtenerHigh(int bytes)
{
    int res = 0;
    bytes >>= 16;
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

            cargarTablaSegmentos(mv, tamCodigo[1] + tamCodigo[0] * 256);

            i = obtenerHigh(mv->tablaSegmentos[0]);

            while (fread(&x, sizeof(char), 1, arch) != 0)
            {
                mv->memoria[i] = x;
                i++;
            }

            fclose(arch);
        }
        else{
            printf("Error: El archivo no es valido\n");
            fclose(arch);
            exit(-1);
        }
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
    printf("tabla0: %d, %d\n", obtenerHigh(mv->tablaSegmentos[0]), obtenerLow(mv->tablaSegmentos[0]));
    printf("tabla1: %d, %d\n", obtenerHigh(mv->tablaSegmentos[1]), obtenerLow(mv->tablaSegmentos[1]));
}

void inicializarRegistros(Tmv* mv)
{
    mv->registros[CS] = 0x00000000; // 0x0000 0000
    mv->registros[DS] = 0x00010000; // 0x0001 0000
    mv->registros[IP] = mv->registros[CS];
}

int leerValOperando(Tmv *mv, int top, int posOp)
{
    int op = 0;

    if (top > 0)
    {
        op = mv->memoria[posOp];
        op <<= 24;
        op >>= 24; // escopeta goes brr

        top--;
        for (int i = 0; i < top; i++)
        {
            op <<= 8;
            op |= mv->memoria[posOp + 1];
        }
    }

    return op;
}

void imprimirReg(char* s, int byte){
    printf("%s: %x, %x\n", s, obtenerHigh(byte), obtenerLow(byte));
}

void leerInstruccion(Tmv* mv)
{
    int posFisInstruccion = obtenerDirFisica(mv, mv->registros[IP]);
    imprimirReg("IP", mv->registros[IP]);
    printf("ip fisico: %d\n", posFisInstruccion);
    char instruccion = mv->memoria[posFisInstruccion];
    printf("ins: %x\n", (unsigned char)instruccion);
    char top2 = (instruccion >> 6) & 0x03;
    char top1 = (instruccion >> 4) & 0x03;
    char opc = instruccion & 0x1F;
    printf("top2: %x, top1: %x, opc: %x\n", top2, top1, opc);

    posFisInstruccion++; // me pongo en posicion para leer op2
    int valOp2 = leerValOperando(mv, top2, posFisInstruccion);
    posFisInstruccion += top2; // me pongo en posicion para leer op1
    int valOp1 = leerValOperando(mv, top1, posFisInstruccion);

    if (top1 == 0)
    {
        top1 = top2;
        valOp1 = valOp2;
        top2 = valOp2 = 0; // TODO preguntar si cuando hay un solo parametro op2 tiene que ser 0 o no
    }

    mv->registros[OPC] = opc;
    mv->registros[OP1] = ((int)top1 << 24) | (valOp1 & 0x00FFFFFF); // maskeado por si era negativo, sino me tapa el top en el primer byte
    mv->registros[OP2] = ((int)top2 << 24) | (valOp2 & 0x00FFFFFF);
    mv->registros[IP] += 1 + top1 + top2;
    imprimirReg("OPC", mv->registros[OPC]);
    imprimirReg("OP1", mv->registros[OP1]);
    imprimirReg("OP2", mv->registros[OP2]);
    imprimirReg("IP", mv->registros[IP]);
    printf("---\n");
}

char obtengoTipoOperando(int bytes) // sin testear
{
    bytes &= 0xC0000000;
    bytes >>= 30;
    return bytes;
}

int getValor(Tmv* mv, int bytes) // sin testear/incompleto
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
        leerMemoria(mv, obtenerDirLogica(mv, bytes));
        valor = mv->registros[MBR];
        break;
    }
    }

    return valor;
}



int obtenerDirFisica(Tmv* mv, int dirLogica)
{
    int segmento = obtenerHigh(dirLogica);
    int offset = obtenerLow(dirLogica);

    int base = obtenerHigh(mv->tablaSegmentos[segmento]);
    return base + offset;
}

void leerMemoria(Tmv* mv, int dirLogica)
{
    int tabla = mv->tablaSegmentos[obtenerHigh(dirLogica)];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);

    mv->registros[LAR] = dirLogica;
    int offsetFisico = obtenerDirFisica(mv, LAR);

    if (offsetFisico >= baseSegmento && offsetFisico < baseSegmento + tamSegmento)
    {
        mv->registros[MAR] = combinarHighLow(CANT_BYTES_A_LEER, offsetFisico);
        mv->registros[MBR] = mv->memoria[offsetFisico];
    }
    else
    {
        printf("Error: Desbordamiento de segmento\n");
        exit(-1);
    }
}

int obtenerDirLogica(Tmv* mv, int valor)
{
    char codRegistro = (valor & 0x001F0000) >> 24;
    int offsetOp = valor & 0x0000FFFF;

    int valRegistro = mv->registros[codRegistro];

    // la direccion logica resultante sera la direccion logica del registro + el offset del operando
    return combinarHighLow(obtenerHigh(valRegistro), obtenerLow(valRegistro) + offsetOp);
}

void actualizarCC(Tmv* mv, int valor){
    mv->registros[CC] &= 0x80000000 & (valor < 0);
    mv->registros[CC] &= 0x40000000 & (valor == 0);
}

void JMP(Tmv mv, int direccion){
    mv.registros[IP] = direccion;
}

void JZ(Tmv mv, int direccion){
    if(mv.registros[CC] >= 0 && (mv.registros[CC] << 1) < 0)
        jmp(mv, direccion);
}

void JNZ(Tmv mv, int direccion){
    if((mv.registros[CC] << 1) >= 0)
        jmp(mv, direccion);
}

void jn(Tmv mv, int direccion){
    if(mv.registros[CC] < 0 && (mv.registros[CC] << 1) >= 0)
        jmp(mv, direccion);
}

void JNN(Tmv mv, int direccion){
    if(mv.registros[CC] >= 0)
        jmp(mv, direccion);
}

void JP(Tmv mv, int direccion){
    if(mv.registros[CC] >= 0 && (mv.registros[CC] << 1) >= 0)
        jmp(mv, direccion);
}

void JNP(Tmv mv, int direccion){
    if(mv.registros[CC] < 0 && (mv.registros[CC] << 1) >= 0 ||
       mv.registros[CC] >= 0 && (mv.registros[CC] << 1) < 0)
        jmp(mv, direccion);
}

void stop(Tmv *mv){
    setValor(mv->registros[IP], -1); 
    exit(0);
}

void not(Tmv *mv,int op1){
    int valor1 = getValor(mv, op1);
    valor1 ^= 0xFFFFFFFF; 
    setValor(mv,op1, valor1);
}

void rnd(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);


}

void impNombreOperando(const Tmv* mv, int ip, int tipo) {
    int num, low, high;
    switch (tipo) {
        case 1: { // registro
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            break;
        }
        case 2: { // inmediato (2 bytes)
            high = (unsigned char) mv->memoria[ip+1];
            low  = (unsigned char) mv->memoria[ip+2];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            printf("%d", num);
            break;
        }
        case 3: { // memoria: [REG + offset]
            printf("[");
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            high = (unsigned char) mv->memoria[ip+2];
            low  = (unsigned char) mv->memoria[ip+3];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            if (num > 0)      printf(" + %d", num);
            else if (num < 0) printf(" %d", num);
            printf("]");
            break;
        }
        default: /* tipo 0/nulo */ break;
    }
}

void disassembler(const Tmv* mv) {
    int base = obtenerHigh(mv->tablaSegmentos[0]); // base seg código
    int tam  = obtenerLow(mv->tablaSegmentos[0]);  // tamaño seg código
    int ip   = 0;

    const int COL_PIPE = 32;

    while (ip < tam) {
        unsigned char ins  = mv->memoria[ip];
        unsigned char opc  = ins & 0x1F;
        unsigned char top2 = (ins >> 6) & 0x03;
        unsigned char top1 = (ins >> 4) & 0x03;


        char left[256];
        int len = snprintf(left, sizeof(left), "[%04x] %02x ", base + ip, ins);

        if (opc == 0x0F) { // STOP
            int spaces = COL_PIPE - len;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| STOP\n", left, spaces, "");
            ip += tam;

        } else if (opc <= 0x08) { // 1 operando
            top1 = top2;
            int pos = len;
            for (int j = 0; j < top1; j++)
                pos += snprintf(left + pos, sizeof(left) - pos, "%02x ", (unsigned char) mv->memoria[ip + 1 + j]);

            int spaces = COL_PIPE - pos;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| %s ", left, spaces, "", mnemonicos[opc]);
            impNombreOperando(mv, ip, top1);
            printf("\n");
            ip += 1 + top1;
        } else if (opc >= 0x10 && opc <= 0x1F) { // 2 operandos
            int aux = top1 + top2;
            int pos = len;
            for (int j = 0; j < aux; j++)
                pos += snprintf(left + pos, sizeof(left) - pos, "%02x ", (unsigned char) mv->memoria[ip + 1 + j]);

            int spaces = COL_PIPE - pos;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| %s ", left, spaces, "", mnemonicos[opc]);
            impNombreOperando(mv, ip + top2, top1);
            printf(", ");
            impNombreOperando(mv, ip, top2);
            printf("\n");
            ip += 1 + aux;
        } else {
            int spaces = COL_PIPE - len;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| UNKNOWN\n", left, spaces, "");
            ip += 1;
        }
    }
}