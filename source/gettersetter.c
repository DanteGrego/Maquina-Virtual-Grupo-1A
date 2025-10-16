#include "mv.h"

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
        char registro = bytes & 0x0000001F;
        char tipoTamanioRegistro = (bytes >> 6) & 0b11; //obtengo el tamaño del registro a leer
    /*   00 4 bytes
         01 cuarto byte
         10 tercer byte
         11 2 bytes */
        switch (tipoTamanioRegistro){
            case 0b00: valor = mv->registros[registro]; break;
            case 0b01: 
                valor = mv->registros[registro] & 0x000000FF; 
                valor <<= 24;
                valor >>= 24;
            break;
            case 0b10: 
                valor = (mv->registros[registro] & 0x0000FF00) >> 8; 
                valor <<= 24;
                valor >>= 24;
            break;
            case 0b11: 
                valor = mv->registros[registro] & 0x0000FFFF; 
                valor <<= 16;
                valor >>= 16;
            break;
        }

            
        
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
        char tamMemoria = 4 - ((bytes >> 22) & 0b11);
        bytes &= 0x001FFFFF;
        char registro = bytes >> 16;
        int segmento = obtenerHigh(mv->registros[registro]);
        leerMemoria(mv, obtenerDirLogica(mv, bytes), tamMemoria, segmento);
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

//leo valor en la memoria sin validar desbordamiento de segmento (usada por leerMemoria)
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

//leo de la memoria actualizando LAR, MAR y MBR (valida desbordamiento)
void leerMemoria(Tmv* mv, int dirLogica, int cantBytes, int segmento)
{
    //busco las posiciones fisicas del segmento al que quiero acceder
    int tabla = mv->tablaSegmentos[segmento];
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
    int offsetOp = ((valor & 0x0000FFFF)<<16)>>16;

    //obtengo el registro con su codigo
    int valRegistro = mv->registros[codRegistro];

    //printf("codRegistro:%x\noffsetOp:%x\nvalReg:%x\n", codRegistro, offsetOp, valRegistro);
    // la direccion logica resultante sera la direccion logica del registro + el offset del operando
    return valRegistro + offsetOp;
}

//como getValor, setea el valor segun que sea
void setValor(Tmv *mv, int operando, int valor) // sin testear/incompleto
{
    char tipoOperando = obtengoTipoOperando(operando);
    switch (tipoOperando)
    {
    case 1:
    { // registro
        char tipoTamanioRegistro = (operando >> 6) & 0b11; //obtengo el tamaño del registro a escribir
    /*   00 4 bytes
         01 cuarto byte
         10 tercer byte
         11 2 bytes */
        char registro = operando & 0x0000001F;


        //printf("set registro: %x, tam: %x\n", registro, tipoTamanioRegistro);
        switch (tipoTamanioRegistro){
            case 0b00: mv->registros[registro] = valor; break;
            case 0b01: 
                mv->registros[registro] &= 0xFFFFFF00;
                mv->registros[registro] |= valor & 0x000000FF;
            break;
            case 0b10: 
                mv->registros[registro] &= 0xFFFF00FF;
                mv->registros[registro] |= (valor & 0x000000FF) << 8;
            break;
            case 0b11: 
                mv->registros[registro] &= 0xFFFF0000;
                mv->registros[registro] |= valor & 0x0000FFFF;
            break;
        }

        //printf("valor final: %x\n", mv->registros[registro]);

        break;
    }
    case 3:
    { // memoria
        char tamMemoria = 4 - ((operando >> 22) & 0b11);
        operando &= 0x001FFFFF;
        char registro = operando >> 16;
        int segmento = obtenerHigh(mv->registros[registro]);
        escribirMemoria(mv, obtenerDirLogica(mv, operando), tamMemoria, valor, segmento);
        break;
    }
    }
    //no puedo setear un inmediato
}

//escribe en la memoria actualizando registros LAR, MAR y MBR
void escribirMemoria(Tmv *mv, int dirLogica, int cantBytes, int valor, int segmento){
    //obtengo dimensiones fisicas del segmentos al que quiero acceder
    int tabla = mv->tablaSegmentos[segmento];
    int baseSegmento = obtenerHigh(tabla);
    int tamSegmento = obtenerLow(tabla);

    //printf("dirLogica: %x\n", dirLogica);
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
            mv->memoria[offsetFisico + i] = valor & 0x000000FF;
            valor >>= 8;
        }
    }
    else
    {
        printf("Error: Desbordamiento de segmento (0x%X)\nSe intento acceder a [0x%X]", mv->registros[IP], offsetFisico);
        exit(-1);
    }
}