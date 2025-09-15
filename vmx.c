#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
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
        if(imprimoDesensamblado)
            disassembler(&mv);

        inicializarRegistros(&mv);
        while(seguirEjecutando(&mv)){
            leerInstruccion(&mv);
            ejecutarInstruccion(&mv);
        }
    }

    return 0;
}

int seguirEjecutando(Tmv* mv){
    //obtengo indice de la tabla para el segmento CS
    int tabla = mv->tablaSegmentos[obtenerHigh(mv->registros[CS])];
    //obtengo base y tamano del CS
    int baseCS = obtenerHigh(tabla);
    int tamCS = obtenerLow(tabla);
    //obtengo la direccion fisica de IP
    int dirFisicaIp = obtenerDirFisica(mv, mv->registros[IP]);
    //resto para comparar mas facil
    dirFisicaIp -= baseCS;
    //si IP esta fuera del CS debo parar la ejecucion
    return dirFisicaIp >= 0 && dirFisicaIp < tamCS;
}

//combina los dos bytes de uno en la alta y dos bytes del otro en la baja
int combinarHighLow(int bytesHigh, int bytesLow)
{
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
}

void inicializarRegistros(Tmv *mv)
{
    mv->registros[CS] = 0x00000000; // 0x0000 0000
    mv->registros[DS] = 0x00010000; // 0x0001 0000
    mv->registros[IP] = mv->registros[CS];
}


int leerValMemoria(Tmv *mv, int cantBytes, int posFisica)
{
    int valor = 0;

    if (cantBytes > 0)
    {
        valor = mv->memoria[posFisica];
        valor <<= 24;
        valor >>= 24; // escopeta goes brr

        cantBytes--;
        for (int i = 0; i < cantBytes; i++)
        {
            valor <<= 8;
            valor |= mv->memoria[++posFisica];
        }
    }

    return valor;
}

void leerInstruccion(Tmv *mv)
{
    int posFisInstruccion = obtenerDirFisica(mv, mv->registros[IP]);
    char instruccion = mv->memoria[posFisInstruccion];
    char top2 = (instruccion >> 6) & 0x03;
    char top1 = (instruccion >> 4) & 0x03;
    char opc = instruccion & 0x1F;

    posFisInstruccion++; // me pongo en posicion para leer op2
    int valOp2 = leerValMemoria(mv, top2, posFisInstruccion);
    posFisInstruccion += top2; // me pongo en posicion para leer op1
    int valOp1 = leerValMemoria(mv, top1, posFisInstruccion);

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
}

void ejecutarInstruccion(Tmv *mv){
    int op1, op2, opC;
    op1 = mv->registros[OP1];
    op2 = mv->registros[OP2];
    opC = mv->registros[OPC];
    if (opC >= 0x0F && opC <= 0x0F){
        opC -= 0x0F;
        pfuncion0Param[opC](mv);
    }
    else if (opC >= 0x00 && opC <= 0x08){
        pfuncion1Param[opC](mv, mv->registros[OP1]);
    }
    else if (opC >= 0x10 && opC <= 0x1F){
        opC -= 0x10;
        pfuncion2Param[opC](mv, mv->registros[OP1], mv->registros[OP2]);
    }
    else {
        printf("Codigo de operacion invalido: %d\n", opC);
        exit(-1);
    }
}
char obtengoTipoOperando(int bytes) // testeado
{
    bytes >>= 30;
    bytes &= 0x00000003;
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
        bytes &= 0x0000001F;
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
        leerMemoria(mv, obtenerDirLogica(mv, bytes), 4, mv->registros[DS]);
        valor = mv->registros[MBR];
        break;
    }
    }

    return valor;
}



int obtenerDirFisica(Tmv *mv, int dirLogica)
{
    int segmento = obtenerHigh(dirLogica);
    int offset = obtenerLow(dirLogica);

    int base = obtenerHigh(mv->tablaSegmentos[segmento]);
    return base + offset;
}

void leerMemoria(Tmv* mv, int dirLogica, int cantBytes, int segmento)
{
    int tabla = mv->tablaSegmentos[obtenerHigh(segmento)];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);

    mv->registros[LAR] = dirLogica;
    int offsetFisico = obtenerDirFisica(mv, LAR);

    if (offsetFisico >= baseSegmento && offsetFisico + cantBytes <= baseSegmento + tamSegmento)
    {
        mv->registros[MAR] = combinarHighLow(cantBytes, offsetFisico);
        mv->registros[MBR] = leerValMemoria(mv, cantBytes, offsetFisico);
    }
    else
    {
        printf("Error: Desbordamiento de segmento\n");
        exit(-1);
    }
}

