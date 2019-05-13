#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arq.h"
#include "extra.c"
#define PageSize 32000

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

//funcao que cria o cabecalho iniciando os valores base
cabecalho *makeHeader() { 
    cabecalho *header = (cabecalho*) malloc(sizeof(cabecalho));
    header->status = '1';
    header->topoLista = -1;
    header->tags[0] = 'i';
    header->tags[1] = 's';
    header->tags[2] = 't';
    header->tags[3] = 'n';
    header->tags[4] = 'c';
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 40; j++) {
            header->campos[i][j] = '@';
        }
    }
    return(header);
}

//escreve o cabecalho no arquivo
void writeHeader(cabecalho *c, FILE *file) {
    fwrite(&c->status, sizeof(c->status), 1, file);
    fwrite(&c->topoLista, sizeof(c->topoLista), 1, file);
    for(int i = 0; i < 5; i++) {
        fwrite(&c->tags[i], sizeof(c->tags[i]), 1, file);
        for(int j = 0; j < 40; j++) {
            fwrite(&c->campos[i][j], sizeof(c->campos[i][j]), 1, file);
        }
    }
    return;
}

//alloca espaco para um registro de dados
dados *makeRegister() {
    dados *d = (dados*) malloc(sizeof(dados));
    return(d);
}

//limpa o resgistro de dados
void clearRegister(dados *d) {
    d->removido = '-';
    d->tamanhoRegistro = 34; //tamanho dos campos fixos
    d->encadeamentoLista = -1;
    for(int i = 0; i < 14; i++) {
        d->telefoneServidor[i] = '@';
    }
    d->tamNomeServidor = 0;
    d->tag4 = 'n';
    d->nomeServidor = (char*) malloc(sizeof(char) * 100);
    d->tamCargoServidor = 0;
    d->tag5 = 'c';
    d->cargoServidor = (char*) malloc(sizeof(char) * 100);
    return;
}

void freeRegister(dados *d) {
    free(d->nomeServidor);
    free(d->cargoServidor);
    free(d);
    d = NULL;
    return;
}

//imprime registro de dados em binario no arquivo
void printBinRegister(dados *d, FILE *file) {
    fwrite(&d->removido, sizeof(char), 1, file);
    fwrite(&d->tamanhoRegistro, sizeof(int), 1, file);
    fwrite(&d->encadeamentoLista, sizeof(long int), 1, file);
    fwrite(&d->idServidor, sizeof(int), 1, file);
    fwrite(&d->salarioServidor, sizeof(d->salarioServidor), 1, file);
    for(int i = 0; i < 14; i++){
        fwrite(&d->telefoneServidor[i], 1, 1, file);
    }
    if(d->tamNomeServidor > 0) {
        fwrite(&d->tamNomeServidor, sizeof(d->tamNomeServidor), 1, file);
        fwrite(&d->tag4, sizeof(d->tag4), 1, file);
        for(int i = 0; i < d->tamNomeServidor - 1; i++){
            fwrite(&d->nomeServidor[i], 1, 1, file);
        }
    }
    if(d->tamCargoServidor > 0) {
        fwrite(&d->tamCargoServidor, sizeof(d->tamCargoServidor), 1, file);
        fwrite(&d->tag5, sizeof(d->tag5), 1, file);
        for(int i = 0; i < d->tamCargoServidor - 1; i++){
            fwrite(&d->cargoServidor[i], 1, 1, file);
        }
    }
    return;
}

//preenche com '@'
void fillAt(int n, FILE *file){
    char c = '@';
    for(int i = 0; i < n; i++){
        fwrite(&c, sizeof(char), 1, file);
    }
}

//funcao que verifica o formato do arquivo pelo nome
int EndsWith(char *name, char *tipo) {
    int tamName = strlen(name);
    int tamTipo = strlen(tipo);
    return strncmp(name + tamName - tamTipo, tipo, tamTipo);
}

//abre arquivo, verifica consistencia e formato
FILE *openFile(char *name, char *type) {
    FILE *fileIn = fopen(name, "rb+"); //abre o arquivo para leitura e escrita
    char aux = '0';
    if(fileIn == NULL || EndsWith(name, type) != 0) {//verifica se abriu e se tem o formato certo
        return NULL;
    }
    if(strcmp(type, ".bin") == 0) {             //se for binario
        cabecalho *c = makeHeader();       //aloca um cabecalho
        readBinHeader(fileIn, c);          //preenche com dados do arquivo
        if(c == NULL||c->status == '0') {  //verifica a consistencia
            return NULL;
        }
        fseek(fileIn, 0, SEEK_SET);
        fwrite(&aux,1,1,fileIn);           //marca como inconsistente até terminar a funcao
    }
    return (fileIn);
}


