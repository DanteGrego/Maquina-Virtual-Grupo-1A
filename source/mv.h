#ifndef MV_H
#define MV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>

//tamanos y cantidades de cosas
#define TAM_MEMORIA 16384 //16 KiB en Bytes
#define TAM_REGISTRO 32 //32 bits
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 2
#define CANT_FORMATOS 5 // 0b, 0x, 0o 0c 0d
#define TAM_IDENTIFICADOR 5 //del archivo, el "VMX25"
#define CANT_FUNCIONES_0_PARAM 1
#define CANT_FUNCIONES_1_PARAM 9
#define CANT_FUNCIONES_2_PARAM 16

//registro - codigo
#define LAR 0
#define MAR 1
#define MBR 2
#define IP 3
#define OPC 4
#define OP1 5
#define OP2 6
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15
#define AC 16
#define CC 17
#define CS 26
#define DS 27

typedef struct Tmv{
    char memoria[TAM_MEMORIA];
    char *fileNameVmi, *fileNameVmx;
    int registros[CANT_REGISTROS];
    int tamMemoria;
    int tablaSegmentos[CANT_SEGMENTOS];
    char modoDebug;//0: normal, '1': debug
}Tmv;

extern const char* nombreRegistros[];
extern const char* mnemonicos[];
extern const char* formatosLectura[];
extern const char* formatosEscritura[];


//prototipos
//cuentas con bytes
int  combinarHighLow(int bytesHigh, int bytesLow);
int  obtenerHigh(int bytes);
int  obtenerLow(int bytes);
char obtengoTipoOperando(int bytes);

//inicializacion
void leerArch(Tmv *mv, char* nomArch);
void cargarTablaSegmentos(Tmv* mv, int tamCodigo);
void inicializarRegistros(Tmv *mv);

//cuentas entre direcciones
int  obtenerDirLogica(Tmv* mv, int valor);
int  obtenerDirFisica(Tmv *mv, int dirLogica);

//lectura y ejecucion de instrucciones
void leerInstruccion(Tmv* mv);
void ejecutarInstruccion(Tmv *mv);
int seguirEjecutando(Tmv* mv);
int  getValor(Tmv *mv, int bytes);
void setValor(Tmv *mv, int operando, int valor);
//manipulacion de memoria
void leerMemoria(Tmv* mv, int dirLogica, int cantBytes, int segmento);
int leerValMemoria(Tmv *mv, int cantBytes, int posFisica);
void escribirMemoria(Tmv *mv, int dirLogica, int cantBytes, int valor, int segmento);

//funciones para las operaciones
void actualizarCC(Tmv* mv, int valor);
int isN(Tmv* mv);
int isZ(Tmv* mv);
void limpiarBuffers();
void imprimirBinario(unsigned int valorLeido, int tamCelda);
void imprimirHexadecimal(unsigned int valorLeido, int tamCelda);
void imprimirOctal(unsigned int valorLeido, int tamCelda);
void imprimirCaracter(unsigned int valorLeido,int tamCelda);
void imprimirDecimal(unsigned int valorLeido, int tamCelda);
int leerBinario();
int leerHexadecimal();
int leerOctal();
int leerCaracter();
int leerDecimal();
void sysBreakpoint(Tmv* mv);

//operaciones 2 parametros
void MOV(Tmv* mv, int op1, int op2);
void ADD(Tmv* mv, int op1, int op2);
void SUB(Tmv* mv, int op1, int op2);
void MUL(Tmv* mv, int op1, int op2);
void DIV(Tmv* mv, int op1, int op2);
void CMP(Tmv* mv, int op1, int op2);
void SHL(Tmv* mv, int op1, int op2);
void SHR(Tmv* mv, int op1, int op2);
void SAR(Tmv* mv, int op1, int op2);
void AND(Tmv* mv, int op1, int op2);
void OR(Tmv* mv, int op1, int op2);
void XOR(Tmv* mv, int op1, int op2);
void SWAP(Tmv* mv, int op1, int op2);
void LDL(Tmv* mv, int op1, int op2);
void LDH(Tmv* mv, int op1, int op2);
void RND(Tmv* mv, int op1, int op2);
//operaciones 1 parametro
void SYS(Tmv* mv, int operando);
void JMP(Tmv* mv, int operando);
void JZ(Tmv* mv, int operando);
void JP(Tmv* mv, int operando);
void JN(Tmv* mv, int operando);
void JNZ(Tmv* mv, int operando);
void JNP(Tmv* mv, int operando);
void JNN(Tmv* mv, int operando);
void NOT(Tmv* mv, int operando);
//operaciones sin parametro
void STOP(Tmv* mv);

//funciones para dissasembler
void disassembler(Tmv* mv);
void impNombreOperando(Tmv* mv, int ip, int tipo);

extern const int (*pfuncionLectura[])();
extern const void (*pfuncionImpresion[])(unsigned int, int);
extern const void (*pfuncion0Param[])(Tmv *mv);
extern const void (*pfuncion1Param[])(Tmv *mv, int);
extern const void (*pfuncion2Param[])(Tmv *mv, int, int);

#endif