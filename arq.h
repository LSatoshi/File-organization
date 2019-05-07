#ifndef ARQ_H
#define ARQ_H

typedef struct _dados dados;
typedef struct _cabecalho cabecalho;

void makeBin(char *name);
void readBin(char *name);
void searchBin(char *name, char *campo, char *valor, int menu);


#endif
