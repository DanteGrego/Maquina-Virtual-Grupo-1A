#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include "mv.h"

int main(int numeroArgumentos, char *vectorArgumentos[])
{
    char *fileName;// nombre del archivo.vmx
    char imprimoDesensamblado = 0; // condicion booleana que decide mostrar el codigo desensamblado
    char devMode = 0;
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
        if (numeroArgumentos > 2 && strcmp(vectorArgumentos[2], "-dev") == 0)
            devMode = 1;
         if (numeroArgumentos > 3 && strcmp(vectorArgumentos[3], "-dev") == 0)
            devMode = 1;

        leerArch(&mv, fileName);
        if(imprimoDesensamblado)
            disassembler(&mv);

        int debugCount = 0;
        inicializarRegistros(&mv);
        while(debugCount < 1000 && seguirEjecutando(&mv)){
            if(devMode){
                printf("\n--------------------------------------------------\n");
                printf("\nTabla de segmentos: \n");
                imprimirTabla(&mv);
                printf("\nInstruccion %d:\n\n Antes de leer: \n",debugCount);
                imprimirRegistros(&mv);
                printf("\n   Memoria: \n");
                imprimirMemoria(&mv);
            }

            leerInstruccion(&mv);

            if(devMode){
                printf("\n  Despues de leer: \n",debugCount);
                imprimirRegistros(&mv);
                printf("\n   Memoria: \n");
                imprimirMemoria(&mv);
            }

            ejecutarInstruccion(&mv);

            if(devMode){
                printf("\n  Despues de ejecutar: \n",debugCount);
                imprimirRegistros(&mv);
                printf("\n   Memoria: \n");
                imprimirMemoria(&mv);
                scanf("%d");
            }

            debugCount++;
        }
    }

    return 0;
}

void imprimirTabla(Tmv* mv){
    for(int i = 0; i < CANT_SEGMENTOS; i++)
        printf("[%d]: 0x%8X\n", i, (unsigned int)mv->tablaSegmentos[i]);
}

void imprimirMemoria(Tmv* mv){
    for(int i = 0; i < mv->nMemoriaAccedida; i++)
        printf("    [%X]: 0x%X (%d)\n", mv->memoriaAccedida[i], mv->memoria[mv->memoriaAccedida[i]], mv->memoria[mv->memoriaAccedida[i]]);
}

void imprimirRegistros(Tmv* mv){
    for(int i = 0; i < 7; i++)
        printf("    %s: 0x%X (%d)\n", nombreRegistros[i], mv->registros[i], mv->registros[i]);
    for(int i = 10; i < 18; i++)
        printf("    %s: 0x%X (%d)\n", nombreRegistros[i], mv->registros[i], mv->registros[i]);

    printf("    %s: 0x%X (%d)\n", nombreRegistros[26], mv->registros[26], mv->registros[26]);
    printf("    %s: 0x%X (%d)\n", nombreRegistros[27], mv->registros[27], mv->registros[27]);
}

