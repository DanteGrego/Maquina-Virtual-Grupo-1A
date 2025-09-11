#define TAM_MEMORIA 16384 //16 KiB en Bytes
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 2
#define TAM_IDENTIFICADOR 5
#define CANT_BYTES_A_LEER 4


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

const char* nombreRegistros[] = {
    [0]  = "LAR",
    [1]  = "MAR",
    [2]  = "MBR",
    [3]  = "IP",
    [4]  = "OPC",
    [5]  = "OP1",
    [6]  = "OP2",
    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",
    [16] = "AC",
    [17] = "CC",
    [26] = "CS",
    [27] = "DS"
};

char* mnemonicos[CANT_REGISTROS];

typedef struct{
    char memoria[TAM_MEMORIA];
    int registros[CANT_REGISTROS];
    int tablaSegmentos[CANT_SEGMENTOS];
}Tmv;

//prototipos
int combinarHighLow(int bytesHigh, int bytesLow); 
int obtenerHigh(int bytes);
int obtenerLow(int bytes);
int obtenerDirFisica(Tmv* mv, int dirLogica);
void leerArch(Tmv* mv, char* nomArch);
int getValor(Tmv* mv, int bytes); //TODO
char obtengoTipoOperando(int bytes);
void cargarTablaSegmentos(Tmv* mv, int tamCodigo);
void leerMemoria(Tmv* mv, int valor);
int obtenerDirLogica(Tmv* mv, int valor);
int leerValOperando(Tmv* mv, int top, int posOp);
void leerInstruccion(Tmv* mv);
void actualizarCC(Tmv* mv, int valor);
void JMP(Tmv* mv, int direccion);
void JZ(Tmv* mv, int direccion);
void JNZ(Tmv* mv, int direccion);
void JN(Tmv* mv, int direccion);
void JNN(Tmv* mv, int direccion);
void JP(Tmv* mv, int direccion);
void JNP(Tmv* mv, int direccion);
