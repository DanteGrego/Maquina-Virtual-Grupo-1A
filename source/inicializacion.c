#include "mv.h"

void inicializarTablaRegistrosVersion1(Tmv* mv, FILE* arch){
    int tamCodigo;
    fread(tamCodigo, 2, 1, arch);

    mv->tablaSegmentos[0] = combinarHighLow(0, tamCodigo);
    mv->tablaSegmentos[1] = combinarHighLow(tamCodigo, mv->tamMemoria - tamCodigo);

    mv->registros[CS] = 0x00000000;
    mv->registros[DS] = 0x00010000;

    mv->registros[IP] = mv->registros[CS];
}

void inicializarTablaRegistrosVersion2(Tmv* mv, FILE* arch, int tamPS){
    int tamSegmento, baseSegmento = 0, segmento = 0, entryPoint;

    if(tamPS > 0){
        mv->tablaSegmentos[0] = combinarHighLow(0, tamPS);
        mv->registros[PS] = 0x00000000;
        segmento++;
        baseSegmento += tamPS;
    }

    for(int i = 0; i < 5; i++){//leo el tamanio de 5 segmentos
        fread(&tamSegmento, 2, 1, arch);
        if(tamSegmento > 0){
            if(baseSegmento + tamSegmento > mv->tamMemoria){
                printf("Memoria insuficiente");
                exit(-1);
            }
            mv->tablaSegmentos[CS+segmento] = combinarHighLow(baseSegmento, tamSegmento);
            mv->registros[CS+i] = combinarHighLow(segmento, 0);
            segmento++;
            baseSegmento += tamSegmento;
        }else{
            mv->registros[CS+i] = -1;
        }
    }

    if(mv->registros[SS] > 0){
        int tamPila = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[SS])]);
        mv->registros[SP] = mv->registros[SS] + tamPila;
    }

    fread(&entryPoint, 2, 1, arch);
    mv->registros[IP] = mv->registros[CS] + entryPoint;
}


void cargarCS(Tmv* mv, FILE* arch){
    int posMemoria = obtenerDirFisica(mv, mv->registros[CS]);
    int tamCS = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]);
    //TODO fijarse si anda
    fread(mv->memoria + posMemoria, 1, tamCS, arch);
}


void leerArchivoVmx(Tmv *mv, int tamPS)
{
    unsigned char cabecera[5], version;
    FILE *arch;
    arch = fopen(mv->fileNameVmx, "rb");

    if (arch != NULL)
    {
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMX25") == 0)
        {
            //leo version y tamano del codigo
            fread(&version, sizeof(unsigned char), 1, arch);
            switch(version){
                case 1:{
                    inicializarTablaRegistrosVersion1(mv, arch);
                    break;
                }
                case 2:{
                    inicializarTablaRegistrosVersion2(mv, arch, tamPS);
                    break;
                }
                default:{
                    printf("Version invalida");
                    exit(-1);
                    break;
                }
            }
            //se supone que se sale con el archivo ya leida la cabecera
            cargarCS(mv, arch);
            
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