void setValor(Tmv *mv, int operando, int valor) // sin testear/incompleto
{
    char tipoOperando = obtengoTipoOperando(operando);
    switch (tipoOperando)
    {
    case 1:
    { // registro
        operando &= 0x000000FF;
        mv->registros[operando] = valor;
        break;
    }
    case 3:
    { // memoria
        operando &= 0x00FFFFFF;
        escribirMemoria(mv, operando, 4, valor, mv->registros[DS]); 
        break;
    }
    }
}


void escribirMemoria(Tmv *mv, int dirLogica, int cantBytes, int valor, int segmento){
    int tabla = mv->tablaSegmentos[obtenerHigh(segmento)];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);
    
    mv->registros[LAR] = dirLogica;
    int offsetFisico = obtenerDirFisica(mv, LAR);

    if (offsetFisico >= baseSegmento && offsetFisico + cantBytes <= baseSegmento + tamSegmento)
    {
        mv->registros[MBR] = valor;
        mv->registros[MAR] = combinarHighLow(cantBytes, offsetFisico);
        for(int i = cantBytes - 1; i >= 0; i++){
            mv->memoria[offsetFisico + i] = valor & 0x000000FF;
            valor >>= 8;//TODO verificar si esta bien el orden
        } 
    }
    else
    {
        printf("Error: Desbordamiento de segmento\n");
        exit(-1);
    }
}

int obtenerDirLogica(Tmv *mv, int valor)
{
    char codRegistro = (valor & 0x001F0000) >> 24;
    int offsetOp = valor & 0x0000FFFF;

    int valRegistro = mv->registros[codRegistro];

    // la direccion logica resultante sera la direccion logica del registro + el offset del operando
    return combinarHighLow(obtenerHigh(valRegistro), obtenerLow(valRegistro) + offsetOp);
}

void actualizarCC(Tmv *mv, int valor){
    mv->registros[CC] &= 0x80000000 & (valor < 0);
    mv->registros[CC] &= 0x40000000 & (valor == 0);
}

void MOV (Tmv *mv, int op1, int op2){
    int valor = getValor(mv, op2);
    setValor(mv, op1, valor); 
}

