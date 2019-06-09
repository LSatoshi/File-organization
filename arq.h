#ifndef ARQ_H
#define ARQ_H

//registro de dados
struct _dados{
    char removido;          //char '*' ou '-' - 1 byte
    int tamanhoRegistro;     //int - 4 bytes
    long int encadeamentoLista; //int - 8 bytes
    int idServidor;         //int - 4 bytes
    double salarioServidor; //numero de dupla precisao - 8 bytes
    char telefoneServidor[14];  //char na forma (DD)NNNNN-NNNN - 14 bytes
    int tamNomeServidor;    //int - 4 bytes
    char tag4;
    char *nomeServidor;     //tamanho variavel
    int tamCargoServidor;   //int - 4 bytes
    char tag5;
    char *cargoServidor;    //tamanho variavel
};

//registro de cabeçalho
struct _cabecalho{
    char status;            //char - 1 byte
    long int topoLista;     //byte offset - int 8 bytes
    char tags[5];           //valor da tag - 5 * char 1 byte
    char campos[5][40];     //descricao da tag - 5 * char 40 bytes
};

typedef struct _dados dados;
typedef struct _cabecalho cabecalho;

void makeBin(char *name);
void readBin(char *name);
int searchBin(char *name, char *campo, char *valor, int menu);
cabecalho *makeHeader();
void writeHeader(cabecalho *c, FILE *file);
dados *makeRegister();
void clearRegister(dados *d);
void freeRegister(dados *d);
void printBinRegister(dados *d, FILE *file);
void fillAt(int n, FILE *file);
int EndsWith(char *name, char *tipo);
FILE *openFile(char *name, char *type);
void readBinRegister(FILE *fileIn, dados *d);
void readBinHeader(FILE *fileIn, cabecalho *c);
void printRegister(dados *d);
void insertList(long int posAtual, int tamAtual, FILE *fileIn);
void removeRegister(FILE *fileIn, dados *d);
void printSearchRegister(cabecalho *c, dados *d);
char tagCampo(char *c);
int cmpReg(dados *a, dados *b, char *campo);
dados *seqSearch(FILE *fileIn, dados *search, char *campo);
void namelessFunction(dados *d, char *campo, char *valor);
void updateRegister(char *name, char *campo, char *valor, char *campoNew, char *valorNew);
void writeRegister(dados *d);
long int findPlace(FILE *fileIn, dados *d);
void addRegister(char *name);
int cmpRegId(const void * a, const void * b);
void sortFile(char *nameIn, char *nameOut);
void mergeFiles(char *nameInA, char *nameInB, char *nameOut);
void intersecFiles(char *nameInA, char *nameInB, char *nameOut);
void setStatus(FILE *file, int n);
void updateField(dados *d, char *campo, char *valor) ;

#endif