int seguirEjecutando(Tmv* mv){
    if(mv->registros[IP] < 0)
        return 0;
    else{
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
}

//combina los dos bytes de uno en la alta y dos bytes del otro en la baja
int combinarHighLow(int bytesHigh, int bytesLow)
{
    return (bytesHigh << 16) | (bytesLow & 0x0000FFFF);
}

//devuelve los dos bytes mas significantes
int obtenerHigh(int bytes)
{
    int res = 0;
    bytes >>= 16;
    res = bytes & 0x0000FFFF;
    return res;
}

//devuelve los dos bytes menos significantes
int obtenerLow(int bytes)
{
    return bytes & 0x0000FFFF;
}

//carga al CS el codigo del archivo
void leerArch(Tmv *mv, char *nomArch)
{
    char x;
    int i;
    unsigned char cabecera[5], version, tamCodigo[2];
    FILE *arch;
    arch = fopen(nomArch, "rb");

    if (arch != NULL)
    {
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMX25") == 0)
        {
            //leo version y tamano del codigo
            fread(&version, sizeof(unsigned char), 1, arch);
            fread(tamCodigo, sizeof(unsigned char), 2, arch);

            //cargo la tabla de segmentos
            cargarTablaSegmentos(mv, tamCodigo[1] + tamCodigo[0] * 256);

            //obtengo base del CS
            i = obtenerHigh(mv->tablaSegmentos[0]);

            //mientras haya para leer cargo al CS el codigo leyendo de memoria
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
    //cargo CS que va de 0 a tamano de codigo
    mv->tablaSegmentos[0] = combinarHighLow(0, tamCodigo);
    //cargo DS que vas desde tamCodigo a el final de la memoria
    mv->tablaSegmentos[1] = combinarHighLow(tamCodigo, TAM_MEMORIA - tamCodigo);
}

void inicializarRegistros(Tmv *mv)
{
    mv->registros[CS] = 0x00000000; // 0x0000 0000
    mv->registros[DS] = 0x00010000; // 0x0001 0000
    //IP arranca en CS como direccion logica
    mv->registros[IP] = mv->registros[CS];

    //debug
    mv->nMemoriaAccedida = 0;
}

int estaEnMemoriaAccedida(Tmv* mv, int pos){
    for(int i = 0; i < mv->nMemoriaAccedida; i++)
        if(mv->memoriaAccedida[i] == pos)
            return 1;
    return 0;
}

//leo valor en la memoria sin validar desbordamiento de segmento
int leerValMemoria(Tmv *mv, int cantBytes, int posFisica)
{
    //valor leido
    int valor = 0;

    //TODO habria que validar si la cantidad de bytes es > 0? porque es funcinamiento interno
    //if (cantBytes > 0)
    //{
        //leo el primer byte y hago propagacion de signo
        valor = (unsigned char)mv->memoria[posFisica];
        valor <<= 24;
        valor >>= 24; // escopeta goes brr


        //leo el restante de bytes
        cantBytes--;
        for (int i = 0; i < cantBytes; i++)
        {
            valor <<= 8;
            valor |= (unsigned char)mv->memoria[++posFisica];
        }
    //}

    return valor;
}

//leo la instruccion actualizando los registros de operancos e incrementando el IP
void leerInstruccion(Tmv *mv)
{
    //obtengo la direccion fisica del IP
    int posFisInstruccion = obtenerDirFisica(mv, mv->registros[IP]);
    //la instruccion esta en la memoria en esa posicion (se supone que el IP esta dentro del CS)
    char instruccion = mv->memoria[posFisInstruccion];
    //obtengo operandos con mascaras y shifteos
    char top2 = (instruccion >> 6) & 0x03;
    char top1 = (instruccion >> 4) & 0x03;
    //printf("Operandos %d %d\n",top1,top2);
    char opc = instruccion & 0x1F;
    //printf("Instruccion %x\n",instruccion);

    posFisInstruccion++; // me pongo en posicion para leer op2
    int valOp2 = leerValMemoria(mv, top2, posFisInstruccion);
    posFisInstruccion += top2; // me pongo en posicion para leer op1
    int valOp1 = leerValMemoria(mv, top1, posFisInstruccion);

    //si hay solo un operando hago swap
    if (top1 == 0)
    {
        top1 = top2;
        valOp1 = valOp2;
        top2 = valOp2 = 0; // TODO preguntar si cuando hay un solo parametro op2 tiene que ser 0 o no
    }

    //asigno registros de la mv con lo obtenido
    mv->registros[OPC] = opc;
    mv->registros[OP1] = ((int)top1 << 24) | (valOp1 & 0x00FFFFFF); // masqueado por si era negativo, sino me tapa el top en el primer byte
    mv->registros[OP2] = ((int)top2 << 24) | (valOp2 & 0x00FFFFFF);
    mv->registros[IP] += 1 + top1 + top2;
}

//busco con el OPC que funcion es y la llamo con OP1 y OP2 cargados
void ejecutarInstruccion(Tmv *mv){
    int op1, op2, opC;
    op1 = mv->registros[OP1];
    op2 = mv->registros[OP2];
    opC = mv->registros[OPC];
    //funciones de 0 parametros
    if (opC >= 0x0F && opC <= 0x0F){
        opC -= 0x0F;
        pfuncion0Param[opC](mv);
    }//funciones de 1 parametro
    else if (opC >= 0x00 && opC <= 0x08){
        pfuncion1Param[opC](mv, mv->registros[OP1]);
    }//funciones de 2 parametros
    else if (opC >= 0x10 && opC <= 0x1F){
        opC -= 0x10;
        pfuncion2Param[opC](mv, mv->registros[OP1], mv->registros[OP2]);
    }
    else {
        printf("Codigo de operacion invalido: %d\n", opC);
        exit(-1);
    }
}

//obtengo de un operando su tipo (de op1 u op2)
char obtengoTipoOperando(int bytes) // testeado
{
    bytes >>= 24;
    bytes &= 3;
    return bytes;
}

//obtengo el valor de donde tenga que ser (registro, inmediato o memoria)
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
        bytes <<= 16;
        bytes >>= 16;
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

//obtengo la direccion fisica de una logica
int obtenerDirFisica(Tmv *mv, int dirLogica)
{
    //obtengo los componentes de la direccion logica
    int segmento = obtenerHigh(dirLogica);
    int offset = obtenerLow(dirLogica);
    //printf("(dir fisica): seg: %d, offset: %x\n", segmento, offset);

    //busco la base fisica con el segmento de la direccion
    int base = obtenerHigh(mv->tablaSegmentos[segmento]);
    //la direccion fisica es esa base fisica mas el offset de la direccion
    return base + offset;
}

//leo de la memoria actualizando LAR, MAR y MBR
void leerMemoria(Tmv* mv, int dirLogica, int cantBytes, int segmento)
{
    //busco las posiciones fisicas del segmento al que quiero acceder
    int tabla = mv->tablaSegmentos[obtenerHigh(segmento)];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);

    //actualizo el LAR
    mv->registros[LAR] = dirLogica;
    //obtengo con esa direccion logica su direccion fisica
    int offsetFisico = obtenerDirFisica(mv, mv->registros[LAR]);

    //valido si estoy dentro del segmento al que queria acceder
    if (offsetFisico >= baseSegmento && offsetFisico + cantBytes <= baseSegmento + tamSegmento)
    {
        //si es asi actualizo el MAR y el MBR
        mv->registros[MAR] = combinarHighLow(cantBytes, offsetFisico);
        mv->registros[MBR] = leerValMemoria(mv, cantBytes, offsetFisico);
    }
    else
    {
        printf("Error: Desbordamiento de segmento (0x%X)\nSe intento acceder a [0x%X]", mv->registros[IP], offsetFisico);
        exit(-1);
    }
}

//como getValor, setea el valor segun que sea
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
        escribirMemoria(mv, obtenerDirLogica(mv, operando), 4, valor, mv->registros[DS]);
        break;
    }
    }
    //no puedo setear un inmediato
}

