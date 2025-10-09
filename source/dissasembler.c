#include "mv.h"


void impKS(Tmv* mv){
    if(mv->tablaSegmentos){

    }
}


// disassembler: imprime el operando
void impNombreOperando(Tmv* mv, int ip, int tipo) {
    const int ancho = 18;           // ancho fijo de la columna del operando
    char nombre[64];                // texto final del operando
    int num, low, high;             // valores temporales

    switch (tipo) {
        case 1: { // registro
            // encuentro el sector de registros y lo imprimo segun corresponda
            char sec = ((unsigned char)(mv->memoria [ip+1]) >> 6); 
            char reg = mv->memoria [ip+1];
            if(sec == 0)
                printf("%s",nombreRegistros[reg]);
            else if(sec == 1)
                printf("%cL",nombreRegistros[reg][1]);
            else if(sec == 2)
                printf("%cH",nombreRegistros[reg][1]);
            else if(sec == 3)
                printf("%cX",nombreRegistros[reg][1]);
            else
                printf("REGISTRO INVALIDO");
            break;
        }
        case 2: { // inmediato (2 bytes)
            // saco el valor con los dos bytes que ocupa el inmediato y propago signo
            high = (unsigned char)mv->memoria[ip + 1];
            low  = (unsigned char)mv->memoria[ip + 2];
            num  = (high << 8) | low;
            num  = (num << 16) >> 16;
            snprintf(nombre, sizeof(nombre), "%d", num);
            break;
        }
        case 3: { // memoria: [registro + offset]
            // imprimo registro como caso 1 y calculo offset
            char tamCelda = ((unsigned char)(mv->memoria [ip+1]) >> 6); 

            if(tamCelda == 0)
                printf("l");
            else if (tamCelda == 2)
                printf("w");
            else if (tamCelda == 3)
                printf("b");

            //hay que modificar
            high = (unsigned char)mv->memoria[ip + 2];
            low  = (unsigned char)mv->memoria[ip + 3];
            num  = (high << 8) | low;
            num  = (num << 16) >> 16;
            // imprimo según el signo el offset (o sin offset si es 0)
            if (num != 0)
                snprintf(nombre, sizeof(nombre), "[%s%+d]", nombreRegistros[mv->memoria[ip + 1]], num);
            else
                snprintf(nombre, sizeof(nombre), "[%s]", nombreRegistros[mv->memoria[ip + 1]]);
            break;
        }
        default: { // tipo inválido
            nombre[0] = '\0';
            break;
        }
    }

    int largo   = (int)strlen(nombre);      // longitud del texto generado
    int espacio = ancho - largo;            // relleno de espacios a la izquierda
    if (espacio < 0) espacio = 0;
    printf("%*s%s", espacio, "", nombre);
}

void disassembler(Tmv* mv) {
    int base = obtenerHigh(mv->tablaSegmentos[0]); // base seg código
    int tam  = obtenerLow(mv->tablaSegmentos[0]);  // tamaño seg código
    int entryPoint = IP;
    int ip = 0;
    

    // ancho de la línea completa antes de la barra
    const int ancho_tab = 32;

    //imprime K segment si existe


    while (ip < tam) {
        unsigned char ins  = mv->memoria[ip];
        // como mv con registros pero acá son variables temporales
        unsigned char opc  = ins & 0x1F;
        unsigned char top2 = (ins >> 6) & 0x03;
        unsigned char top1 = (ins >> 4) & 0x03;

        char izq[256];

        if (ip == entryPoint)
            printf(">");
        else
            printf(" ");


        int largo = snprintf(izq, sizeof(izq), "[%04X] %02X ", base + ip, ins);

        if (opc == 0x0F || opc == 0x0E) { // STOP o RET
            int espacios = ancho_tab - largo;
            if (espacios < 1) espacios = 1;

            if (opc == 0x0F)
                printf("%s%*s| STOP\n", izq, espacios, "");
            else
                printf("%s%*s| RET\n", izq, espacios, "");
            ip += 1;
        } else if (opc <= 0x08 || (opc >= 0x0B && opc <= 0x0D)) { // 1 operando
            // swap
            top1 = top2;
            int pos = largo;
            for (int j = 0; j < top1; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos,
                                "%02X ", (unsigned char)mv->memoria[ip + 1 + j]);

            int espacios = ancho_tab - pos;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| %s ", izq, espacios, "", mnemonicos[opc]);
            impNombreOperando(mv, ip, top1);
            printf("\n");
            ip += 1 + top1;

        } else if (opc >= 0x10 && opc <= 0x1F) { // 2 operandos
            int aux = top1 + top2;
            int pos = largo;
            for (int j = 0; j < aux; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos,
                                "%02X ", (unsigned char)mv->memoria[ip + 1 + j]);

            int espacios = ancho_tab - pos;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| %s ", izq, espacios, "", mnemonicos[opc]);
            impNombreOperando(mv, ip + top2, top1);
            printf(", ");
            impNombreOperando(mv, ip, top2);
            printf("\n");
            ip += 1 + aux;

        } else { // UNKNOWN
            int espacios = ancho_tab - largo;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| UNKNOWN\n", izq, espacios, "");
            ip += 1;
        }
    }
}