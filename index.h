#ifndef INDEX_H
#define INDEX_H

#define PageSize 32000
#define MAX 10000
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#include <stdio.h>
// struct do cabecalho do arquivo de Index
struct _headerI {  // header index
    char status;
    int numReg;
};

// struct dos registros do arquivo de index
struct _regI {  // register index
    char chaveBusca[120];
    long int byteOffset;
};

typedef struct _headerI headerI;
typedef struct _regI regI;

void makeIndex(char* nameIn, char* indexName);
int returnArrayIndex(char* indexName, regI* arrayIndex, int numReg);
int buscaNomeIndex(regI* arrayIndex, char* valorNome, int numReg, long int* byteOffset);
void printRegisterIndex(FILE* binarioEntrada, long int* byteOffset, int numReg);
int returnNumReg(char* valorNome);
regI* alocaArrayRegI2d(regI* arrayIndex, int n);
long int* alocaArrayInt2d(long int* byteOffset, int n);
void removeRegisterI(char *fileName, char *indexName, char *name);
void addRegisterIndex(char *name, char* indexName);

#endif