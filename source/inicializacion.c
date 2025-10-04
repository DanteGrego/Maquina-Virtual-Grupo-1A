#include "mv.h"

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
            //TODO separar en otro archivo la carga del codigo, ademas la busqueda de la posicion inicial del CS esta mal (usar obtenerDirFisica(mv->registros[CS]))
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