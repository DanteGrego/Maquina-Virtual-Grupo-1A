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


void SYS(Tmv* mv, int operando){
    int valor = getValor(mv, operando); // valor decide que funcion del sistema se quiere utilizar   1. escritura   2. lectura
    int formato = mv->registros[EAX];
    int cantCeldas = obtenerLow(mv->registros[ECX]);
    int tamCelda = obtenerHigh(mv->registros[ECX]);

    if(tamCelda <= 4 && tamCelda > 0 && tamCelda != 3 && formato > 0)
        switch(valor){
            case 1:{ // lectura
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

                    escribirMemoria(mv, posActual, tamCelda, valorLeido, mv->registros[DS]);
                }
                break;
            }
            case 2:{ // escritura
                for(int i = 0; i < cantCeldas; i++){
                    int posActual = mv->registros[EDX] + i * tamCelda;
                    printf("[%04X]:", obtenerLow(obtenerDirFisica(mv, posActual)));
                    leerMemoria(mv, posActual, tamCelda, mv->registros[DS]); //cargo MBR
                    int valorLeido = mv->registros[MBR]; //saco el valor leido del mbr y lo almaceno en una variable
                    unsigned char mascara = 0x10;
                    for(int j = 0; j < CANT_FORMATOS; j++){
                        if((formato & mascara) != 0)
                            pfuncionImpresion[j](valorLeido, tamCelda); //imprimo en el orden 0b 0x 0o 0c 0d con sus respectivas funciones
                        mascara >>= 1;
                    }
                    printf("\n");
                }
                break;
            }
            default:{
                //TODO que hace si sys tiene operando erroneo?
            }
        }
}