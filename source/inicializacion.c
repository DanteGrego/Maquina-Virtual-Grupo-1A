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
    //printf("tamSegmento: %d, segmento: %d, indiceTabla: %d\n", tamSegmento, segmento, indiceTabla);
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
    if(version == 1)
        registrarSegmento(mv, &baseSegmento, TAM_MEMORIA - baseSegmento, &indiceTabla, DS);
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

    /*
    for(int i = 0; i < CANT_SEGMENTOS; i++){
        printf("Tabla %d: %x, %x\n", i, obtenerHigh(mv->tablaSegmentos[i]), obtenerLow(mv->tablaSegmentos[i]));
        printf("Registro: %x\n", mv->registros[CS+i]);
    }
    */
}

void cargarCS(Tmv* mv, FILE* arch){
    int posMemoria = obtenerDirFisica(mv, mv->registros[CS]);
    int tamCS = obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]);
    //TODO fijarse si anda
    fread(mv->memoria + posMemoria, 1, tamCS, arch);
    //printf("Leido el codigo: tamCS: %x\n", obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]));
    //for(int i = 0; i < obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[CS])]); i++){
    //    printf("%x: %x \n", posMemoria+i, (unsigned char)mv->memoria[posMemoria+i]);
    //}
}

void cargarKS(Tmv* mv, FILE* arch){
    int tablaKS = mv->tablaSegmentos[obtenerHigh(mv->registros[KS])];
    int baseKS = obtenerHigh(tablaKS);
    int tamKS = obtenerLow(tablaKS);

    fread(mv->memoria+baseKS, 1, tamKS, arch);

    /*
    printf("Leido el const: tamKS: %x\n", obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[KS])]));
    for(int i = 0; i < obtenerLow(mv->tablaSegmentos[obtenerHigh(mv->registros[KS])]); i++){
        printf("%x: %x \n", baseKS+i, (unsigned char)mv->memoria[baseKS+i]);
    }
    */
}


void leerArchivoVmx(Tmv *mv, int tamPS)
{
    unsigned char cabecera[6], version;
    FILE *arch;

    arch = fopen(mv->fileNameVmx, "rb");

    //printf("Se abrio el archivo\n");

    if (arch != NULL)
    {
        //printf("Se abrio el vmx\n");
        //leo cabecera
        fread(cabecera, sizeof(unsigned char), TAM_IDENTIFICADOR, arch);
        cabecera[5] = '\0';

        //printf("Cabecera: %s\n", cabecera);

        //me fijo si el archivo es valido por la cabecera
        if (strcmp(cabecera, "VMX25") == 0)
        {
            inicializarTablaRegistros(mv, arch, tamPS);

            cargarCS(mv, arch);
            
            if(mv->registros[KS] >= 0)
                cargarKS(mv, arch);
            //printf("Leida cabezera vmx25 \n");
            //leo version y tamano del codigo
            //fread(&version, 1, 1, arch);
            //printf("Version: %d\n", version);
            /*
            switch(version){
                case 1:{
                    //printf("version 1 \n");
                    inicializarTablaRegistrosVersion1(mv, arch);
                    mv->version = 1;
                    break;
                }
                case 2:{
                    //printf("Entro a version 2\n");
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
            */
            //se supone que se sale con el archivo ya leida la cabecera
            /*
            cargarCS(mv, arch);

            if(mv->registros[KS] > 0)
                cargarKS(mv, arch);
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