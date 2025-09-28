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