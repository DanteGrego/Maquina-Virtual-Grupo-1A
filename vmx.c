#include <stdio.h>
#include "mv.h"
#include <string.h>

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

void JN(Tmv mv, int direccion){
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

void impNombreOperando(Tmv mv,int ip,int tipo){
    int num,low,high;

    switch (tipo)
    {
    case 1: { //registro
        printf("%s",nombreRegistros[mv.memoria[++ip]]);
        break;
    }

    case 2: { //inmediato
        high = (unsigned char) mv.memoria[++ip];
        low = (unsigned char) mv.memoria[++ip];
        num = (high<<4) | low;
        num <<= 16;
        num >>= 16;
        printf("%d ",num);
    }

    case 3: {
        //offset
        printf("[");
        printf("%s",nombreRegistros[mv.memoria[++ip]]);

        high = (unsigned char)  mv.memoria[++ip];
        low = (unsigned char) mv.memoria[++ip];
        num = (high<<4) | low;
        num <<= 16;
        num >>= 16;

        if(num > 0)
            printf("+ %d",num);
        else if(num < 0)
            printf("%d",num);

        printf("]");
        break;
    }

    }
}

void disassembler(Tmv mv){
    int base = obtenerHigh(mv.tablaSegmentos[0]);
    int ip = 0;
    int ipaux;
    int j;
    int tam = obtenerLow(mv.tablaSegmentos[0]);
    char *nom1,*nom2;
    unsigned char opc,top1,top2,op1,op2,ins,aux;

    do{
        printf("[%04x] ",base + ip);
        ins = mv.memoria[ip];
        printf("%02x ",ins);

        ipaux = ip + 1;
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
            printf("| %s ",mnemonicos[opc]);
            impNombreOperando(mv,ip,top1);
        }
        else if(opc >= 0x10 && opc <= 0x1F){ // 2 operandos
            top2 = (ins >> 6) & (0x03);
            top1 = (ins >> 4) & (0x03);
            aux = top1 + top2;

            for(j = 0; j < aux; j++){
                printf("%02x ", (unsigned char) mv.memoria[++ip]);
            }


            printf("| %s ",mnemonicos[opc]);
            impNombreOperando(mv,ipaux+top2,top1); //imprime op1
            printf(",");
            impNombreOperando(mv,ipaux,top2); //imprime op2

        }
        else{
            printf("Operando invalido");
            opc = 0x0F;
        }

    }while ((ip <= tam) && (opc != 0x0F));
}