void ADD (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 + valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void SUB (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void MUL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 * valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void DIV (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    if(valor2 == 0){
        printf("Error, division por 0");
        exit(-1);
    }
    int cociente = valor1/valor2;
    int resto = valor1%valor2;
    actualizarCC(mv, cociente);
    setValor(mv, mv->registros[AC], resto);
    setValor(mv, op1, cociente); 
}

void CMP(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
}

void SHL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 << valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void SHR(Tmv *mv, int op1, int op2) {
    unsigned int valor1 = (unsigned int)getValor(mv, op1);
    unsigned int valor2 = (unsigned int)getValor(mv, op2);

    unsigned int resultado = valor1 >> valor2;   

    actualizarCC(mv, resultado);                 
    setValor(mv, op1, (int)resultado);           
}


void SAR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 >> valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void AND (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 & valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void OR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 | valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void XOR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 ^ valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor); 
}

void SWAP (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    setValor(mv, op1, valor2);
    setValor(mv, op2, valor1); 
}

void LDL(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0xFFFF0000;
    valor2 &= 0X0000FFFF;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

void LDH(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0X0000FFFF;
    valor2 &= 0X0000FFFF;
    valor2 <<= 16;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

void RND(Tmv *mv, int op1, int op2){
    srand(time(NULL));
    int valor2 = getValor(mv, op2);
    int valor1 = rand() % (valor2);
    
    setValor(mv,op1,valor1);
}

void JMP(Tmv *mv, int direccion){
    mv->registros[IP] = direccion;
}

void JZ(Tmv *mv, int direccion){
    if(mv->registros[CC] >= 0 && (mv->registros[CC] << 1) < 0)
        JMP(mv, direccion);
}

void JNZ(Tmv *mv, int direccion){
    if((mv->registros[CC] << 1) >= 0)
        JMP(mv, direccion);
}

void JN(Tmv *mv, int direccion){
    if(mv->registros[CC] < 0 && (mv->registros[CC] << 1) >= 0)
        JMP(mv, direccion);
}

void JNN(Tmv *mv, int direccion){
    if(mv->registros[CC] >= 0)
        JMP(mv, direccion);
}

void JP(Tmv *mv, int direccion){
    if(mv->registros[CC] >= 0 && (mv->registros[CC] << 1) >= 0)
        JMP(mv, direccion);
}

void JNP(Tmv *mv, int direccion){
    if(mv->registros[CC] < 0 && (mv->registros[CC] << 1) >= 0 || 
       mv->registros[CC] >= 0 && (mv->registros[CC] << 1) < 0)
        JMP(mv, direccion);
}

void NOT(Tmv *mv,int op1){
    int valor1 = getValor(mv, op1);
    valor1 ^= 0xFFFFFFFF; 
    actualizarCC(mv,valor1);
    setValor(mv, op1, valor1);
}

void STOP(Tmv *mv){
    mv->registros[IP] = -1; 
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

    const int ancho_tab = 32;

    while (ip < tam) {
        unsigned char ins  = mv->memoria[ip];
        unsigned char opc  = ins & 0x1F;
        unsigned char top2 = (ins >> 6) & 0x03;
        unsigned char top1 = (ins >> 4) & 0x03;


        char izq[256];
        int len = snprintf(izq, sizeof(izq), "[%04x] %02x ", base + ip, ins);

        if (opc == 0x0F) { // STOP
            int espacios = ancho_tab - len;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| STOP\n", izq, espacios, "");
            ip += tam;

        } else if (opc <= 0x08) { // 1 operando
            top1 = top2;
            int pos = len;
            for (int j = 0; j < top1; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02x ", (unsigned char) mv->memoria[ip + 1 + j]);

            int espacios = ancho_tab - pos;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| %s ", izq, espacios, "", mnemonicos[opc]);
            impNombreOperando(mv, ip, top1);
            printf("\n");
            ip += 1 + top1;
        } else if (opc >= 0x10 && opc <= 0x1F) { // 2 operandos
            int aux = top1 + top2;
            int pos = len;
            for (int j = 0; j < aux; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02x ", (unsigned char) mv->memoria[ip + 1 + j]);

            int spaces = ancho_tab - pos;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| %s ", izq, spaces, "", mnemonicos[opc]);
            impNombreOperando(mv, ip + top2, top1);
            printf(", ");
            impNombreOperando(mv, ip, top2);
            printf("\n");
            ip += 1 + aux;
        } else {
            int espacios = ancho_tab - len;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| UNKNOWN\n", izq, espacios, "");
            ip += 1;
        }
    }
}

//TODO probar
void imprimirBinario(unsigned int valor){
    printf(" 0b");
    do{
        printf("%d", valor & 1);
        valor >>= 1; 
    }while(valor != 0);
}

int leerBinario(){

}

void SYS(Tmv* mv, int operando){
    int valor = getValor(mv, operando);
    int formato = mv->registros[EAX];
    int cantCeldas = obtenerLow(mv->registros[ECX]);
    int tamCelda = obtenerHigh(mv->registros[ECX]);

    if(tamCelda <= 4 && tamCelda > 0 && tamCelda != 3 && formato > 0)
        switch(valor){
            case 1:{
                for(int i = 0; i < cantCeldas; i++){
                    int posActual = mv->registros[EDX] + i * tamCelda;
                    printf("[%x]:", obtenerLow(obtenerDirFisica(mv, posActual)));
                    leerMemoria(mv, posActual, tamCelda, mv->registros[DS]);
                    //TODO separar en imprimir valor
                    if(formato & 0x10 != 0)
                        imprimirBinario(mv->registros[MBR]);
                    char mascara = 0x8;
                    for(int j = 0; j < CANT_FORMATOS; j++){
                        if(formato & mascara != 0){
                            printf(formatos[j], mv->registros[MBR]);
                            mascara >>= 1;
                        }
                    }
                    printf("\n");
                }
                break;
            }
            case 2:{
                if(formato % 2 == 0 || formato == 0x1)
                    for(int i = 0; i < cantCeldas; i++){
                        int posActual = mv->registros[EDX]  + i * tamCelda;
                        int valorLeido;
                        printf("[%x]:", obtenerLow(obtenerDirFisica(mv, posActual)));
                        if(formato & 0x10 != 0)
                            valorLeido = leerBinario();
                        else{
                            char mascara = 0x8;
                            while(formato & mascara == 0)
                                mascara >>= 1;
                            scanf(formatos[sqrt(mascara)], &valorLeido);
                        }
                        escribirMemoria(mv, posActual, tamCelda, valorLeido, mv->registros[DS]);
                    }
                break;
            }
            default:{
                //TODO que hace si sys tiene operando erroneo?
            }
        }
}