//escribe en la memoria actualizando registros LAR, MAR y MBR
void escribirMemoria(Tmv *mv, int dirLogica, int cantBytes, int valor, int segmento){
    //obtengo dimensiones fisicas del segmentos al que quiero acceder
    int tabla = mv->tablaSegmentos[obtenerHigh(segmento)];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);

    //actualizo LAR
    mv->registros[LAR] = dirLogica;
    //saco la direccion fisica con la logica
    int offsetFisico = obtenerDirFisica(mv, mv->registros[LAR]);
    //printf("escribir mem: logica: %x, fis: %x, cantBytes: %d\n", dirLogica, offsetFisico, cantBytes);

    //valido estar dentro del segmento al que quiero acceder
    if (offsetFisico >= baseSegmento && offsetFisico + cantBytes <= baseSegmento + tamSegmento)
    {
        //actualizo MBR y MAR
        mv->registros[MBR] = valor;
        mv->registros[MAR] = combinarHighLow(cantBytes, offsetFisico);
        //escribo la memoria
        for(int i = cantBytes - 1; i >= 0; i--){
            if(!estaEnMemoriaAccedida(mv, offsetFisico + i)){
                mv->memoriaAccedida[mv->nMemoriaAccedida] = offsetFisico + i;
                (mv->nMemoriaAccedida)++;
            }
            mv->memoria[offsetFisico + i] = valor & 0x000000FF;
            valor >>= 8;//TODO verificar si esta bien el orden
        }
    }
    else
    {
        printf("Error: Desbordamiento de segmento (0x%X)\nSe intento acceder a [0x%X]", mv->registros[IP], offsetFisico);
        exit(-1);
    }
}

//obtengo la direccion logica con un operando dentro de []
int obtenerDirLogica(Tmv *mv, int valor)
{
    //obtengo registro y offset
    char codRegistro = (valor & 0x001F0000) >> 16;
    int offsetOp = valor & 0x0000FFFF;

    //obtengo el registro con su codigo
    int valRegistro = mv->registros[codRegistro];

    // la direccion logica resultante sera la direccion logica del registro + el offset del operando
    return combinarHighLow(obtenerHigh(valRegistro), obtenerLow(valRegistro) + offsetOp);
}

