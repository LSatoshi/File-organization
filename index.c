#include "index.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arq.h"
#include "escrevernatela.h"

/*
 *
 *struct handler functions
 *
 */

// aloca struct de cabecalho do index
headerI *makeHeaderI() {
    headerI *h = (headerI *)malloc(sizeof(headerI));
    return (h);
}

// insere dados na estrutura de cabecalho do index
void setHeaderI(headerI *h, int status, int numReg) {
    if (h != NULL) {
        if (status == 1)
            h->status = '1';
        else
            h->status = '0';
        h->numReg = numReg;
    }
    return;
}

// aloca struct de registro do index
regI *makeRegisterI() {
    regI *r = (regI *)malloc(sizeof(regI));
    return (r);
}

// limpra registro do index, colocando valores padroes
void clearRegisterI(regI *reg) {
    for (int i = 0; i < 120; i++) {
        reg->chaveBusca[i] = '@';
    }
    reg->byteOffset = -1;
    return;
}

// funcao compare para qsort
int regNameCmp(const void *a, const void *b) {
    regI *r1 = (regI *)a;
    regI *r2 = (regI *)b;
    char aux = 0;
    int i = 0, size = MIN(strlen(r1->chaveBusca), strlen(r2->chaveBusca));
    while (aux == 0 && i <= size) {
        aux = r1->chaveBusca[i] - r2->chaveBusca[i];
        i++;
    }
    return ((int)aux);
}

void removeFromIndex(regI *Arr, long int *offset) {
    int i, j = 0;
    for(i = 0; offset[j] != -1; i++) {
        if(Arr[i].byteOffset == offset[j]) {
            Arr[i].chaveBusca[0] = '@';
            j++;
        }
    }
    return;
}

// funcao que adiciona um registro ao indice na ram
void addToIndex(regI *indexArr, int numReg, long int pos, dados *d) {
    indexArr = (regI *) realloc(indexArr, sizeof(regI) * numReg);    // adiciona um novo registro
    indexArr[numReg-1].byteOffset = pos;  // insere o byteoffset do novo registro
    strcpy(indexArr[numReg-1].chaveBusca, d->nomeServidor);   // insere o nome do novo registro
    char lixo = '@';
    for(int i = strlen(d->nomeServidor) + 1; i < 120; i++) {
        indexArr[numReg-1].chaveBusca[i] = lixo;  //  completa com lixo
    }
    qsort(indexArr, numReg, sizeof(regI), regNameCmp);
    return;
}

/*
 *
 *file handler functions
 *
 */

// escreve o cabecalho no binario de indice
void writeBinHeaderI(FILE *file, headerI *head) {
    long int place = ftell(file);
    fseek(file, 0, SEEK_SET);
    fwrite(&head->status, sizeof(char), 1, file);
    fwrite(&head->numReg, sizeof(int), 1, file);
    fillAt(PageSize - 5, file);
    fseek(file, place, SEEK_SET);
    return;
}

// escreve um registro no binario de indice
void writeBinRegI(FILE *fileOut, regI *reg) {
    if(reg->chaveBusca[0] != '@') {
        fwrite(&reg->chaveBusca, 120 * sizeof(char), 1, fileOut);
        fwrite(&reg->byteOffset, sizeof(long int), 1, fileOut);
    }
    return;
}

// le um registro de um binario de dados e guarda em uma struct de registro para
// indice
void readBinRegI(FILE *fileIn, regI *reg) {
    char status, tag;
    int size, nameSize;
    reg->byteOffset = ftell(fileIn);
    fread(&status, sizeof(char), 1, fileIn);
    fread(&size, sizeof(int), 1, fileIn);
    if (status != '*' && size > 34) {
        fseek(fileIn, 34, SEEK_CUR);
        fread(&nameSize, sizeof(int), 1, fileIn);
        fread(&tag, sizeof(char), 1, fileIn);
        if (tag == 'n') {
            for (int i = 0; i < nameSize - 1; i++) {
                fread(&reg->chaveBusca[i], sizeof(char), 1, fileIn);
            }
        }
    }
    fseek(fileIn, reg->byteOffset + size + 5, SEEK_SET);
}

