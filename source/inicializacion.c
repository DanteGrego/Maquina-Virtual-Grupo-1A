#include "mv.h"

void cargoTamMemoria(Tmv* mv, char* argumento){
    int tamMemoria = 0;
    int i = 2;//salteo "m="

    while(argumento[i] != '\0'){
        tamMemoria *= 10;
        tamMemoria += argumento[i] - '0';
        i++;
    }

    tamMemoria *= 1024;//M esta en KiB

    mv->tamMemoria = tamMemoria;
}

void registrarSegmento(Tmv* mv, int* baseSegmento, int tamSegmento, int* indiceTabla, int segmento){
    if(tamSegmento > 0){
        if((*baseSegmento) + tamSegmento > mv->tamMemoria){
            printf("\nMemoria insuficiente\n");
            exit(-1);
        }
        mv->registros[segmento] = (*indiceTabla) << 16;
        mv->tablaSegmentos[*indiceTabla] = combinarHighLow(*baseSegmento, tamSegmento);
        (*baseSegmento) += tamSegmento;
        (*indiceTabla)++;
    }else
        mv->registros[segmento] = -1;
}

void inicializarTablaRegistros(Tmv* mv, FILE* arch, int tamPS){
    int tamSegmentos[CANT_SEGMENTOS], baseSegmento, cantSegmentosLeidos, indiceTabla, entryPoint;
    unsigned char lecturaTams[CANT_SEGMENTOS][2], version;

    baseSegmento = 0;
    indiceTabla = 0;
    registrarSegmento(mv, &baseSegmento, tamPS, &indiceTabla, PS);

    fread(&version, 1, 1, arch);

    switch(version){
        case 1:{
            cantSegmentosLeidos = 1;
            break;
        }
        case 2:{
            cantSegmentosLeidos = 5;
            break;
        }
        default:{
            printf("\nVersion invalida\n");
            exit(-1);
        }
    }

    fread(lecturaTams, 1, cantSegmentosLeidos * 2, arch);

    for(int i = 0; i < CANT_SEGMENTOS; i++)
        tamSegmentos[i] = (lecturaTams[i][0] << 8) + lecturaTams[i][1];

    if(cantSegmentosLeidos >= 5 && tamSegmentos[4] > 0){//KS
        registrarSegmento(mv, &baseSegmento, tamSegmentos[4], &indiceTabla, KS); 
        cantSegmentosLeidos--;  
    }

    for(int i = 0; i < cantSegmentosLeidos; i++)
        registrarSegmento(mv, &baseSegmento, tamSegmentos[i], &indiceTabla, CS+i);

    entryPoint = 0;
    if(version == 1){
        registrarSegmento(mv, &baseSegmento, TAM_MEMORIA - baseSegmento, &indiceTabla, DS);
        for(int i = ES; i <= PS; i++)
            mv->registros[i] = -1;
    }
    else if(version == 2){
        unsigned char entryPointLeido[2];
        fread(entryPointLeido, 1, 2, arch);
        entryPoint = (entryPointLeido[0] << 8) + entryPointLeido[1];
    }

    mv->registros[IP] = mv->registros[CS] + entryPoint;

    //dejo en -1 el resto de lugares no usados en la tabla
    for(int i = indiceTabla; i < CANT_SEGMENTOS; i++)
        mv->tablaSegmentos[i] = -1;
    
    if(mv->registros[SS] > 0){
        int tamPila = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[SS])]);
        mv->registros[SP] = mv->registros[SS] + tamPila;
    }
}

void cargarCS(Tmv* mv, FILE* arch){
    int posMemoria = obtenerDirFisica(mv, mv->registros[CS]);
    int tamCS = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]);
    fread(mv->memoria + posMemoria, 1, tamCS, arch);
}

void cargarKS(Tmv* mv, FILE* arch){
    int tablaKS = mv->tablaSegmentos[obtenerHigh(mv->registros[KS])];
    int baseKS = obtenerHigh(tablaKS);
    int tamKS = obtenerLow(tablaKS);

    fread(mv->memoria+baseKS, 1, tamKS, arch);
}


void leerArchivoVmx(Tmv *mv, int tamPS)
{
    unsigned char cabecera[6], version;
    FILE *arch;

    arch = fopen(mv->fileNameVmx, "rb");

    if (arch != NULL)
    {
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);
        cabecera[5] = '\0';

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMX25") == 0)
        {
            inicializarTablaRegistros(mv, arch, tamPS);

            cargarCS(mv, arch);
            
            if(mv->registros[KS] >= 0)
                cargarKS(mv, arch);

            /*
            for(int i = CS; i < PS; i++){
                printf("\n\n");
                printf("Tabla: %x\n", mv->tablaSegmentos[i-CS]);
                printf("Registro: %x\n", mv->registros[i]);
            }
            */
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

int bytes4AInt(unsigned char bytes[4]){
    unsigned int res = 0;
    for(int i = 0; i < 4; i++)
        res += bytes[i] << (3-i) * 8;
    return res;
}

void cargarTodoMv(Tmv* mv, FILE* arch){
    unsigned char lecturaTams[4];
    fread(lecturaTams, 1, 2, arch);
    mv->tamMemoria = ((lecturaTams[0] << 8) + lecturaTams[1]) * 1024;
    printf("Tam: %x\n", mv->tamMemoria);
    mv->memoria = (char*) malloc(mv->tamMemoria);

    for(int i = 0; i < CANT_REGISTROS; i++){
        fread(lecturaTams, 1, 4, arch);
        mv->registros[i] = bytes4AInt(lecturaTams);
        printf("registro %i: %x\n", i, mv->registros[i]);
    }

    for(int i = 0; i < CANT_SEGMENTOS; i++){
        fread(lecturaTams, 1, 4, arch);
        mv->tablaSegmentos[i] = bytes4AInt(lecturaTams);
        printf("tabla %i: %x\n", i, mv->tablaSegmentos[i]);
    }
    
    fread(mv->memoria, 1, mv->tamMemoria, arch);
}


void leerArchivoVmi(Tmv* mv){
    unsigned char cabecera[6], version;
    FILE *arch;
    arch = fopen(mv->fileNameVmi, "rb");

    if (arch != NULL)
    {
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);
        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMI25") == 0)
        {
            //leo version y tamano del codigo
            fread(&version, 1, 1, arch);
            switch(version){
                case 1:{
                    cargarTodoMv(mv, arch);
                    break;
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