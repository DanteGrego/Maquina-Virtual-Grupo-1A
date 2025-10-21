#include "mv.h"

int main(int numeroArgumentos, char *vectorArgumentos[])
{
    Tmv mv;
    mv.fileNameVmx = NULL;              // nombre del archivo.vmx
    mv.fileNameVmi = NULL;              // nombre del archivo.vmy 
    mv.tamMemoria = TAM_MEMORIA;        // se inicializa en 16Kib

    char imprimoDesensamblado = 0; // condicion booleana que decide mostrar el codigo desensamblado
    char ingresoDebug;
    //printf("entro al main \n");

    if (numeroArgumentos < 2)
    {
        printf("Numero insuficiente de argumentos");
        return -1;
    }
    else
    {
        //printf("numero de argumentos: %d \n", numeroArgumentos);
        int i = 1;
        char argumentoActual[500];
        char extensionArchivo[500];
        while (i < numeroArgumentos && strcmp(vectorArgumentos[i],"-p") != 0){
            //printf("argumento actual: %s \n", vectorArgumentos[i]);
            strcpy(argumentoActual, vectorArgumentos[i]);
            strcpy(extensionArchivo, getExtension(argumentoActual));
            if (strcmp(extensionArchivo, ".vmx") == 0)
                mv.fileNameVmx = vectorArgumentos[i];
            else if (strcmp(extensionArchivo, ".vmi") == 0)
                mv.fileNameVmi = vectorArgumentos[i];
            else if (argumentoActual[0] == 'm' && argumentoActual[1] == '=')
                cargoTamMemoria(&mv, argumentoActual); // aca se carga mv.tamMemoria, si nunca entra ya se inicializo en 16Kib
            else if (argumentoActual[0] == '-' && argumentoActual[1] == 'd')
                imprimoDesensamblado = 1;
            i++;
        }

        if (mv.fileNameVmi == NULL && mv.fileNameVmx == NULL){
            printf("ERROR: no se especificaron archivos para la ejecucion");
            return -1;
        }
        else
            if (mv.fileNameVmx != NULL){    // si hay vmx ->
                //printf("hay archivo vmx \n");
                //printf("nombre del archivo: %s \n", mv.fileNameVmx);
                i++;//salteo argumento -p
                int *vectorPunteros = (int*) malloc(sizeof(int)*(numeroArgumentos-i));
                mv.memoria = (char*) malloc(mv.tamMemoria);
                int tamPS = 0;
                int posArgv = 0;
                int k = 0;
                while (i < numeroArgumentos){ // cargo los parametros en el param segment y obtengo su tamaño
                    int j = 0;
                    vectorPunteros[k++] = tamPS;
                    //printf("argumento del param segment %d %s \n",k,vectorArgumentos[i]);
                    do{
                        if (tamPS > mv.tamMemoria){
                            printf("Excedido tamanio de memoria");
                            exit(-1);
                        }
                        mv.memoria[tamPS] = vectorArgumentos[i][j];
                        tamPS++;
                    } while (vectorArgumentos[i][j++] != '\0');
                    i++;
                }
                posArgv = tamPS;
                //printf("fin lectura del ps, tamanio: %d cantidad parametros: %d\n",tamPS, k);
                for (int w = 0; w < k; w++){//cargo punteros a los parametros en el param segment
                    if (tamPS + 4 > mv.tamMemoria){
                            printf("Excedido tamanio de memoria");
                            exit(-1);
                    }
                    
                    int puntero = vectorPunteros[w];
                    for(int x = 0; x < 4; x++){
                        mv.memoria[tamPS+x] = (puntero >> 24) & 0x000000FF;
                        puntero <<= 8;
                    }
                    tamPS += 4;
                }
                //printf("salio del for de parametros \n");

                leerArchivoVmx(&mv, tamPS);


                //printf("Se leyo el archivo vmx \n");
                if(mv.registros[SS] >= 0){
                    if(tamPS == 0)
                        posArgv = -1;
                    //printf("SP0: %x\n", mv.registros[SP]);
                    mv.registros[SP] -= 4;
                    //printf("SS: %x\n", mv.registros[SS]);
                    escribirMemoria(&mv, mv.registros[SP], 4, posArgv, obtenerHigh(mv.registros[SS]));
                    //printf("SP: %x\n", mv.registros[SP]);
                    //printf("Se escribio SSargv: %x\n", mv.memoria[obtenerDirFisica(&mv, mv.registros[SP])]);
                    mv.registros[SP] -= 4;
                    escribirMemoria(&mv, mv.registros[SP], 4, k, obtenerHigh(mv.registros[SS]));//escribo argc
                    //printf("SP: %x\n", mv.registros[SP]);
                    //printf("Se escribio SSargc: %x\n", mv.memoria[obtenerDirFisica(&mv, mv.registros[SP])]);
                    mv.registros[SP] -= 4;
                    escribirMemoria(&mv, mv.registros[SP], 4, -1, obtenerHigh(mv.registros[SS]));//escribo ret del main
                    //printf("SP: %x\n", mv.registros[SP]);
                    //printf("Se escribio SSret: %x\n", mv.memoria[obtenerDirFisica(&mv, mv.registros[SP])]);
                }
            }
            else{
                leerArchivoVmi(&mv);
            }

        if (imprimoDesensamblado == 1)
            disassembler(&mv);


        mv.modoDebug = 0;//TODO esta bien ubicarlo aca?
        while(seguirEjecutando(&mv)){
            //printf("debug i: %d ",debugi++);
            leerInstruccion(&mv);
            //printf("  se leyo inst OPC: %d", mv.registros[OPC]);
            ejecutarInstruccion(&mv);
            //printf("  se ejecuto inst: IP: %d\n",mv.registros[IP]);
            if(mv.modoDebug){
                scanf("%c", &ingresoDebug);
                switch(ingresoDebug){
                    case 'g':{
                        mv.modoDebug = 0;//sigue con la ejecucion hasta el proximo breakpoint o hasta terminar
                        break;
                    }
                    case 'q':{
                        exit(0);//termina la ejecucion
                        break;
                    }
                    default:{
                        sysBreakpoint(&mv);//sigue ejecucion paso a paso con un breakpoint en cada uno
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
//obtiene la extension de una cadena, de no tener extension devuelve un string vacio
char *getExtension(char *cadena) {
    char *ultimoPunto = strrchr(cadena, '.'); // busca el último punto

    if (ultimoPunto == NULL || *(ultimoPunto + 1) == '\0') {
        return ""; // no hay extensión o el punto está al final
    }

    return ultimoPunto; // devuelve la parte después del punto con el punto
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
    //printf("OP1: %x\nOP2: %x\nOPC: %x\nIP: %x\n", mv->registros[OP1], mv->registros[OP2], mv->registros[OPC], mv->registros[IP]);
}

//busco con el OPC que funcion es y la llamo con OP1 y OP2 cargados
void ejecutarInstruccion(Tmv *mv){
    int op1, op2, opC;
    op1 = mv->registros[OP1];
    op2 = mv->registros[OP2];
    opC = mv->registros[OPC];

    //funciones de 0 parametros
    if (opC >= 0x0E && opC <= 0x0F){
        opC = vectorTraductorIndicesCOperacion[opC];
        pfuncion0Param[opC](mv);
    }//funciones de 1 parametro
    else if (opC >= 0x00 && opC <= 0x08 || opC >= 0x0B && opC <= 0x0D){
        opC = vectorTraductorIndicesCOperacion[opC];
        pfuncion1Param[opC](mv, mv->registros[OP1]);
    }//funciones de 2 parametros
    else if (opC >= 0x10 && opC <= 0x1F){
        opC = vectorTraductorIndicesCOperacion[opC];
        pfuncion2Param[opC](mv, mv->registros[OP1], mv->registros[OP2]);
    }
    else {
        printf("Codigo de operacion invalido: %d\n", opC);
        exit(-1);
    }
}