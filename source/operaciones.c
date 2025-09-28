#include "mv.h"

void actualizarCC(Tmv *mv, int valor){
    //actualizo NZ de CC, 1er y 2do bit si es negativo o nulo respectivamente
    if(valor < 0)
        mv->registros[CC] |= 0x80000000;
    else
        mv->registros[CC] &= 0x7FFFFFFF;//~(0x80000000)

    if(valor == 0){
        mv->registros[CC] |= 0x40000000;
    }
    else
        mv->registros[CC] &= 0xbFFFFFFF;//~(0x40000000)
}


//op1 = op2
void MOV (Tmv *mv, int op1, int op2){
    int valor = getValor(mv, op2);
    setValor(mv, op1, valor);
}

//op1 = op1 + op2
void ADD (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 + valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 - op2
void SUB (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 * op2
void MUL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 * valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 / op2
//ac = resto de la division
void DIV (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    if(valor2 == 0){
        printf("Error, division por 0");
        exit(-1);
    }
    int cociente = valor1/valor2;
    int resto = valor1%valor2;
    actualizarCC(mv, cociente);
    mv->registros[AC] = resto;
    setValor(mv, op1, cociente);
}

//actualizo NZ con op1 - op2
void CMP(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 - valor2;
    actualizarCC(mv, valor);
}

//shifteo left op1 op2 veces
void SHL (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 << valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//shifteo logico right op1 op2 veces
void SHR(Tmv *mv, int op1, int op2) {
    unsigned int valor1 = (unsigned int)getValor(mv, op1);
    unsigned int valor2 = (unsigned int)getValor(mv, op2);

    unsigned int resultado = valor1 >> valor2;

    actualizarCC(mv, resultado);
    setValor(mv, op1, (int)resultado);
}

//shifteo aritmetico op1 op2 veces
void SAR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 >> valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 & op2
void AND (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 & valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 | op2
void OR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 | valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op1 ^ op2
void XOR (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    int valor = valor1 ^ valor2;
    actualizarCC(mv, valor);
    setValor(mv, op1, valor);
}

//op1 = op2
//op2 = op1
void SWAP (Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);
    setValor(mv, op1, valor2);
    setValor(mv, op2, valor1);
}

//cargo los dos bytes menos significantes de op1 con los de op2
void LDL(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0xFFFF0000;
    valor2 &= 0X0000FFFF;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

//cargo los dos bytes mas significantes de op1 con los de op2
void LDH(Tmv *mv, int op1, int op2){
    int valor1 = getValor(mv, op1);
    int valor2 = getValor(mv, op2);

    valor1 &= 0X0000FFFF;
    valor2 &= 0X0000FFFF;
    valor2 <<= 16;

    valor1 = valor1 | valor2;
    setValor(mv,op1,valor1);
}

//op1 = random >= 0 y < op2
void RND(Tmv *mv, int op1, int op2){
    srand(time(NULL));
    int valor2 = getValor(mv, op2);
    int valor1 = rand() % (valor2);

    setValor(mv,op1,valor1);
}

int isN(Tmv* mv){
    return mv->registros[CC] < 0;
}

int isZ(Tmv* mv){
    return (mv->registros[CC] << 1) < 0;
}

//TODO llamarlo operando, no direccion
//salto siempre a la direccion
void JMP(Tmv *mv, int direccion){
    int valor = getValor(mv, direccion);
    mv->registros[IP] = valor;
}

//salto si esta z
void JZ(Tmv *mv, int direccion){
    if(isZ(mv) && !isN(mv))
        JMP(mv, direccion);
}

//salto si no esta zs
void JNZ(Tmv *mv, int direccion){
    if(!isZ(mv))
        JMP(mv, direccion);
}

//salto si esta n
void JN(Tmv *mv, int direccion){
    if(isN(mv) && !isZ(mv))
        JMP(mv, direccion);
}

//salto si no esta n
void JNN(Tmv *mv, int direccion){
    if(!isN(mv))
        JMP(mv, direccion);
}

//salto si no esta n ni z
void JP(Tmv *mv, int direccion){
    if(!isZ(mv) && !isN(mv))
        JMP(mv, direccion);
}

//salto si esta z o esta n
void JNP(Tmv *mv, int direccion){
    if(!isZ(mv) && isN(mv) || isZ(mv) && !isN(mv))
        JMP(mv, direccion);
}

//op1 = ~op1
void NOT(Tmv *mv,int op1){
    int valor1 = getValor(mv, op1);
    valor1 ^= 0xFFFFFFFF;
    actualizarCC(mv,valor1);
    setValor(mv, op1, valor1);
}

//paro el programa, mando IP fuera del CS (posicion -1)
void STOP(Tmv *mv){
    mv->registros[IP] = -1;
}
