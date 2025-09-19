#include "mv.h"

//dissasembler: imprime el operando
void impNombreOperando(const Tmv* mv, int ip, int tipo) {
    int num, low, high;
    switch (tipo) {
        case 1: { // registro
            //saco el registro con el codigo que tiene en memoria
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            break;
        }
        case 2: { // inmediato (2 bytes)
            //saco el valor con los dos bytes que ocupa el inmediato y propago signo
            high = (unsigned char) mv->memoria[ip+1];
            low  = (unsigned char) mv->memoria[ip+2];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            printf("%d (0x%X)", num, num);
            break;
        }
        case 3: { // memoria: [registro + offset]
            printf("[");
            //imprimo registro como caso 1
            printf("%s", nombreRegistros[mv->memoria[ip+1]]);
            //obtengo offset como el inmediato en caso 2
            high = (unsigned char) mv->memoria[ip+2];
            low  = (unsigned char) mv->memoria[ip+3];
            num  = (high << 8) | low;
            num = (num << 16) >> 16;
            //imprimo segun el signo el offset
            if (num > 0)      printf(" + %d", num);
            else if (num < 0) printf(" %d", num);
            printf("]");
            break;
        }
        default: /* tipo invalido */ break;
    }
}

void disassembler(const Tmv* mv) {
    int base = obtenerHigh(mv->tablaSegmentos[0]); // base seg código
    int tam = obtenerLow(mv->tablaSegmentos[0]);  // tamaño seg código
    int ip = 0;

    //ancho de la linea
    const int ancho_tab = 32;

    while (ip < tam) {
        unsigned char ins = mv->memoria[ip];
        //como mv con registros pero aca son variables temporales
        unsigned char opc = ins & 0x1F;
        unsigned char top2 = (ins >> 6) & 0x03;
        unsigned char top1 = (ins >> 4) & 0x03;


        char izq[256];
        int len = snprintf(izq, sizeof(izq), "[%04X] %02X ", base + ip, ins);

        if (opc == 0x0F) { // STOP
            int espacios = ancho_tab - len;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| STOP\n", izq, espacios, "");
            ip += tam;

        } else if (opc <= 0x08) { // 1 operando
            //swap
            top1 = top2;
            int pos = len;
            for (int j = 0; j < top1; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02X ", (unsigned char) mv->memoria[ip + 1 + j]);

            int espacios = ancho_tab - pos;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| %s ", izq, espacios, "", mnemonicos[opc]);
            impNombreOperando(mv, ip, top1);
            printf("\n");
            ip += 1 + top1;
        } else if (opc >= 0x10 && opc <= 0x1F) { // 2 operandos
            int aux = top1 + top2;
            int pos = len;
            for (int j = 0; j < aux; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos, "%02X ", (unsigned char) mv->memoria[ip + 1 + j]);

            int spaces = ancho_tab - pos;
            if (spaces < 1) spaces = 1;
            printf("%s%*s| %s ", izq, spaces, "", mnemonicos[opc]);
            impNombreOperando(mv, ip + top2, top1);
            printf(", ");
            impNombreOperando(mv, ip, top2);
            printf("\n");
            ip += 1 + aux;
        } else {
            int espacios = ancho_tab - len;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| UNKNOWN\n", izq, espacios, "");
            ip += 1;
        }
    }
}