//funcao principal 1, cria um binario a partir de um csv valido
void makeBin(char *name) {
    int aux = 0;
    long int Atual = 0, Espaco;
    FILE *fileIn = fopen(name, "r+");
    FILE *fileOut = fopen("arquivoTrab1.bin", "wb");
    if(fileIn == NULL || EndsWith(name,".csv") != 0){
        printf("Falha no carregamento do arquivo.");
        return;
    }
    cabecalho *header = makeHeader();
    for(int i = 0; i < 5; i++) {
        fscanf(fileIn,"%40[^,\n\r]%*c", header->campos[i]);//le ate 40 char, colocando em campos[i] e ignorando , \n e \r e depois ignorando esse simbolo com %*c
    }
    writeHeader(header, fileOut);
    fillAt((PageSize) - 214, fileOut);
    dados *anterior = makeRegister();
    dados *data;
    while(fgetc(fileIn) != EOF) {
        fseek(fileIn, -1, SEEK_CUR);
        data = makeRegister();
        clearRegister(data);
        fscanf(fileIn,"%d%*c", &data->idServidor);
        fscanf(fileIn,"%lf%*c", &data->salarioServidor);
        if(fgetc(fileIn) != ',') {
            fseek(fileIn, -1, SEEK_CUR);
            fscanf(fileIn,"%[^,]%*c", data->telefoneServidor);
        } else data->telefoneServidor[0] = '\0';
        if(fgetc(fileIn) != ',') {
            fseek(fileIn, -1, SEEK_CUR);
            fscanf(fileIn,"%[^,]%*c", data->nomeServidor);
            data->tamNomeServidor = strlen(data->nomeServidor) + 2;//tag + \0
            data->tamanhoRegistro += data->tamNomeServidor + 4;//int tamanho + tag + size of nome
        } else data->tamNomeServidor = 0;
        if(fgetc(fileIn) != '\n') {
            fseek(fileIn, -1, SEEK_CUR);
            fscanf(fileIn,"%[^\n\r]%*c", data->cargoServidor);
            data->tamCargoServidor = strlen(data->cargoServidor) + 2;//tag + \0
            data->tamanhoRegistro += data->tamCargoServidor + 4;
        } else data->tamCargoServidor = 0;
        if(Atual + data->tamanhoRegistro + 5 > PageSize) {
            aux -= (anterior->tamanhoRegistro + 5);
            fseek(fileOut, aux, SEEK_CUR);
            Espaco = PageSize - Atual;
            anterior->tamanhoRegistro += Espaco;
            printBinRegister(anterior, fileOut);
            fillAt(Espaco, fileOut);
            aux = 0;
            Atual = 0;
        }
        Atual += data->tamanhoRegistro + 5;
        printBinRegister(data, fileOut);
        freeRegister(anterior);
        anterior = data;
    }
    printf("arquivoTrab1.bin");
    return;
}

//**************************************************************************************//

//le um registro binario e guarda em uma struct de registros
void readBinRegister(FILE *fileIn, dados *d) {
    char prox;
    int aux = 34;
    fread(&d->removido, sizeof(char), 1, fileIn);
    fread(&d->tamanhoRegistro, sizeof(int), 1, fileIn);
    if(d->removido == '*') return;
    long int inicio = ftell(fileIn);
    fread(&d->encadeamentoLista, sizeof(long int), 1, fileIn);
    fread(&d->idServidor, sizeof(int), 1, fileIn);
    fread(&d->salarioServidor, sizeof(d->salarioServidor), 1, fileIn);
    for(int i = 0; i < sizeof(d->telefoneServidor); i++){
        fread(&d->telefoneServidor[i], sizeof(d->telefoneServidor[i]), 1, fileIn);
    }
    fread(&prox, sizeof(char), 1, fileIn);
    if(prox != '@' && aux < d->tamanhoRegistro) {
        fseek(fileIn, -1, SEEK_CUR);
        fread(&d->tamNomeServidor, sizeof(d->tamNomeServidor), 1, fileIn);
        fread(&d->tag4, sizeof(d->tag4), 1, fileIn);
        if(d->tag4 != 'n') {
            fseek(fileIn, -5, SEEK_CUR);
            d->tamNomeServidor = 0;
        }
        else {
            aux += 5;
            for(int i = 0; i <= d->tamNomeServidor - 2; i++) {
                fread(&d->nomeServidor[i], sizeof(char), 1, fileIn);
                aux++;
            }
        }
        fread(&prox, sizeof(char), 1, fileIn);
        if(prox != '@' && aux < d->tamanhoRegistro) {
            fseek(fileIn, -1, SEEK_CUR);
            fread(&d->tamCargoServidor, sizeof(d->tamCargoServidor), 1, fileIn);
            fread(&d->tag5, sizeof(d->tag5), 1, fileIn);
            for(int i = 0; i <= d->tamCargoServidor - 2; i++) {
                fread(&d->cargoServidor[i], sizeof(char), 1, fileIn);
            }
        }
    }
    fseek(fileIn, inicio + d->tamanhoRegistro, SEEK_SET);
    return;
}

