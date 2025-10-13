#include "mv.h"

void cargoTamMemoria(Tmv* mv, char* argumento){
    int tamMemoria = 0;
    int i = 0;

    while(argumento[i] != '\0'){
        tamMemoria *= 10;
        tamMemoria += argumento[i] - '0';
        i++;
    }

    mv->tamMemoria = tamMemoria;
}

void inicializarTablaRegistrosVersion1(Tmv* mv, FILE* arch){
    int tamCodigo;
    fread(&tamCodigo, 2, 1, arch);

    mv->tablaSegmentos[0] = combinarHighLow(0, tamCodigo);
    mv->tablaSegmentos[1] = combinarHighLow(tamCodigo, mv->tamMemoria - tamCodigo);

    mv->registros[CS] = 0x00000000;
    mv->registros[DS] = 0x00010000;

    mv->registros[IP] = mv->registros[CS];
}

void inicializarTablaRegistrosVersion2(Tmv* mv, FILE* arch, int tamPS){
    int tamSegmento, baseSegmento = 0, segmento = 0, entryPoint;
    char lectura[2];

    printf("tamPS: %d\n", tamPS);

    if(tamPS > 0){
        mv->tablaSegmentos[0] = combinarHighLow(0, tamPS);
        mv->registros[PS] = 0x00000000;
        segmento++;
        baseSegmento += tamPS;
    }

    for(int i = 0; i < 5; i++){//leo el tamanio de 5 segmentos
        fread(lectura, 1, 2, arch);
        tamSegmento = (lectura[0] << 8) + lectura[1];
        printf("tamSegmento %d \n",tamSegmento);
        printf("segmento: %d\n", segmento);
        printf("baseSegmento: %x\n", baseSegmento);
        printf("tamSegmento: %x\n", tamSegmento);
        if(tamSegmento > 0){
            if(baseSegmento + tamSegmento > mv->tamMemoria){
                printf("Memoria insuficiente");
                exit(-1);
            }
            mv->tablaSegmentos[segmento] = combinarHighLow(baseSegmento, tamSegmento);
            mv->registros[CS+i] = combinarHighLow(segmento, 0);
            printf("contenido registro: %x \n", mv->registros[CS+i]);
            segmento++;
            baseSegmento += tamSegmento;
        }else{
            mv->registros[CS+i] = -1;
        }

    }
    for(int i = 0; i < CANT_SEGMENTOS; i++){
        printf("Tabla %d: %x, %x\n", i, obtenerHigh(mv->tablaSegmentos[i]), obtenerLow(mv->tablaSegmentos[i]));
        printf("Registro: %x\n", mv->registros[CS+i]);
    }
    printf("tamanio de los 5 leido bien\n");

    if(mv->registros[SS] > 0){
        int tamPila = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[SS])]);
        mv->registros[SP] = mv->registros[SS] + tamPila;
    }
    printf("fin cargar SP\n");

    fread(lectura, 1, 2, arch);
    entryPoint = (lectura[0] << 8) + lectura[1];
    mv->registros[IP] = mv->registros[CS] + entryPoint;
    printf("fin inicializacion\n");
}


void cargarCS(Tmv* mv, FILE* arch){
    int posMemoria = obtenerDirFisica(mv, mv->registros[CS]);
    int tamCS = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]);
    //TODO fijarse si anda
    fread(mv->memoria + posMemoria, 1, tamCS, arch);
}


void leerArchivoVmx(Tmv *mv, int tamPS)
{
    unsigned char cabecera[6], version;
    FILE *arch;

    arch = fopen(mv->fileNameVmx, "rb");

    printf("Se abrio el archivo\n");

    if (arch != NULL)
    {
        printf("Se abrio el vmx\n");
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);
        cabecera[5] = '\0';

        printf("Cabecera: %s\n", cabecera);

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMX25") == 0)
        {
            printf("Leida cabezera vmx25 \n");
            //leo version y tamano del codigo
            fread(&version, 1, 1, arch);
            printf("Version: %d\n", version);
            switch(version){
                case 1:{
                    printf("version 1 \n");
                    inicializarTablaRegistrosVersion1(mv, arch);
                    mv->version = 1;
                    break;
                }
                case 2:{
                    printf("Entro a version 2\n");
                    inicializarTablaRegistrosVersion2(mv, arch, tamPS);
                    mv->version = 2;
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

void cargarTodoMv(Tmv* mv, FILE* arch){
    fread(&mv->tamMemoria, 2, 1, arch);
    mv->memoria = (char*) malloc(mv->tamMemoria);
    fread(mv->registros, 4, 32, arch);
    fread(mv->tablaSegmentos, 4, 8, arch);
    fread(mv->memoria, 1, mv->tamMemoria, arch);
}


void leerArchivoVmi(Tmv* mv){
    unsigned char cabecera[5], version;
    FILE *arch;
    arch = fopen(mv->fileNameVmx, "rb");

    if (arch != NULL)
    {
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMI25") == 0)
        {
            //leo version y tamano del codigo
            fread(&version, sizeof(unsigned char), 1, arch);
            switch(version){
                case 1:{
                    cargarTodoMv(mv, arch);
                }
                default:{
                    printf("Version invalida");
                    exit(-1);
                    break;
                }
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