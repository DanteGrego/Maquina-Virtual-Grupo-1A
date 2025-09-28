#include "mv.h"

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