//le um cabecalho em binario e guarda em uma struct de cabecalho
void readBinHeader(FILE *fileIn, cabecalho *c) {
    fread(&c->status, sizeof(char), 1, fileIn);
    fread(&c->topoLista, sizeof(long int), 1, fileIn);
    for(int i = 0; i < 5; i++){
        fread(&c->tags[i], sizeof(char), 1, fileIn);
        for(int j = 0; j < 40; j++) {
            fread(&c->campos[i][j], sizeof(c->campos[i][j]), 1, fileIn);
        }
    }
    return;
}

//imprime um registro na tela
void printRegister(dados *d) {
    printf("%d",d->idServidor);
    if(d->salarioServidor == -1) {
        printf("         ");
    }
    else {
        printf(" %.2lf",d->salarioServidor);
    }
    if(d->telefoneServidor[0] == '\0') {
        printf("               ");
    }
    else {
        printf(" %.14s",d->telefoneServidor);
    }
    if(d->tamNomeServidor > 0) {
        printf(" %d",(d->tamNomeServidor - 2));
        printf(" %s",d->nomeServidor);
    }
    if(d->tamCargoServidor > 0) {
        printf(" %d", (d->tamCargoServidor - 2));
        printf(" %s",d->cargoServidor);
    }
    printf("\n");
    return;
}

//funcao principal da funcinalidade 2, le um arquivo binario valido e imprime registros na tela
void readBin(char *name) {
    FILE *fileIn = fopen(name, "rb");
    char aux = '*';
    if(fileIn == NULL || EndsWith(name,".bin") != 0) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    cabecalho *header = makeHeader();
    readBinHeader(fileIn, header);
    if(header->status == '0') {
        printf("Falha no processamento do arquivo.");
        return;
    }
    dados *d = makeRegister();
    fseek(fileIn, PageSize, SEEK_SET);
    while(fgetc(fileIn) != EOF) {
        fseek(fileIn, -1, SEEK_CUR);
        clearRegister(d);
        readBinRegister(fileIn, d);
        if(d->removido == '*') {
            fseek(fileIn, d->tamanhoRegistro, SEEK_CUR);
        }
        else {
            aux = '-';
            printRegister(d);
        }
    }
    if(aux == '*') {
        printf("Registro inexistente.");
        return;
    }
    printf("Número de páginas de disco acessadas: %ld", (ftell(fileIn)/PageSize) + 1);
    return;
}

//**************************************************************************************//


void insertList(long int posAtual, int tamAtual, FILE *fileIn) {
    fseek(fileIn, 1, SEEK_SET);                          //vai para o inicio da lista
    long int proxLista = -1, antLista = 1;
    int tamProx = 0;
    char aux;
    fread(&proxLista, sizeof(long int), 1, fileIn);     //le a proxima posicao
    if(proxLista == -1) {                               //se for zero
        fseek(fileIn, 1, SEEK_SET);   //insere a posicao do registro no topo da lista
        fwrite(&posAtual, sizeof(long int), 1, fileIn);
        return;
    }
    else {
        do {    
            fseek(fileIn, proxLista, SEEK_SET);              //vai para  a posicao do proximo na lista
            fread(&aux , sizeof(char), 1, fileIn);
            if(aux == '*') {                                 //verifica se esta removido 
                fread(&tamProx, sizeof(long int), 1, fileIn);//pega o tamanho do registro
            }
            else {
                printf("erro: lista aponta para registro nao removido\n");//erro caso não esteja removido
                return;
            }
            if(tamProx >= tamAtual) break;                  //se o proximo for maior, para
            antLista = proxLista + 5;                       //caso contrario guarda a posicao onde esta o encadeamento da lista
            fread(&proxLista, sizeof(long int), 1, fileIn); //e vai para o proximo
        }while(proxLista != -1);
        fseek(fileIn, antLista, SEEK_SET);              //insere na lista
        fwrite(&posAtual, sizeof(long int), 1, fileIn);
        fseek(fileIn, posAtual + 5, SEEK_SET);
        fwrite(&proxLista, sizeof(long int), 1, fileIn);
    }
    return;
}

