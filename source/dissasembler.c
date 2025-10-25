#include "mv.h"


void impKS(Tmv* mv){
    if(mv->tablaSegmentos){

    }
}



void impKS(Tmv* mv){
    if (mv->registros[KS] != -1){
        int aux  = obtenerHigh(mv->registros[KS]);
        int base = obtenerHigh(mv->tablaSegmentos[aux]); 
        int tam  = obtenerLow (mv->tablaSegmentos[aux]);
        int i = base;
        const int ancho_tab = 32; // mismo ancho que disassembler

        while (i < base + tam){
           

            int inicio = i;   // guardo inicio de la palabra
            int n = 0;       // largo de la palabra (en chars)

            // ---- IZQUIERDA: dirección ----
            int col = 0;
            col += printf(" [%04X] ", i);

            // ---- recorro palabra completa para contar n y mostrar hasta 6 bytes en hex ----
            while (i < base + tam && mv->memoria[i] != '\0'){
                unsigned char car = (unsigned char)mv->memoria[i];

                if (n < 6) {
                    col += printf("%02X ", car);
                } else if (n == 6) {
                    col += printf("..");   // hay más de 6 bytes -> trunc en la izquierda
                }
                n++;
                i++;
            }

            if (n <= 6 && mv->memoria[i] == '\0')
                col += printf("%02X ", (unsigned char)mv->memoria[i]);

            // ---- alineación de la barra usando lo impreso a la izquierda ----
            int espacios = ancho_tab - col;
            if (espacios < 1) espacios = 1;
            while (espacios--) printf(" ");

            // ---- DERECHA: palabra completa (todos los n chars) ----
            printf(" | \"");
            for (int j = 0; j < n; j++){
                unsigned char c = (unsigned char)mv->memoria[inicio + j];
                putchar((c >= 32 && c <= 126) ? (char)c : '.');
            }
            printf("\"\n");

            // ---- saltar '\0' si existe ----
            if (i < base + tam && mv->memoria[i] == '\0') i++;
        }
    }
}



void impNombreOperando(Tmv* mv, int ip, int tipo) {
    const int ancho = 18;
    char nombre[64];
    int num, low, high;

    switch (tipo) {
        case 1: {
            char sec = ((unsigned char)(mv->memoria[ip + 1]) >> 6); 
            char reg = mv->memoria[ip + 1] & 0x1F;
            if (sec == 0)      sprintf(nombre, "%s", nombreRegistros[reg]);
            else if (sec == 1) sprintf(nombre, "%cL", nombreRegistros[reg][1]);
            else if (sec == 2) sprintf(nombre, "%cH", nombreRegistros[reg][1]);
            else if (sec == 3) sprintf(nombre, "%cX", nombreRegistros[reg][1]);
            else               sprintf(nombre, "REG INVALIDO");
            break;
        }
        case 2: {
            high = (unsigned char)mv->memoria[ip + 1];
            low  = (unsigned char)mv->memoria[ip + 2];
            num  = (high << 8) | low;
            num  = (num << 16) >> 16;
            sprintf(nombre, "%d", num);
            break;
        }
        case 3: {
            char tamCelda = ((unsigned char)(mv->memoria[ip + 1]) >> 6); 
            char pref = '?';
            if (tamCelda == 0) 
                pref = 'l';
            else if (tamCelda == 2)
                pref = 'w';
            else if (tamCelda == 3) 
                pref = 'b';

            high = (unsigned char)mv->memoria[ip + 2];
            low  = (unsigned char)mv->memoria[ip + 3];
            num  = (high << 8) | low;
            num  = (num << 16) >> 16;

            char *reg = (char*)nombreRegistros[mv->memoria[ip + 1] & 0x1F];
            if (num != 0) 
                sprintf(nombre, "%c[%s%+d]", pref, reg, num);
            else          
                sprintf(nombre, "%c[%s]",     pref, reg);
            break;
        }
        default:
            nombre[0] = '\0';
            break;
    }

    int largo   = (int)strlen(nombre);
    int espacio = 18 - largo;
    if (espacio < 0) espacio = 0;
    printf("%*s%s", espacio, "", nombre);
}


void disassembler(Tmv* mv) {
    int i = obtenerHigh(mv->registros[CS]);
    int base = obtenerHigh(mv->tablaSegmentos[i]); // base seg código
    int tam  = obtenerLow(mv->tablaSegmentos[i]);  // tamaño seg código
    int entryPoint = obtenerLow(mv->registros[IP]) + base;
    int ip = base;
    

    // ancho de la línea completa antes de la barra
    const int ancho_tab = 32;

    //imprime K segment si existe


    //imprime K segment si existe
    impKS(mv);

    while (ip < tam + base) {
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



        if (ip == entryPoint)
            printf(">");
        else
            printf(" ");


        int largo = snprintf(izq, sizeof(izq), "[%04X] %02X ", base + ip, ins);

        if (opc == 0x0F || opc == 0x0E) { // STOP o RET
        if (opc == 0x0F || opc == 0x0E) { // STOP o RET
            int espacios = ancho_tab - largo;
            if (espacios < 1) espacios = 1;

            if (opc == 0x0F)
                printf("%s%*s| STOP\n", izq, espacios, "");
            else
                printf("%s%*s| RET\n", izq, espacios, "");

            if (opc == 0x0F)
                printf("%s%*s| STOP\n", izq, espacios, "");
            else
                printf("%s%*s| RET\n", izq, espacios, "");
            ip += 1;
        } else if (opc <= 0x08 || (opc >= 0x0B && opc <= 0x0D)) { // 1 operando
        } else if (opc <= 0x08 || (opc >= 0x0B && opc <= 0x0D)) { // 1 operando
            // swap
            top1 = top2;
            int pos = largo;
            for (int j = 0; j < top1; j++)
                pos += snprintf(izq + pos, sizeof(izq) - pos,
                                "%02X ", (unsigned char)mv->memoria[ip + 1 + j]);

            int espacios = ancho_tab - pos;
            if (espacios < 1) espacios = 1;
            printf("%s%*s| %-4s", izq, espacios, "", mnemonicos[opc]);
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