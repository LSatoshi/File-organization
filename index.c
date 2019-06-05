#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "arq.h"
#include "escrevernatela.h"
#include "index.h"


/*
*
*struct handler functions
*
*/

//aloca struct de cabecalho do index
headerI* makeHeaderI() {
    headerI* h = (headerI*) malloc(sizeof(headerI));
    return(h);
}

//insere dados na estrutura de cabecalho do index
void setHeaderI(headerI *h, int status, int numReg) {
    if(h != NULL) {
        if(status == 1) h->status = '1';
        else h->status = '0';
        h->numReg = numReg;
    }
    return;
}

//aloca struct de registro do index
regI* makeRegisterI() {
    regI* r = (regI*) malloc(sizeof(regI));
    return(r);
}

//limpra registro do index, colocando valores padroes
void clearRegisterI(regI *reg) {
    for(int i = 0; i < 120; i++) {
        reg->chaveBusca[i] = '@';
    }
    reg->byteOffset = -1;
    return;
}

//funcao compare para qsort
int regNameCmp(const void * a, const void * b) {
    regI *r1 = (regI *) a;
    regI *r2 = (regI *) b;
    char aux = 0;
    int i = 0, size = MIN(strlen(r1->chaveBusca),strlen(r2->chaveBusca));
    while(aux == 0 && i <= size) {
        aux = r1->chaveBusca[i] - r2->chaveBusca[i];
        i++;
    }
    return((int)aux);
}



/*
*
*file handler functions
*
*/

//escreve o cabecalho no binario de indice
void writeBinHeaderI(FILE *file, headerI *head) {
    long int place = ftell(file);
    fseek(file, 0, SEEK_SET);
    fwrite(&head->status, sizeof(char), 1, file);
    fwrite(&head->numReg, sizeof(int), 1, file);
    fillAt(PageSize-5, file);
    fseek(file, place, SEEK_SET);
    return;
}

//escreve um registro no binario de indice
void writeBinRegI(FILE *fileOut, regI *reg) {
    fwrite(&reg->chaveBusca, 120*sizeof(char), 1, fileOut);
    fwrite(&reg->byteOffset,sizeof(long int), 1, fileOut);
    return;
}

//le um registro de um binario de dados e guarda em uma struct de registro para indice
void readBinRegI(FILE *fileIn, regI *reg) {
    char status, tag;
    int size, nameSize;
    reg->byteOffset = ftell(fileIn);
    fread(&status, sizeof(char), 1, fileIn);
    fread(&size, sizeof(int), 1, fileIn);
    if(status != '*' && size > 34) {
        fseek(fileIn, 34, SEEK_CUR);
        fread(&nameSize, sizeof(int), 1, fileIn);
        fread(&tag, sizeof(char), 1, fileIn);
        if(tag == 'n') {
            for(int i = 0; i < nameSize - 1; i++) {
                fread(&reg->chaveBusca[i], sizeof(char), 1, fileIn);
            }
        }
    }
    fseek(fileIn, reg->byteOffset + size + 5, SEEK_SET);
}

//faz um arquivo de indice a partir de um arquivo de dados
void makeIndex(char *nameIn, char *indexName) {
    FILE *fileIn = openFile(nameIn, ".bin");
    FILE *fileOut = fopen(indexName, "wb+");
    if(fileIn == NULL || fileOut == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    headerI* h = makeHeaderI();
    regI reg[MAX];          //cria um vetor de registros para indice
    int numReg;
    fseek(fileIn,PageSize,SEEK_SET);
    for(numReg = 0; fgetc(fileIn) != EOF; numReg++) {//enquanto o arquivo de dados nao cheaga ao fim
        fseek(fileIn, -1, SEEK_CUR);
        clearRegisterI(&reg[numReg]);
        readBinRegI(fileIn, &reg[numReg]);          //le um registro, guarda  o nome e a posicao
        if(reg[numReg].chaveBusca[0] == '@') numReg--;//se for um removido, ignora
    }
    qsort(reg, numReg, sizeof(regI), regNameCmp);//ordena o array com qsort
    setHeaderI(h, 0, numReg);                    //define informacoes do cabecalho
    writeBinHeaderI(fileOut, h);                 //escreve a pagina de cabecalho no arquivo de indice
    fseek(fileOut,PageSize,SEEK_SET);
    for(int i = 0; i < numReg; i++) {
        writeBinRegI(fileOut, &reg[i]);         //escreve registros ordenados
    }
    free(h);
    setStatus(fileOut, 1);//muda status do indice para 1
    fclose(fileIn);
    fclose(fileOut);
    return;
}

// funcao que pega o arquivo de indice e escreve
// os registros desse arquivo em um array, retornando este ultimo
int returnArrayIndex(char* valorIndex, regI* arrayIndex) {
    FILE* indexName = fopen(valorIndex, "rb");
    if(indexName == NULL) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    int nPagina = 1;    // numero de paginas acessadas, contando a pagina do header

    char c = fgetc(indexName);  // checa o status do arquivo index
    printf("c = %c\n", c);
    
    if(c == '0') {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    setStatus(indexName, 0);
    fseek(indexName, 1, SEEK_SET);
    int numReg;
    fread(&numReg, sizeof(int), 1, indexName);  // aloca quantos registros tem no arquivo de index
    arrayIndex = (regI*) malloc(sizeof(regI) * numReg);   // aloca o vetor dos registros do index

    fseek(indexName, 32000, SEEK_SET);
    for(int i = 0; i < numReg; i++) {
        fread(&arrayIndex[i].chaveBusca, sizeof(char), 120, indexName); // le o nome, ou seja, a chave de busca
        fread(&arrayIndex[i].byteOffset, sizeof(int), 1, indexName);    // le o byteoffset daquele registro
    }

    nPagina += floor((numReg * 128) / 32000);   // calcula o numero de paginas
    fclose(indexName);
    return nPagina;
}

// funcao que busca no array do index
// o campo nomeServidor e o valor deste, depois
// define um array com os bytes offset dos valores encontrados
// e retorna o numero de paginas acessadas
int buscaNomeIndex(regI* arrayIndex, char* valorNome, int numReg, long int* byteOffset) {
    int nPagina = 1; 
    int countPagina;

    for(int i = 0; i < numReg; i++) {
        if(strcmp(arrayIndex[i].chaveBusca, valorNome) == 0) {  // se encontrar o nome...
            byteOffset[i] = arrayIndex[i].byteOffset;   // atribui o byteoffset encontrado
            if(countPagina >= 32000) {  //
                nPagina++;              //  se o valorNome estiver em outra pagina
                countPagina = 0;        //  de disco, adiciona ao numero de paginas acessadas
            }
            countPagina += 128;     // soma o tamanho dos registros do index

        } else {
            if(countPagina >= 32000) {  //
                nPagina++;              //  adiciona ao numero de paginas acessadas  
                countPagina = 0;        //
            }
            countPagina += 128;     // soma o tamanho dos registros do index
        }
    }

    return nPagina;
}

void printRegisterIndex(FILE* binarioEntrada, long int* byteOffset, int numReg) {
    dados *reg = makeRegister();
    cabecalho *cab = makeHeader();
    readBinHeader(binarioEntrada, cab);

    for(int i = 0; i < numReg; i++) {
        fseek(binarioEntrada, byteOffset[i], SEEK_SET);  // pula para o byteoffset do registro para impressao
        clearRegister(reg);
        readBinRegister(binarioEntrada, reg);
        printSearchRegister(cab, reg);
    }
    setStatus(binarioEntrada, 1);
    return;
}