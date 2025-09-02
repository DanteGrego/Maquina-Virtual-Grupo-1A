#include <stdio.h>
#include "mv.h"

int vmx(int argc, char* argv[]){
    
    
    return 0;
}

int combinarHighLow(int bytesHigh, int bytesLow){
    return (bytesHigh << 16) | (bytesLow & 0x0000FFFF);
}

int obtenerHigh(int bytes){
    int res = 0;
    bytes >> 16;
    res = bytes & 0x0000FFFF;
    return res;
}

int obtenerLow(int bytes){
    return bytes & 0x0000FFFF;
}

void leerArch(Tmv* mv, char* nomArch){
    char x;
    int i = 0;
    FILE * arch;
    arch = fopen(nomArch, "rb");
    
    //LEER ENCABEZADO ANTES

    while(fscanf("%c",&x) != 0){
        mv->memoria[i] = x;
        i++;
    }

}