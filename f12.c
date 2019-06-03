#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arq.h"
#include "escrevernatela.h"
#include "index.h"

void removeFromIndex(regI *Arr, long int *offset) {
    int i, j = 0;
    for(i = 0; offset[j] != 0; i++) {
        if(Arr[i].byteOffset == offset[j]) {
            Arr[i].chaveBusca[0] = '@';
            j++;
        }
    }
    return;
}

void removeRegisterI(char *fileName, char *indexName, char *name, int n) {
    FILE *fileIn = fopen(fileName, "wb+");
    FILE *index = fopen(indexName, "rb+");
    if(fileIn == NULL || index == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    if(fgetc(fileIn) == '0' || fgetc(index) == '0') {
        printf("Falha no processamento do arquivo.");
        return;
    }
    setStatus(fileIn, 0);
    setStatus(index, 0);
    int numReg;
    fread(&numReg, sizeof(int), 1, index);
    regI *indexArr = returnArrayIndex(index);
    dados *reg = makeRegister();
    long int offset[100] = 0;
    for(int i = 0; i < n; i++) {
        buscaNomeIndex(indexArr, name, numReg, offset);
        for(int j = 0; offset[j] != 0; j++) {
            fseek(fileIn, offset[j], SEEK_CUR);
            clearRegister(reg);
            readBinRegister(fileIn, reg);
            removeRegister(fileIn, reg);
        }
    }
    removeFromIndex(indexArr, offset);
    setStatus(fileIn, 1);
    setStatus(index, 1);
    fclose(fileIn);
    fclose(index);
    return;
}

/*notes:
    alterar writebinregI if [i] = @
    alterar main
//regi*, char, int, long int*