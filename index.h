#ifndef INDEX_H
#define INDEX_H

#define PageSize 32000
#define MAX 10000
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

//struct do cabecalho do arquivo de Index
struct _headerI {       //header index
    char status;
    int numReg;
};

//struct dos registros do arquivo de index
struct _regI {          //register index
    char chaveBusca[120];
    long int byteOffset;
};

typedef struct _headerI headerI;
typedef struct _regI regI;

void makeIndex(char *nameIn, char *indexName);

#endif