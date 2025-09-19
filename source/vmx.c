#include "mv.h"

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
    if(segmento > CANT_SEGMENTOS){
        printf("Error: Desbordamiento de segmento (0x%X)\nSe intento acceder al segmento %d", mv->registros[IP], segmento);
        exit(-1);
    }else{
        int offset = obtenerLow(dirLogica);
        //printf("(dir fisica): seg: %d, offset: %x\n", segmento, offset);

        //busco la base fisica con el segmento de la direccion
        int base = obtenerHigh(mv->tablaSegmentos[segmento]);
        //la direccion fisica es esa base fisica mas el offset de la direccion
        return base + offset;
    }
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

//debug
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

int estaEnMemoriaAccedida(Tmv* mv, int pos){
    for(int i = 0; i < mv->nMemoriaAccedida; i++)
        if(mv->memoriaAccedida[i] == pos)
            return 1;
    return 0;
}