void actualizarCC(Tmv *mv, int valor){
    //actualizo NZ de CC, 1er y 2do bit si es negativo o nulo respectivamente
    if(valor < 0)
        mv->registros[CC] |= 0x80000000;
    else
        mv->registros[CC] &= 0x7FFFFFFF;//~(0x80000000)

    if(valor == 0){
        mv->registros[CC] |= 0x40000000;
    }
    else
        mv->registros[CC] &= 0xbFFFFFFF;//~(0x40000000)
}

//dissasembler: imprime el operando
void impNombreOperando(const Tmv* mv, int ip, int tipo) {
    int num, low, high;
    switch (tipo) {
        case 1: { // registro
            //saco el registro con el codigo que tiene en memoria
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            break;
        }
        case 2: { // inmediato (2 bytes)
            //saco el valor con los dos bytes que ocupa el inmediato y propago signo
            high = (unsigned char) mv->memoria[ip+1];
            low  = (unsigned char) mv->memoria[ip+2];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            printf("%d (0x%X)", num, num);
            break;
        }
        case 3: { // memoria: [registro + offset]
            printf("[");
            //imprimo registro como caso 1
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            //obtengo offset como el inmediato en caso 2
            high = (unsigned char) mv->memoria[ip+2];
            low  = (unsigned char) mv->memoria[ip+3];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            //imprimo segun el signo el offset
            if (num > 0)      printf(" + %d", num);
            else if (num < 0) printf(" %d", num);
            printf("]");
            break;
        }
        default: /* tipo invalido */ break;
    }
}

