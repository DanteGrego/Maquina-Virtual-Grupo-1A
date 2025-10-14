#include "mv.h"

const char* nombreRegistros[] = {
    [0]  = "LAR",
    [1]  = "MAR",
    [2]  = "MBR",
    [3]  = "IP",
    [4]  = "OPC",
    [5]  = "OP1",
    [6]  = "OP2",
    [7] = "SP",
    [8] = "BP",
    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",
    [16] = "AC",
    [17] = "CC",
    [26] = "CS",
    [27] = "DS",
    [28] = "ES",
    [29] = "SS",
    [30] = "KS",
    [31] = "PS"
};

const char* mnemonicos[CANT_REGISTROS] = {
    [0x00] = "SYS",
    [0x01] = "JMP",
    [0x02] = "JZ",
    [0x03] = "JP",
    [0x04] = "JN",
    [0x05] = "JNZ",
    [0x06] = "JNP",
    [0x07] = "JNN",
    [0x08] = "NOT",
    [0x0B] = "PUSH",
    [0x0C] = "POP",
    [0x0D] = "CALL",
    [0x0F] = "STOP",
    [0x10] = "MOV",
    [0x11] = "ADD",
    [0x12] = "SUB",
    [0x13] = "MUL",
    [0x14] = "DIV",
    [0x15] = "CMP",
    [0x16] = "SHL",
    [0x17] = "SHR",
    [0x18] = "SAR",
    [0x19] = "AND",
    [0x1A] = "OR",
    [0x1B] = "XOR",
    [0x1C] = "SWAP",
    [0x1D] = "LDL",
    [0x1E] = "LDH",
    [0x1F] = "RND"
};
const int vectorTraductorIndicesCOperacion[32] = {
    [0x00] = 0,
    [0x01] = 1,
    [0x02] = 2,
    [0x03] = 3,
    [0x04] = 4,
    [0x05] = 5,
    [0x06] = 6,
    [0x07] = 7,
    [0x08] = 8,
    [0x09] = -1,
    [0x0A] = -1,
    [0x0B] = 9, // push
    [0x0C] = 10, // pop
    [0x0D] = 11, // call
    [0x0E] = 1, //ret
    [0x0F] = 0, //stop
    [0x10] = 0,
    [0x11] = 1,
    [0x12] = 2,
    [0x13] = 3,
    [0x14] = 4,
    [0x15] = 5,
    [0x16] = 6,
    [0x17] = 7,
    [0x18] = 8,
    [0x19] = 9,
    [0x1A] = 1,
    [0x1B] = 1,
    [0x1C] = 1,
    [0x1D] = 1,
    [0x1E] = 1,
    [0x1F] = 15
};
/*
// en desuso
const char* formatosLectura[CANT_FORMATOS - 1] = {" %d", " %c", " %o", " %x"};
const char* formatosEscritura[CANT_FORMATOS - 1] = {" %d", " %c", " 0o%o", " 0x%x"}; //char va aparte
*/
const int (*pfuncionLectura[CANT_FORMATOS])() = {
    [0] = &leerBinario,
    [1] = &leerHexadecimal,
    [2] = &leerOctal,
    [3] = &leerCaracter,
    [4] = &leerDecimal
};

const void (*pfuncionImpresion[CANT_FORMATOS])(unsigned int, int) = {
    [0] = &imprimirBinario,
    [1] = &imprimirHexadecimal,
    [2] = &imprimirOctal,
    [3] = &imprimirCaracter,
    [4] = &imprimirDecimal
};

const void (*pfuncion0Param[CANT_FUNCIONES_0_PARAM])(Tmv *mv) = {
    [0x00] = &STOP,
    [0x01] = &RET
};

const void (*pfuncion1Param[CANT_FUNCIONES_1_PARAM])(Tmv *mv, int) = {
    [0x00] = &SYS,
    [0x01] = &JMP,
    [0x02] = &JZ,
    [0x03] = &JP,
    [0x04] = &JN,
    [0x05] = &JNZ,
    [0x06] = &JNP,
    [0x07] = &JNN,
    [0x08] = &NOT,
    [0x09] = &PUSH,
    [0x0A] = &POP,
    [0x0B] = &CALL
};

const void (*pfuncion2Param[CANT_FUNCIONES_2_PARAM])(Tmv *mv, int, int) = {
    [0x00] = &MOV,
    [0x01] = &ADD,
    [0x02] = &SUB,
    [0x03] = &MUL,
    [0x04] = &DIV,
    [0x05] = &CMP,
    [0x06] = &SHL,
    [0x07] = &SHR,
    [0x08] = &SAR,
    [0x09] = &AND,
    [0x0a] = &OR,
    [0x0b] = &XOR,
    [0x0c] = &SWAP,
    [0x0d] = &LDL,
    [0x0e] = &LDH,
    [0x0f] = &RND
};