void removeRegister(FILE *fileIn, dados *d) {//funcionando
    char c = '*';
    int aux = 0;
    aux -= (d->tamanhoRegistro + 5);
    fseek(fileIn, aux, SEEK_CUR);//volta para inicio do registro, no arquivo
    long int atual = ftell(fileIn);                    //guarda a posicao
    fwrite(&c, sizeof(char), 1, fileIn);               //marcar como removido
    insertList(atual, d->tamanhoRegistro, fileIn);     //chama funcao de inserir na lista
    fseek(fileIn, (d->tamanhoRegistro + 4), SEEK_CUR); //vai para o fim do arquivo
    return;
}

//**************************************************************************************//

//imprime registro encontrado
void printSearchRegister(cabecalho *c, dados *d) {
    printf("%s: ", c->campos[0]);
    if(d->idServidor != -1) printf("%d\n", d->idServidor);
    else printf("valor nao declarado\n");
    printf("%s: ", c->campos[1]);
    if(d->salarioServidor != -1) printf("%.2lf\n", d->salarioServidor);
    else printf("valor nao declarado\n");
    printf("%s: ", c->campos[2]);
    if(d->telefoneServidor[0] != '@') printf("%s\n", d->telefoneServidor);
    else printf("valor nao declarado\n");
    printf("%s: ", c->campos[3]);
    if(d->tamNomeServidor > 1) printf("%s\n", d->nomeServidor);
    else printf("valor nao declarado\n");
    printf("%s: ", c->campos[4]);
    if(d->tamCargoServidor > 1) printf("%s\n", d->cargoServidor);
    else printf("valor nao declarado\n");
    printf("\n");
    return;
}

//retorna campo de acordo com a tag
char tagCampo(char *c) {
    char tag;
    if(strcmp(c,"idServidor") == 0) tag ='i';
    if(strcmp(c,"salarioServidor") == 0) tag ='s';
    if(strcmp(c,"telefoneServidor") == 0) tag ='t';
    if(strcmp(c,"nomeServidor") == 0) tag ='n';
    if(strcmp(c,"cargoServidor") == 0) tag ='c';
    else tag = '0';
    return(tag);
}

