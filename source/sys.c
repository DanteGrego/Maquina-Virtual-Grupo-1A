#include "mv.h"

void imprimirBinario(unsigned int valorLeido, int tamCelda){
    printf(" 0b");
    unsigned int mascara = 0x80000000; 
    valorLeido <<= ((4 - tamCelda) * 8);
    if (valorLeido == 0)
        printf("0");
    else{
        int i = 0;
        while ((valorLeido & mascara) == 0){ // me posiciono en el primer 1
            valorLeido <<= 1; 
            i++;
        }
        while (i < tamCelda * 8){
            printf("%d", (valorLeido & mascara) != 0);
            valorLeido <<= 1; 
            i++;
        }
    }
}

void imprimirHexadecimal(unsigned int valorLeido, int tamCelda){
    printf(" 0x%x", valorLeido);
}

void imprimirOctal(unsigned int valorLeido, int tamCelda){
    printf(" 0o%o", valorLeido);
}

void imprimirCaracter(unsigned int valorLeido,int tamCelda){
    int i;
    char vecCaracteres[4];
    for (i = 0; i < 4; i++){
        unsigned char c = valorLeido & 0xFF;
        if(c < 32 || c > 126)
            c = '.';
        vecCaracteres[i] = c;
        valorLeido >>= 8;
    }
    printf(" ");
    for (i = tamCelda-1; i>=0; i--)
        printf("%c", vecCaracteres[i]);
}

void imprimirDecimal(unsigned int valorLeido, int tamCelda){
    printf(" %d", valorLeido);
}

int leerBinario(){
    char cadLeida[TAM_REGISTRO + 1]; //reservo lugar, sino da error
    int retorno = 0;

    scanf(" %s", cadLeida);
    fflush(stdout);

    int i = 0;
    while(cadLeida[i] == '0' || cadLeida[i] == '1'){
        retorno <<= 1;
        retorno |= (int)cadLeida[i] - 48;
        i++;
    }

    return retorno;
}

int leerHexadecimal(){
    unsigned int valor;
    scanf(" %x", &valor);
    return valor;
}

int leerOctal(){
    unsigned int valor;
    scanf(" %o", &valor);
    return valor;
}

int leerCaracter(){
    char valor;
    scanf(" %c", &valor);
    return valor;
}

int leerDecimal(){
    unsigned int valor;
    scanf(" %d", &valor);
    return valor;
}

void limpiarBuffers(){
    char c;
    //getchar agarra un caracter del buffer de entrada, agarro todos para vaciarlo
    while((c = getchar()) != '\n' && c != EOF){}
    //fflush limpia el buffer de salida
    fflush(stdout);
}

void sysLeer(Tmv* mv){
    int formato = mv->registros[EAX];
    int cantCeldas = obtenerLow(mv->registros[ECX]);
    int tamCelda = obtenerHigh(mv->registros[ECX]);
    int segmento = obtenerHigh(mv->registros[EDX]);
    for(int i = 0; i < cantCeldas; i++){
        int posActual = mv->registros[EDX] + i * tamCelda;
        int valorLeido, leido = 0, j = 0;
        printf("[%04X]: ", obtenerLow(obtenerDirFisica(mv, posActual)));
        char mascara = 0x10;
        while(!leido && j < CANT_FORMATOS){
            if((formato & mascara) != 0){
                valorLeido = pfuncionLectura[j]();
                limpiarBuffers();
                leido = 1;//una vez lee sale del ciclo y agarra el primer valor leido => se lee en el formato mas grande que hay
            }
            mascara >>= 1;
            j++;
        }

        escribirMemoria(mv, posActual, tamCelda, valorLeido, segmento);
    }
}

