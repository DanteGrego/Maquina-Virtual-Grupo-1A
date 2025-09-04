#define TAM_MEMORIA 16384 //16 KiB en Bytes
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 2
#define TAM_IDENTIFICADOR 5

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

typedef struct{
    char memoria[TAM_MEMORIA];
    int registros[CANT_REGISTROS];
    int tablaSegmentos[CANT_SEGMENTOS];
}Tmv;

//prototipos
int combinarHighLow(int bytesHigh, int bytesLow); 
int obtenerHigh(int bytes);
int obtenerLow(int bytes);
void leerArch(Tmv* mv, char* nomArch);
int getValor(Tmv* mv, int bytes); //TODO
void cargarTablaSegmentos(Tmv* mv, int tamCodigo);