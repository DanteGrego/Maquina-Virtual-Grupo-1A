#include "mv.h"
#include <windows.h>

int main(int numeroArgumentos, char *vectorArgumentos[])
{
    char *fileName;            // nombre del archivo.vmx
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

//devuelve true or false si se deben seguir ejecutando las instrucciones
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
        top2 = valOp2 = 0;
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