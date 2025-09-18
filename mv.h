//tamanos y cantidades de cosas
#define TAM_MEMORIA 16384 //16 KiB en Bytes
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 2
#define CANT_FORMATOS 4
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
    int memoriaAccedida[TAM_MEMORIA];//para el modo dev, guarda las posiciones de la memoria que se escribio para mostrarla
    int nMemoriaAccedida;//cantidad de celdas accedidas
    int registros[CANT_REGISTROS];
    int tablaSegmentos[CANT_SEGMENTOS];
}Tmv;

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

const char* formatosLectura[CANT_FORMATOS] = {"%d", "%c", "%o", "%x"};
const char* formatosEscritura[CANT_FORMATOS] = {" %d", " %c", " 0o%o", " 0x%x"};


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
int  leerValOperando(Tmv* mv, int top, int posOp);
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
void imprimirBinario(unsigned int valor, int tamCelda);
int leerBinario();
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

//funciones para modo dev
void imprimirTabla(Tmv* mv);
void imprimirMemoria(Tmv* mv);
void imprimirRegistros(Tmv* mv);
int estaEnMemoriaAccedida(Tmv* mv, int pos);

//funciones para dissasembler
void disassembler(const Tmv* mv);
void impNombreOperando(const Tmv* mv, int ip, int tipo);

//punteros a funcion
const void (*pfuncion0Param[CANT_FUNCIONES_0_PARAM])(Tmv *mv) = {
    [0x00] = &STOP
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
    [0x08] = &NOT
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