void sysEscribir(Tmv* mv){
    int formato = mv->registros[EAX];
    int cantCeldas = obtenerLow(mv->registros[ECX]);
    int tamCelda = obtenerHigh(mv->registros[ECX]);
    int segmento = obtenerHigh(mv->registros[EDX]);
    //printf("ECX: %x\n", mv->registros[ECX]);
    //printf("Sys escribir: \nformato: %d\ncantCeldas: %d\ntamCelda:%d\nsegmento:%d\n", formato, cantCeldas, tamCelda, segmento);
    for(int i = 0; i < cantCeldas; i++){
        int posActual = mv->registros[EDX] + i * tamCelda;
        printf("[%04X]:", obtenerLow(obtenerDirFisica(mv, posActual)));
        leerMemoria(mv, posActual, tamCelda, segmento); //cargo MBR
        int valorLeido = mv->registros[MBR]; //saco el valor leido del mbr y lo almaceno en una variable
        unsigned char mascara = 0x10;
        for(int j = 0; j < CANT_FORMATOS; j++){
            if((formato & mascara) != 0)
                pfuncionImpresion[j](valorLeido, tamCelda); //imprimo en el orden 0b 0x 0o 0c 0d con sus respectivas funciones
            mascara >>= 1;
        }
        printf("\n");
    }
}


void SYS(Tmv* mv, int operando){
    int valor = getValor(mv, operando); // valor decide que funcion del sistema se quiere utilizar   1. escritura   2. lectura
    
    switch(valor){
        case 1:{ // lectura
            sysLeer(mv);
            break;
        }
        case 2:{ // escritura
            sysEscribir(mv);
            break;
        }
        case 3:{
            sysStringRead(mv);
            break;
        }
        case 4:{
            sysStringWrite(mv);
            break;
        }
        case 7:{
            system("cls");
            break;
        }
        case 0xF:{
            sysBreakpoint(mv);
            break;
        }
        default:{
            //TODO que hace si sys tiene operando erroneo?
        }
    }
}

void generarArchivoImagen(Tmv* mv){
    FILE* arch;

    arch = fopen(mv->fileNameVmi, "wb");
    if(arch == NULL){
        printf("No se pudo abrir el archivo .vmi: %s", mv->fileNameVmi);//(debug)
    }else{
        //header
        char identificador[] = "VMI25";
        fwrite(identificador, 1, 5, arch);

        char version = 1;
        fwrite(&version, 1, 1, arch);
        
        char tamMemoria[] = {(mv->tamMemoria & 0x0000FF00) >> 8, mv->tamMemoria & 0x000000FF};
        fwrite(tamMemoria, 1, 2, arch);

        //registros
        for(int i = 0; i < CANT_REGISTROS; i++){
            char registro[] = {(mv->registros[i]>>24)&0x000000FF,
                                (mv->registros[i]>>16)&0x000000FF,
                                (mv->registros[i]>>8)&0x000000FF,
                                (mv->registros[i])&0x000000FF};
            fwrite(registro, 1, 4, arch);
        }
        
        //tabla segmentos
        for(int i = 0; i < CANT_SEGMENTOS; i++){
            char segmento[] = {(mv->tablaSegmentos[i]>>24)&0x000000FF,
                                (mv->tablaSegmentos[i]>>16)&0x000000FF,
                                (mv->tablaSegmentos[i]>>8)&0x000000FF,
                                (mv->tablaSegmentos[i])&0x000000FF};
            fwrite(segmento, 1, 4, arch);
        }
        
        //memoria
        fwrite(mv->memoria, 1, mv->tamMemoria, arch);

        fclose(arch);
    }
}

void sysBreakpoint(Tmv* mv){
    if(mv->fileNameVmi != NULL){
        generarArchivoImagen(mv);
        mv->modoDebug = 1;
    }
}

void sysStringRead(Tmv* mv){
    int caracteresMaximo = obtenerLow(mv->registros[ECX]);//CX
    int posActual = mv->registros[EDX];
    int segmento = obtenerHigh(mv->registros[EDX]);
    char stringLeido[1024];

    scanf("%s", stringLeido);

    int i = 0;

    while(stringLeido[i] != '\0' && (i < caracteresMaximo || caracteresMaximo == -1)){
        escribirMemoria(mv, posActual + i, 1, stringLeido[i], segmento);
        i++;
    }
    escribirMemoria(mv, posActual + i, 1, '\0', segmento);
}


void sysStringWrite(Tmv* mv){
    int posActual = mv->registros[EDX];
    int segmento = obtenerHigh(mv->registros[EDX]);
    int i = 1;
    char c;

    leerMemoria(mv, posActual, 1, segmento);
    c = mv->registros[MBR];

    while(c != '\0'){
        printf("%c", c);
        leerMemoria(mv, posActual + i, 1, segmento);
        c = mv->registros[MBR];
        i++;
    }
}