//funcao principal da funcionalidade 3, busca um registro em um arquivo binario valido
void searchBin(char *name, char *campo, char *valor, int menu) {

    FILE *fileIn = fopen(name, "rb+");
    int id, numReg = 0, numCampo = -1;
    double salario;
    char tag = tagCampo(campo), aux = '0';
    if(fileIn == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    cabecalho *c = makeHeader();
    readBinHeader(fileIn, c);
    if(c == NULL||c->status == '0') {
        printf("Falha no processamento do arquivo.");
        return;
    }
    for(int i = 0; i < 5; i++) {
        if(tag == c->tags[i]) {
            numCampo = i;
            if(i == 0) id = atoi(valor);
            if(i == 1) sscanf(valor, "%lf", &salario);
        }
    }
    if(numCampo == -1) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    fseek(fileIn, 0, SEEK_SET);
    fwrite(&aux,1,1,fileIn);
    aux = '1';
    dados *d = makeRegister();
    fseek(fileIn, PageSize, SEEK_SET);
    while(fgetc(fileIn) != EOF) {
        fseek(fileIn, -1, SEEK_CUR);
        clearRegister(d);
        readBinRegister(fileIn, d);
        if(d->removido == '*') {
            fseek(fileIn, d->tamanhoRegistro, SEEK_CUR);
        }
        
        else {
            switch (numCampo) {
            case 0:
                if(d->idServidor == id) {
                    if(menu == 3) {
                        printSearchRegister(c,d);
                        printf("Número de páginas de disco acessadas: %ld", (ftell(fileIn)/PageSize) + 1);
                        numReg++;
                    }
                    else if(menu == 4) {
                        removeRegister(fileIn, d);
                    }
                    return;
                }
                break;
            
            case 1:
                if(d->salarioServidor == salario) {
                    if(menu == 3) {
                        printSearchRegister(c,d);
                        numReg++;
                    }
                    else if(menu == 4) {
                        removeRegister(fileIn, d);
                    }
                }
                break;
            
            case 2:
                if(strcmp(d->telefoneServidor, valor) == 0) {
                    if(menu == 3) {
                        printSearchRegister(c,d);
                        numReg++;
                    }
                    else if(menu == 4) {
                        removeRegister(fileIn, d);
                    }
                }
                break;
            
            case 3:
                if(strcmp(d->nomeServidor, valor) == 0) {
                    if(menu == 3) {
                        printSearchRegister(c,d);
                        numReg++;
                    }
                    else if(menu == 4) {
                        removeRegister(fileIn, d);
                    }
                }
                break;
            
            case 4:
                if(strcmp(d->cargoServidor, valor) == 0) {
                    if(menu == 3) {
                        printSearchRegister(c,d);
                        numReg++;
                    }
                    else if(menu == 4) {
                        removeRegister(fileIn, d);
                    }
                }
                break;

            }
        }
    }
    if(menu == 3) {
        if(numReg == 0) printf("Registro inexistente.");
        else printf("Número de páginas de disco acessadas: %ld", (ftell(fileIn)/PageSize) + 1);
    }
    fseek(fileIn, 0, SEEK_SET);
    fwrite(&aux,1,1,fileIn);
    return;
}

//**************************************************************************************//

void writeRegister(dados *d) {
    char *aux = (char*) malloc(sizeof(char)*100);
    scanf("%s%*c", &aux);
    if(strcmp(aux,"NULO") == 0) {
        d->idServidor = -1;
    } else d->idServidor = atoi(aux);
    scanf("%s%*c", &aux);
    if(strcmp(aux,"NULO") == 0) {
        d->salarioServidor = -1;
    } else d->salarioServidor = sscanf(aux, "%lf", &d->salarioServidor);
    scanf("%s%*c", &aux);
    if(strcmp(aux,"NULO") == 0) {
    } else d->telefoneServidor = aux;
    scan_quote_string(d->cargoServidor);
    d->tamCargoServidor = strlen(d->cargoServidor) + 2;//tag + \0
    d->tamanhoRegistro += d->tamCargoServidor + 4;//int tamanho + tag + size of nome
    scan_quote_string(d->nomeServidor);    
    d->tamNomeServidor = strlen(d->nomeServidor) + 2;//tag + \0
    d->tamanhoRegistro += d->tamNomeServidor + 4;//int tamanho + tag + size of nome
    free(aux);
    return;
}

//
long int findPlace(FILE *fileIn, int regSize) {
    long int prev, cur, next;
    int size;
    char aux;
    fseek(fileIn, 1, SEEK_SET);                 //vai no topo da lista
    fread(&cur, sizeof(long), 1, fileIn);       //pega posicao
    if(cur == -1) {                             //se for -1, retorna o fim do arq
        fseek(fileIn, 0, SEEK_END);
        return(ftell(fileIn));
    }
    prev = cur;
    do {                                        //enquanto não acabar a lista
        fseek(fileIn, cur, SEEK_SET);           //vai pra posicao do atual
        fread(&aux, sizeof(char), 1, fileIn);   //verifica se foi removido de vdd
        if(aux != '*') {
            printf("erro");
            return (-1)
        }
        fread(&size, sizeof(int), 1, fileIn);   //pega o tamanho do registro removido
        if(size > regSize) break;               //se for o suficiente para o novo, sai do loop
        prev = cur;
        fread(&cur, sizeof(long), 1, fileIn);   //continua para proximo registro da lista
    }while(cur != -1);
    if(cur == -1) {                             //se saiu do loop com -1, retorna a posicao do final do arq
        fseek(fileIn, 0, SEEK_END);
        return(ftell(fileIn));
    }
    fread(&next, sizeof(long), 1, fileIn);      //caso contrario, pega a posicao do proximo da lista
    fseek(fileIn, prev+5, SEEK_SET);            //volta no encadeamento do anterior
    fwrite(&next, sizeof(long), 1, fileIn);     //escreve posicao do prox
    return(cur);                                //retorna posicao do atual
}


//
void addRegister(char *name) {
    FILE *fileIn = openFile(name, ".bin"); //abre o arquivo
    long int pos;                          //posicao para escrever o registro
    int tam;
    if(fileIn == NULL) {                   //verifica se tudo ocorreu corretamente
        printf("Falha no processamento do arquivo.");
        return;
    }
    dados *d = makeRegister();             //cria um registro
    clearRegister(d);
    writeRegister(d);                      //preenche ele com os dados fornecidos pelo usuario
    pos = findPlace(fileIn, d->tamanhoRegistro);//encontra uma posicao livre
    fseek(fileIn, pos+1, SEEK_SET);        //vai para a posicao com o tamanho do registro
    fread(&tam, sizeof(int), 1, fileIn);   //pega o tamanho do registro removido
    fseek(fileIn, pos, SEEK_SET);          //volta para o inicio do registro
    printBinRegister(d, fileIn);           //escreve o registro no arquivo
    fillAt(tam - (d->tamanhoRegistro), fileIn);//completa com @y
    return;
}