// faz um arquivo de indice a partir de um arquivo de dados
void makeIndex(char *nameIn, char *indexName) {
    FILE *fileIn = openFile(nameIn, ".bin");
    FILE *fileOut = fopen(indexName, "wb+");
    if (fileIn == NULL || fileOut == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    headerI *h = makeHeaderI();
    regI reg[MAX];  // cria um vetor de registros para indice
    int numReg;
    fseek(fileIn, PageSize, SEEK_SET);
    for (numReg = 0; fgetc(fileIn) != EOF; numReg++) {  // enquanto o arquivo de dados nao cheaga ao fim
        fseek(fileIn, -1, SEEK_CUR);
        clearRegisterI(&reg[numReg]);
        readBinRegI(fileIn, &reg[numReg]);  // le um registro, guarda  o nome e a posicao
        if (reg[numReg].chaveBusca[0] == '@')
            numReg--;  // se for um removido, ignora
    }
    qsort(reg, numReg, sizeof(regI), regNameCmp);  // ordena o array com qsort
    setHeaderI(h, 0, numReg);  // define informacoes do cabecalho
    writeBinHeaderI(fileOut, h);  // escreve a pagina de cabecalho no arquivo de indice
    fseek(fileOut, PageSize, SEEK_SET);
    for (int i = 0; i < numReg; i++) {
        writeBinRegI(fileOut, &reg[i]);  // escreve registros ordenados
    }
    free(h);
    setStatus(fileOut, 1);  // muda status do indice para 1
    fclose(fileIn);
    fclose(fileOut);
    return;
}

// funcao que retorna o numero de registros
// do arquivo de indice
int returnNumReg(char *valorIndex) {
    FILE *indexName = fopen(valorIndex, "rb");
    fseek(indexName, 1, SEEK_SET);
    int numReg;
    fread(&numReg, sizeof(int), 1, indexName);
    fclose(indexName);
    return numReg;
}

// funcao que aloca um array regI bidimensional
regI *alocaArrayRegI2d(regI *arrayIndex, int n) {
    arrayIndex = (regI *)malloc(sizeof(regI) *n);  // aloca o vetor dos registros do index
    /*for (int i = 0; i < n; i++) {
        arrayIndex[i] = (regI )malloc(sizeof(regI) * n);  // aloca o vetor dos registros do index
    }*/
    return arrayIndex;
}

// funcao que pega o arquivo de indice e escreve
// os registros desse arquivo em um array, retornando este ultimo
int returnArrayIndex(char *valorIndex, regI *arrayIndex, int numReg) {
    FILE *indexName = fopen(valorIndex, "rb");
    if (indexName == NULL) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }
    int nPagina = 1;  // numero de paginas acessadas, contando a pagina do header
    if (fgetc(indexName) == '0') { // checa o status do arquivo index
        printf("Falha no processamento do arquivo.");
        return 0;
    }
    fseek(indexName, PageSize, SEEK_SET);
    for (int i = 0; i < numReg; i++) {
        fread(&arrayIndex[i].chaveBusca, sizeof(char), 120, indexName);  // le o nome, ou seja, a chave de busca
        fread(&arrayIndex[i].byteOffset, sizeof(long int), 1, indexName);  // le o byteoffset daquele registro
    }
    nPagina += (numReg*128) / PageSize;  // calcula o numero de paginas inteiras acessadas
    if((numReg*128) % PageSize > 0) nPagina++; //se houver resto, ja entrou em outra, entao adiciona mais um
    fclose(indexName);
    return nPagina;
}

// funcao que aloca um array int bidimensional
long int *alocaArrayInt2d(long int *byteOffset, int n) {
    byteOffset = (long int *)malloc(sizeof(long int ) * n);  // aloca o vetor dos registros do index
    /*for (int i = 0; i < n; i++) {
        byteOffset[i] = (long int *)malloc(sizeof(long int) * n);  // aloca o vetor dos registros do index
    }*/
    return byteOffset;
}

// funcao que busca no array do index
// o campo nomeServidor e o valor deste, depois
// define um array com os bytes offset dos valores encontrados
// e retorna o numero de paginas acessadas
int buscaNomeIndex(regI *arrayIndex, char *valorNome, int numReg, long int *byteOffset) {
    int nPagina = 0;
    long int countPagina = 0;
    long int curPag, prevPag = -1;
    int count = 0;  // para atribuir na ordem correta
    for (int i = 0; i < numReg; i++) {
        if (strcmp(arrayIndex[i].chaveBusca, valorNome) == 0) {  // se encontrar o nome...
            /*if((byteOffset[count] - countPagina) > 32000 || (byteOffset[count] - countPagina) < -32000) {
                nPagina++;  // conta mais uma pagina de acesso de disco
            }
            byteOffset[count] = arrayIndex[i].byteOffset;  // atribui o byteoffset encontrado
            countPagina = arrayIndex[i].byteOffset; // guarda esse byteoffset para contar as paginas
            count++;  // aumenta o contador, para atribuir no proximo byteoffset*/

            curPag = arrayIndex[i].byteOffset % PageSize; //guarda a pagina onde esta o registro atual
            if(curPag != prevPag) nPagina++;              //se a pagina do registro atual for diferente, aumenta contador
            byteOffset[count] = arrayIndex[i].byteOffset; //guarda offset num array
            prevPag = curPag;
            count++;
        }
    }
    byteOffset[count] = -1;
    return nPagina;
}