void disassembler(const Tmv* mv) {
    int base = obtenerHigh(mv->tablaSegmentos[0]); // base seg código
    int tam = obtenerLow(mv->tablaSegmentos[0]);  // tamaño seg código
    int ip = 0;

    //ancho de la linea
    const int ancho_tab = 32;

    while (ip < tam) {
        unsigned char ins = mv->memoria[ip];
        //como mv con registros pero aca son variables temporales
        unsigned char opc = ins & 0x1F;
        unsigned char top2 = (ins >> 6) & 0x03;
        unsigned char top1 = (ins >> 4) & 0x03;


        char izq[256];
        int len = snprintf(izq, sizeof(izq), "[%04X] %02X ", base + ip, ins);

        if (opc == 0x0F) { // STOP
            int espacios = ancho_tab - len;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| STOP\n", izq, espacios, "");
            ip += tam;

        } else if (opc <= 0x08) { // 1 operando
            //swap
            top1 = top2;
            int pos = len;
            for (int j = 0; j < top1; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02X ", (unsigned char) mv->memoria[ip + 1 + j]);

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
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02X ", (unsigned char) mv->memoria[ip + 1 + j]);

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
void imprimirBinario(unsigned int valor, int tamCelda){
    printf(" 0b");
    int mascara = 0x80 << ((tamCelda - 1) * 8);
    for(int i = 0; i < tamCelda * 8; i++){
        printf("%d", (valor & mascara) != 0);
        valor <<= 1;
    }
}

int leerBinario(){
    char* cadLeida;
    int retorno = 0;

    scanf("%s", cadLeida);

    int i = 0;
    while(cadLeida[i] == '0' || cadLeida[i] == '1'){
        retorno <<= 1;
        retorno |= (int)cadLeida[i] - 48;
        i++;
    }

    return retorno;
}

void SYS(Tmv* mv, int operando){
    int valor = getValor(mv, operando);
    int formato = mv->registros[EAX];
    int cantCeldas = obtenerLow(mv->registros[ECX]);
    int tamCelda = obtenerHigh(mv->registros[ECX]);

    if(tamCelda <= 4 && tamCelda > 0 && tamCelda != 3 && formato > 0)
        switch(valor){
            case 1:{
                if(formato % 2 == 0 || formato == 0x1)
                    for(int i = 0; i < cantCeldas; i++){
                        int posActual = mv->registros[EDX] + i * tamCelda;
                        int valorLeido;
                        printf("[%X]: ", obtenerLow(obtenerDirFisica(mv, posActual)));
                        if((formato & 0x10) != 0)
                            valorLeido = leerBinario();
                        else{
                            char mascara = 0x8;
                            int i = CANT_FORMATOS - 1;
                            while((formato & mascara) == 0){
                                mascara >>= 1;
                                i--;
                            }

                            scanf(formatosLectura[i], &valorLeido);
                        }

                        escribirMemoria(mv, posActual, tamCelda, valorLeido, mv->registros[DS]);
                    }
                break;
            }
            case 2:{
                for(int i = 0; i < cantCeldas; i++){
                    int posActual = mv->registros[EDX] + i * tamCelda;
                    printf("[%X]:", obtenerLow(obtenerDirFisica(mv, posActual)));
                    leerMemoria(mv, posActual, tamCelda, mv->registros[DS]);
                    //TODO separar en imprimir valor
                    if((formato & 0x10) != 0)
                        imprimirBinario(mv->registros[MBR], tamCelda);
                    unsigned char mascara = 0x8;
                    for(int j = 0; j < CANT_FORMATOS; j++){
                        if((formato & mascara) != 0)
                            printf(formatosEscritura[CANT_FORMATOS - j - 1], mv->registros[MBR]);
                        mascara >>= 1;
                    }
                    printf("\n");
                }
                break;
            }
            default:{
                //TODO que hace si sys tiene operando erroneo?
            }
        }
}

//op1 = op2
void MOV (Tmv *mv, int op1, int op2){
    int valor = getValor(mv, op2);
    setValor(mv, op1, valor);
}

//op1 = op1 + op2
void ADD (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 + valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 - op2
void SUB (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 * op2
void MUL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 * valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 / op2
//ac = resto de la division
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

//actualizo NZ con op1 - op2
void CMP(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
}

//shifteo left op1 op2 veces
void SHL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 << valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//shifteo logico right op1 op2 veces
void SHR(Tmv *mv, int op1, int op2) {
    unsigned int valor1 = (unsigned int)getValor(mv, op1);
    unsigned int valor2 = (unsigned int)getValor(mv, op2);

    unsigned int resultado = valor1 >> valor2;

    actualizarCC(mv, resultado);
    setValor(mv, op1, (int)resultado);
}

//shifteo aritmetico op1 op2 veces
void SAR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 >> valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 & op2
void AND (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 & valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 | op2
void OR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 | valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 ^ op2
void XOR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 ^ valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op2
//op2 = op1
void SWAP (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    setValor(mv, op1, valor2);
    setValor(mv, op2, valor1);
}

//cargo los dos bytes menos significantes de op1 con los de op2
void LDL(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0xFFFF0000;
    valor2 &= 0X0000FFFF;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

//cargo los dos bytes mas significantes de op1 con los de op2
void LDH(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0X0000FFFF;
    valor2 &= 0X0000FFFF;
    valor2 <<= 16;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

//op1 = random >= 0 y < op2
void RND(Tmv *mv, int op1, int op2){
    srand(time(NULL));
    int valor2 = getValor(mv, op2);
    int valor1 = rand() % (valor2);

    setValor(mv,op1,valor1);
}

int isN(Tmv* mv){
    return mv->registros[CC] < 0;
}

int isZ(Tmv* mv){
    return (mv->registros[CC] << 1) < 0;
}

//salto siempre a la direccion
void JMP(Tmv *mv, int operando){
    int direccion = getValor(mv, operando);
    mv->registros[IP] = direccion;
}

//salto si esta z
void JZ(Tmv *mv, int operando){
    if(isZ(mv) && !isN(mv))
        JMP(mv, operando);
}

//salto si no esta zs
void JNZ(Tmv *mv, int operando){
    if(!isZ(mv))
        JMP(mv, operando);
}

//salto si esta n
void JN(Tmv *mv, int operando){
    if(isN(mv) && !isZ(mv))
        JMP(mv, operando);
}

//salto si no esta n
void JNN(Tmv *mv, int operando){
    if(!isN(mv))
        JMP(mv, operando);
}

//salto si no esta n ni z
void JP(Tmv *mv, int operando){
    if(!isZ(mv) && !isN(mv))
        JMP(mv, operando);
}

//salto si esta z o esta n
void JNP(Tmv *mv, int operando){
    if(!isZ(mv) && isN(mv) || isZ(mv) && !isN(mv))
        JMP(mv, operando);
}

//op1 = ~op1
void NOT(Tmv *mv,int op1){
    int valor1 = getValor(mv, op1);
    valor1 ^= 0xFFFFFFFF;
    actualizarCC(mv,valor1);
    setValor(mv, op1, valor1);
}

//paro el programa, mando IP fuera del CS (posicion -1)
void STOP(Tmv *mv){
    mv->registros[IP] = -1;
}