void printRegisterIndex(FILE *binarioEntrada, long int *byteOffset, int numReg) {
    dados *reg = makeRegister();
    cabecalho *cab = makeHeader();
    readBinHeader(binarioEntrada, cab);
    int count = 0;
    int size = 0;
    // obtem quantos bytes foram lidos na funcaode busca de nome
    // no indice e atribui ao size
    while (1) {
        count++;
        if (byteOffset[count] == -1) {
            size = count;
            break;
        }
    }
    count = 0;
    for (int i = 0; i < size; i++) {
        fseek(binarioEntrada, byteOffset[i], SEEK_SET);  // pula para o byteoffset do registro para impressao
        clearRegister(reg);
        readBinRegister(binarioEntrada, reg);
        printSearchRegister(cab, reg);
        count++;
    }
    setStatus(binarioEntrada, 1);
    return;
}

//função de remover registro utilizando indice
void removeRegisterI(char *fileName, char *indexName, char *name) {
    FILE *fileIn = fopen(fileName, "rb+");          //abre arquivo de dados e indice para leitura e escrita
    FILE *index = fopen(indexName, "rb+");
    if(fileIn == NULL || index == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    if(fgetc(fileIn) == '0' || fgetc(index) == '0') {//verifica consistencia
        printf("Falha no processamento do arquivo.");
        return;
    }
    int numReg, j;
    fread(&numReg, sizeof(int), 1, index);          //le o numero de registros do indice
    regI *indexArr = alocaArrayRegI2d(indexArr, numReg);//aloca um array de registro desse tamanho
    returnArrayIndex(indexName, indexArr, numReg);  //pega os registros do indice e coloca no array
    dados *reg = makeRegister();
    long int *offset;
    alocaArrayInt2d(offset, 50);//cria um array de long int
    buscaNomeIndex(indexArr, name, numReg, offset);             //funcao retorna o array de long com os offset dos registros a serem removidos
    setStatus(fileIn, 0);               //muda status do arquivo para zero enqunato altera ele
    for(j = 0; offset[j] !=  -1; j++) { //enquanto houver offsets validos no array
        fseek(fileIn, offset[j], SEEK_SET);//vai até eles e remove o registro
        clearRegister(reg);
        readBinRegister(fileIn, reg);   
        removeRegister(fileIn, reg);       //remoção feita como na funcionalidade [4]
    }
    removeFromIndex(indexArr, offset);     //remove o dados desses registros do array que vai pro indice
    headerI *head = makeHeaderI();
    setHeaderI(head, 0, numReg-j);
    setStatus(index, 0);                //muda status do arquivo para zero enquanto altera ele
    index = fopen(indexName, "wb");     //reabre o arquivo de indice para escrita
    writeBinHeaderI(index, head);       //escreve o novo indice sem os removidos
    fseek(index, PageSize, SEEK_SET);
    for(int i = 0; i < numReg; i++) {
        writeBinRegI(index, &indexArr[i]);//imprime todos registros no indice
    }
    setStatus(fileIn, 1);
    setStatus(index, 1);
    fclose(fileIn);
    fclose(index);
    return;
}

//funcao estendida da funcionalidade 5,
//tambem conhecida como funcionalidade 13
void addRegisterIndex(char *name, char* indexName) {
    FILE *fileIn = fopen(name, "rb+");          //abre arquivo de dados e indice para leitura e escrita
    FILE *index = fopen(indexName, "rb+");
    if(fileIn == NULL || index == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    if(fgetc(fileIn) == '0' || fgetc(index) == '0') {//verifica consistencia
        printf("Falha no processamento do arquivo.");
        return;
    }
    int numReg = returnNumReg(indexName);   // obtem o numero de registradores
    setStatus(fileIn, 0);
    long int pos;                          //posicao para escrever o registro
    int tam;
    dados *d = makeRegister();             //cria um registro
    clearRegister(d);
    writeRegister(d);                      //preenche ele com os dados fornecidos pelo usuario
    pos = findPlace(fileIn, d);            //encontra uma posicao livre
    fseek(fileIn, pos, SEEK_SET);          //vai para a posicao 
    printBinRegister(d, fileIn);           //escreve o registro no arquivo
    regI *indexArr = (regI *) malloc(sizeof(regI) * numReg);
    returnArrayIndex(indexName, indexArr, numReg);  //pega os registros do indice e coloca no array em ram
    if(!strcmp(d->nomeServidor, "NULO")) {
        printf("deu nulo\n");
        numReg++;   // adiciona um registro
        addToIndex(indexArr, numReg, pos, d);  // adiciona o novo registro ao indice na ram
        headerI *head = makeHeaderI();
        setHeaderI(head, 0, numReg);
        setStatus(index, 0);                //muda status do arquivo para zero enquanto altera ele
        writeBinHeaderI(index, head);       //escreve o novo indice com os adicionados
        fseek(index, PageSize, SEEK_SET);
        for(int i = 0; i < numReg; i++) {
            writeBinRegI(index, &indexArr[i]);
        }
    }
    setStatus(fileIn, 1);
    setStatus(index, 1);
    freeRegister(d);
    fclose(fileIn);
    fclose(index);
    return;
}