#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arq.h"
#include "escrevernatela.h"
#define PageSize 32000
#define MAX 10000

//funcao que atualiza o status do arquivo
void setStatus(FILE *file, int n) {
    if(file != NULL) {
        long int aux = ftell(file);
        char status;
        if(n == 0) status = '0';
        if(n == 1) status = '1';
        fseek(file, 0, SEEK_SET);
        fwrite(&status, sizeof(char), 1, file);
        fseek(file, aux, SEEK_SET);
    }
    return;
}

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

dados **makeRegArr(int n) {
    dados **d = (dados**) malloc(sizeof(dados)*n);
    return(d);
}

//limpa o resgistro de dados
void clearRegister(dados *d) {
    d->removido = '-';
    d->tamanhoRegistro = 34; //tamanho dos campos fixos
    d->encadeamentoLista = -1;
    d->idServidor = -1;
    d->salarioServidor = -1;
    for(int i = 0; i < 14; i++) {
        d->telefoneServidor[i] = '@';
    }
    d->tamNomeServidor = 0;
    d->tag4 = 'n';
    d->nomeServidor = (char*) malloc(sizeof(char) * 100);
    d->nomeServidor[0] = '\0';
    d->tamCargoServidor = 0;
    d->tag5 = 'c';
    d->cargoServidor = (char*) malloc(sizeof(char) * 100);
    d->cargoServidor[0] = '\0';
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
    if(n > 0) {
        char c = '@';
        for(int i = 0; i < n; i++){
            fwrite(&c, sizeof(char), 1, file);
        }
    }
    return;
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
    fclose(fileIn);
    fclose(fileOut);
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
    fseek(fileIn, 0,SEEK_SET);
    fread(&c->status, sizeof(char), 1, fileIn);
    if(c->status == '0') return;
    fread(&c->topoLista, sizeof(long int), 1, fileIn);
    for(int i = 0; i < 5; i++){
        fread(&c->tags[i], sizeof(char), 1, fileIn);
        for(int j = 0; j < 40; j++) {
            fread(&c->campos[i][j], sizeof(char), 1, fileIn);
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
    fclose(fileIn);
    return;
}

//**************************************************************************************//

//adiciona um registro removido na lista encadeada de removidos
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
                fread(&tamProx, sizeof(int), 1, fileIn);//pega o tamanho do registro
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

//remove logicamente um registro, adiciona na lista encadeada e preenche com @
void removeRegister(FILE *fileIn, dados *d) {
    char c = '*';
    int aux = 0;
    aux -= (d->tamanhoRegistro + 5);
    fseek(fileIn, aux, SEEK_CUR);//volta para inicio do registro, no arquivo
    long int atual = ftell(fileIn);                    //guarda a posicao
    fwrite(&c, sizeof(char), 1, fileIn);               //marcar como removido
    insertList(atual, d->tamanhoRegistro, fileIn);     //chama funcao de inserir na lista
    fseek(fileIn, atual + 13, SEEK_SET); //vai para posicao apos encadeamento lista (char + int + long = 13)
    fillAt(d->tamanhoRegistro - 8, fileIn);   //preenche com @, -8 pois em tamanhoRegistro ja esta incluso o long do encademento
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
    if(d->telefoneServidor[0] != '@' && d->telefoneServidor[0] != '\0') printf("%.14s\n", d->telefoneServidor);
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
    tag = '0';
    if(strcmp(c,"idServidor") == 0) tag ='i';
    if(strcmp(c,"salarioServidor") == 0) tag ='s';
    if(strcmp(c,"telefoneServidor") == 0) tag ='t';
    if(strcmp(c,"nomeServidor") == 0) tag ='n';
    if(strcmp(c,"cargoServidor") == 0) tag ='c';
    return(tag);
}

//funcao principal da funcionalidade 3, busca um registro em um arquivo binario valido
//tambem utilizado para remocao na funcionalidade 4
void searchBin(char *name, char *campo, char *valor, int menu) {
    FILE *fileIn = fopen(name, "rb+");
    int id, numReg = 0, numCampo = -1;
    double salario;
    char tag = tagCampo(campo);
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
    setStatus(fileIn, 0);
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
                    setStatus(fileIn, 1);
                    fclose(fileIn);
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
    free(c);
    setStatus(fileIn, 1);
    fclose(fileIn);
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//Trabalho 2 /////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//************************************************************************************//
//will substitute serchbin later on - funcao de busca mais modularizada para ser usada futuramente

int cmpReg(dados *a, dados *b, char *campo) { //compara dois registro
    if(a != NULL && b != NULL) { //se nenhum for nulo
        //compara todos os campos se houver algum igual, retorna 1
        if(strcmp(campo,"idServidor") == 0) {
            if(a->idServidor == b->idServidor) {
                return 1;
            }
        }
        else if(strcmp(campo,"salarioServidor") == 0){
            if(a->salarioServidor == b->salarioServidor) return 1;
        }
        else if(strcmp(campo,"telefoneServidor") == 0) {
            if(strcmp(a->telefoneServidor, b->telefoneServidor ) == 0) return 1;
        }
        else if(strcmp(campo,"nomeServidor") == 0) {
            if(strcmp(a->nomeServidor, b->nomeServidor) == 0) return 1;
        }
        else if(strcmp(campo,"cargoServidor") == 0) {
            if(strcmp(a->cargoServidor, b->cargoServidor) == 0) return 1;
        }
    }
    return 0; //caso nao exista um dos registros ou nao exista camṕos iguais, retorna zero
}

//funcao que faz uma busca sequencial em busca de um registro que possua o campo especificado igual ao dado
dados *seqSearch(FILE *fileIn, dados *search, char *campo) {
    dados *d = makeRegister();
    int aux = 0;
    while(aux != 1 && fgetc(fileIn) != EOF) {
        fseek(fileIn, -1, SEEK_CUR);
        clearRegister(d);
        readBinRegister(fileIn, d);
        if(d->removido == '*') {
            fseek(fileIn, d->tamanhoRegistro, SEEK_CUR);
        }
        else aux = cmpReg(d, search, campo);
    }
    if(aux == 1) {
        return d;
    }
    else {
        return NULL;
    }
        
}


//funcao que atualiza um campo de um registro
void updateField(dados *d, char *campo, char *valor) {
    if(d != NULL) { 
        if(strcmp(campo,"idServidor") == 0) {
            d->idServidor = atoi(valor);
        }
        if(strcmp(campo,"salarioServidor") == 0){
            d->salarioServidor = atof(valor);
        }
        if(strcmp(campo,"telefoneServidor") == 0) {
            strcpy(d->telefoneServidor,valor);
        }
        if(strcmp(campo,"nomeServidor") == 0) {
            strcpy(d->nomeServidor, valor);
            d->tamNomeServidor = 0;
            if(strlen(d->nomeServidor) > 0) {
                d->tamNomeServidor = strlen(d->nomeServidor) + 2;
            }
        }
        if(strcmp(campo,"cargoServidor") == 0) {
            strcpy(d->cargoServidor, valor);
            d->tamCargoServidor = 0;
            if(strlen(d->cargoServidor) > 0) {
                d->tamCargoServidor = strlen(d->cargoServidor) + 2; //tag + \0
            }
        }
        d->tamanhoRegistro = 34;
        if(d->tamNomeServidor > 0) {
            d->tamanhoRegistro += d->tamNomeServidor + 4;
        }
        if(d->tamCargoServidor > 0) {
            d->tamanhoRegistro += d->tamCargoServidor + 4;
        }
    }
    return;
}

//funcao que atualiza um registro no arquivo binario, funcionalidade 6
void updateRegister(char *name, char *campo, char *valor, char *campoNew, char *valorNew) {
    FILE *fileIn = openFile(name, ".bin");
    int espaco;
    long int inicioRegistro, continuacao;
    if(fileIn == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    setStatus(fileIn, 0); //altera status para zero
    dados *search = makeRegister(), *d = makeRegister();//cria registros auxiliares, um com os dados desejados e outro com os dados encontrados
    clearRegister(search);
    updateField(search, campo, valor);//copia valores desejados para search
    fseek(fileIn, PageSize, SEEK_SET);
    while(fgetc(fileIn) != EOF) {
        fseek(fileIn, -1, SEEK_CUR);
        clearRegister(d);
        d = seqSearch(fileIn, search, campo);//busca um registro que contenha algum campo igual ao registro search
        if(d != NULL) {
            continuacao = ftell(fileIn);
            espaco = d->tamanhoRegistro;//guarda o tamanho do registro encontrado
            inicioRegistro = ftell(fileIn) - (d->tamanhoRegistro) - 5;// e a posicao dele
            search->tamanhoRegistro = d->tamanhoRegistro;//gambiarra para guardar o tamanho antes de atualizar
            updateField(d, campoNew, valorNew);//copia os valores atualizados para o registro
            if(d->tamanhoRegistro > espaco) {
                inicioRegistro = findPlace(fileIn, d);//se o novo registro for maior, procura um lugar para ele
                removeRegister(fileIn, search);       //usa o tamanho antigo para excluir o registro desatualizado
            }
            else {
                d->tamanhoRegistro = espaco;
            }
            fseek(fileIn, inicioRegistro, SEEK_SET);
            printBinRegister(d, fileIn);//escreve o registro atualizado
            fillAt(espaco - d->tamanhoRegistro, fileIn);
            if(strcmp(campo, "idServidor") == 0) break;
            fseek(fileIn, continuacao, SEEK_SET);
        }
    }
    if(d != NULL) {
        freeRegister(d);
    }
    freeRegister(search);
    setStatus(fileIn, 1); //funcao finalizada com sucesso, retorna status para 1
    fclose(fileIn);
    return;
}



//**************************************************************************************//

//funcao que recebe input do usuario para criar novo registro, precisa de um nome melhor ainda
void writeRegister(dados *d) { 
    char *aux = (char*) malloc(sizeof(char)*100);
    scanf("%s%*c", aux);
    if(strcmp(aux,"NULO") == 0) {
        d->idServidor = -1;
    } else d->idServidor = atoi(aux);
    scanf("%s%*c", aux);
    if(strcmp(aux,"NULO") == 0) {
        d->salarioServidor = -1;
    } else d->salarioServidor = atof(aux);
    scan_quote_string(aux);
    if(strcmp(aux,"NULO") == 0) {
    } else strcpy(d->telefoneServidor,aux);
    scan_quote_string(d->nomeServidor);
    if(strlen(d->nomeServidor) > 0) {  
        d->tamNomeServidor = strlen(d->nomeServidor) + 2;//tag + \0
        d->tamanhoRegistro += d->tamNomeServidor + 4;//int tamanho + tag + size of nome
    }
    scan_quote_string(d->cargoServidor);
    if(strlen(d->cargoServidor) > 0) {
        d->tamCargoServidor = strlen(d->cargoServidor) + 2;//tag + \0
        d->tamanhoRegistro += d->tamCargoServidor + 4;//int tamanho + tag + size of nome
    }
    free(aux);
    return;
}

//funcao que busca um lugar vazio para um registro de tamanho dado
long int findPlace(FILE *fileIn, dados *d) {
    long int prev = 1, cur, next;
    int size;
    char aux;
    fseek(fileIn, 1, SEEK_SET);                 //vai no topo da lista
    fread(&cur, sizeof(long), 1, fileIn);       //pega posicao
    if(cur == -1) {                             //se for -1, retorna o fim do arq
        fseek(fileIn, 0, SEEK_END);
        return(ftell(fileIn));
    }
    do {                                        //enquanto não acabar a lista
        fseek(fileIn, cur, SEEK_SET);           //vai pra posicao do atual
        fread(&aux, sizeof(char), 1, fileIn);   //verifica se foi removido de vdd
        if(aux != '*') {
            printf("erro");
            return (-1);
        }
        fread(&size, sizeof(int), 1, fileIn);   //pega o tamanho do registro removido
        if(size > d->tamanhoRegistro) {                    //se for o suficiente para o novo
            d->tamanhoRegistro = size;                     //guarda o tamanho do registro e sai do loop
            break;
        }
        prev = cur;
        fread(&cur, sizeof(long), 1, fileIn);   //continua para proximo registro da lista
    } while(cur != -1);
    if(cur == -1) {                             //se saiu do loop com -1, retorna a posicao do final do arq
        fseek(fileIn, 0, SEEK_END);
        return(ftell(fileIn));
    }
    fread(&next, sizeof(long), 1, fileIn);      //caso contrario, pega a posicao do proximo da lista
    if(prev != 1) {
        fseek(fileIn, prev+5, SEEK_SET);            //volta no encadeamento do anterior
    }
    else {
        fseek(fileIn, prev, SEEK_SET);              //ou para o topo da lista
    }
    fwrite(&next, sizeof(long), 1, fileIn);     //escreve posicao do prox
    return(cur);                                //retorna posicao do atual
}


//funcao principal da funcionalidade 5
void addRegister(char *name) {
    FILE *fileIn = openFile(name, ".bin"); //abre o arquivo
    setStatus(fileIn, 0);
    long int pos;                          //posicao para escrever o registro
    int tam;
    if(fileIn == NULL) {                   //verifica se tudo ocorreu corretamente
        printf("Falha no processamento do arquivo.");
        return;
    }
    dados *d = makeRegister();             //cria um registro
    clearRegister(d);
    writeRegister(d);                      //preenche ele com os dados fornecidos pelo usuario
    pos = findPlace(fileIn, d);            //encontra uma posicao livre
    fseek(fileIn, pos, SEEK_SET);          //vai para a posicao 
    printBinRegister(d, fileIn);           //escreve o registro no arquivo
    freeRegister(d);
    setStatus(fileIn, 1);
    fclose(fileIn);
    return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//Trabalho 3 /////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

//atualiza o registro anterior se o espaco na pag nao for o suficiente, retorna a posicao logica na pag
int updatePrev(FILE *fileOut, dados *curReg, dados *prevReg, int curPos) {
    fseek(fileOut, -(prevReg->tamanhoRegistro + 5), SEEK_CUR);
    prevReg->tamanhoRegistro += PageSize-curPos;
    printBinRegister(prevReg, fileOut);
    fillAt(PageSize - curPos, fileOut);
    return 0;
}


//funcao compar utilizada pela qsort
int cmpRegId(const void * a, const void * b) {
    dados *d1 = (dados *) a;
    dados *d2 = (dados *) b;
    return((d1->idServidor) - (d2->idServidor));
}

//funcao principal da funcionalidade 7, cria um binario com os registros ordenador a partir de outro binario
//to do: printando registros com tamanho errado
void sortFile(char *nameIn, char *nameOut) {
    int numReg, atual = 0;
    FILE *fileIn = openFile(nameIn, ".bin");//abre com a funcao q ja verifica
    FILE *fileOut = fopen(nameOut, "wb+"); //criar um novo para escrita
    dados registerArr[MAX];//cria um vetor de ponteiros para registros
    if(fileIn == NULL || fileOut == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }
    cabecalho *header = makeHeader();//cria um cabecalho;
    readBinHeader(fileIn, header);   //le o cabecalho do arquivo de entrada
    header->topoLista = -1;          //zera o topo da lista
    fseek(fileIn, PageSize, SEEK_SET);//vai para os registros do arquivo de entrada
    for(numReg = 0;fgetc(fileIn) != EOF; numReg++) {//le os registros enquanto não encontrar EOF, e anota quantos registro em numReg
        fseek(fileIn, -1,SEEK_CUR);
        clearRegister(&registerArr[numReg]);  //limpa registro
        readBinRegister(fileIn, &registerArr[numReg]);// le o registro
        if(registerArr[numReg].removido == '*') {//se for removido
            fseek(fileIn, registerArr[numReg].tamanhoRegistro, SEEK_CUR);//avanca para o proximo registro
            numReg--;        //reduz um numReg, ja que nao leu um registro valido
        }
        updateField(&registerArr[numReg],"telefoneServidor",registerArr[numReg].telefoneServidor);//usa update field para atualizar o tamanho
    }
    qsort(registerArr, numReg, sizeof(dados), cmpRegId);//usa q sort para ordenar o array
    //comeca escrever no fileOut
    writeHeader(header, fileOut);//escreve o cabecalho
    setStatus(fileOut, 0); //muda status do arquivo de saida para 0
    fillAt((PageSize) - 214, fileOut);//completa pagina com arroba
    for(int i = 0; i < numReg; i++) {//escreve registros
        if(atual + registerArr[i].tamanhoRegistro + 5 > PageSize) {
            atual = updatePrev(fileOut, &registerArr[i], &registerArr[i-1], atual);
        }
        atual += registerArr[i].tamanhoRegistro + 5;//se possuir espaco na pag, adiciona o tamanho no atual, para controle
        printBinRegister(&(registerArr[i]),fileOut);//escreve o registro
    }
    for(int i = 0; i < numReg; i++) {//da free em todos ponteiros de registro do array
        free(registerArr[i].nomeServidor);
        free(registerArr[i].cargoServidor);
    }
    setStatus(fileOut, 1); //muda status do arquivo de saida para 1
    free(header);
    fclose(fileIn);
    fclose(fileOut);
    return;
}

//funcao principal da func 8, da merge em dois arquivos ordenados
void mergeFiles(char *nameInA, char *nameInB, char *nameOut) {
    FILE *fileInA = fopen(nameInA, "rb+");//abre em modo de leitura
    FILE *fileInB = fopen(nameInB, "rb+");
    FILE *fileOut = fopen(nameOut, "wb+");//abre em modo de escrita
    if(fileInA == NULL || fileInB == NULL || fileOut == NULL) {//verifica se abriu
        printf("Falha no processamento do arquivo.");
        return;
    }
    int atual = 0, end = -1;
    char status;
    cabecalho *header = makeHeader();
    readBinHeader(fileInB, header);
    status = header->status;
    readBinHeader(fileInA, header);
    if(header->status == '0' || status == '0') {//verifica status de ambos arquivos de entrada
        printf("Falha no processamento do arquivo.");
        return;
    }
    //cria registros, aloca espaco, e vai para o inicio da pagina de registros
    dados *regA = makeRegister();
    dados *regB = makeRegister();
    dados *ant = makeRegister(), *d;
    clearRegister(regA);
    clearRegister(regB);
    fseek(fileInA, PageSize, SEEK_SET);
    fseek(fileInB, PageSize, SEEK_SET);
    readBinRegister(fileInA, regA);
    readBinRegister(fileInB, regB);
    header->topoLista = -1;
    writeHeader(header, fileOut);     //escreve o cabecalho
    setStatus(fileOut, 0);            //muda status do arquivo de saida para 0
    fillAt((PageSize) - 214, fileOut);//completa pagina com arroba
    while(end == -1) {                //variavel end utilizada para saber se ja terminou algum arquivo e qual foi
        if(regA->idServidor <= regB->idServidor) {      //se o id de regA for menor ou igual o regB
            if(regA->idServidor == regB->idServidor) {  //se for igual avanca pro prox registro em fileB
                clearRegister(regB);
                if(fgetc(fileInB) == EOF) end += 2;     //se estiver no fim do arquivo B, soma 2 a end
                else {
                    fseek(fileInB, -1, SEEK_CUR);
                    readBinRegister(fileInB, regB);
                }
            }
            d = regA;                               //d aponta para registro
            regA = makeRegister();                  //cria novo registro para regA
            clearRegister(regA);
            if(fgetc(fileInA) == EOF) end += 1;     //se acabar arquivo A, soma 1 a end
            else {
                fseek(fileInA, -1, SEEK_CUR);       //se nao acabar o arquivo, le o prox registro
                readBinRegister(fileInA, regA);
            }
        }
        else {                                  //se id de regA for maior que o id de regB 
            d = regB;                           //d aponta para registro
            regB = makeRegister();              //cria novo registro para regB
            clearRegister(regB);
            if(fgetc(fileInB) == EOF) end += 2; //se acabar arquivo B, soma 2 a end
            else {
                fseek(fileInB, -1, SEEK_CUR);
                readBinRegister(fileInB, regB); //se nao acabar o arquivo, le o prox registro
            }
        }
        updateField(d,"telefoneServidor", d->telefoneServidor);//utiliza funcao update para atualizar o tamanho do registro
        if(atual + d->tamanhoRegistro + 5 > PageSize) 
            atual = updatePrev(fileOut, d, ant, atual);        //se nao houver espaco na pagina, preenche com @ e atualiza tam do anterior
        printBinRegister(d, fileOut);                          //imprime o registro no arq de saida
        atual += d->tamanhoRegistro + 5;                       //aumenta marcador atual
        freeRegister(ant);                                     //da free no anterior
        ant = d;                                               //anteior aponta para o registro q acabou de ser escrito
    }
    //nesse ponto um ou ambos od arquivos ja acabaram
    //end = 0, arq A acabou; end = 1 arq B acabou; end = 2, ambos acabaram
    if(end == 0) {
        while(1) {
            //enquanto houver registros, faz as operacoes necessarias e imprime no arquivo de saida
            updateField(regB,"telefoneServidor", regB->telefoneServidor);
            if(atual + regB->tamanhoRegistro + 5 > PageSize)
                atual = updatePrev(fileOut, regB, ant, atual);
            printBinRegister(regB, fileOut);
            atual += regB->tamanhoRegistro + 5;
            freeRegister(ant);
            ant = regB;
            regB = makeRegister();
            clearRegister(regB);
            if(fgetc(fileInB) == EOF) break;
            fseek(fileInB, -1, SEEK_CUR);
            readBinRegister(fileInB, regB);
        }
    }

    if(end == 1) {
        while(1) {
            //enquanto houver registros, faz as operacoes necessarias e imprime no arquivo de saida
            updateField(regA,"telefoneServidor", regA->telefoneServidor);
            if(atual + regA->tamanhoRegistro + 5 > PageSize)
                atual = updatePrev(fileOut, regA, ant, atual);
            printBinRegister(regA, fileOut);
            atual += regA->tamanhoRegistro + 5;
            freeRegister(ant);
            ant = regA;
            regA = makeRegister();
            clearRegister(regA);
            if(fgetc(fileInA) == EOF) break;
            fseek(fileInA, -1, SEEK_CUR);
            readBinRegister(fileInA, regA);
        }
    }
    setStatus(fileOut, 1);//muda status para 1 no arq de saida
    free(header);         //fecha arquivos e da free nas structs
    fclose(fileInA);
    fclose(fileInB);
    fclose(fileOut);
    return;
}


//funcao principal da func 9, cria um arquivo com a interseccao dos registros de dois arquivos
void intersecFiles(char *nameInA, char *nameInB, char *nameOut) {
    FILE *fileInA = fopen(nameInA, "rb+");//abre em modo de leitura
    FILE *fileInB = fopen(nameInB, "rb+");
    FILE *fileOut = fopen(nameOut, "wb+");//abre em modo de escrita
    if(fileInA == NULL || fileInB == NULL || fileOut == NULL) {//verifica se abriu corretamente
        printf("Falha no processamento do arquivo.");
        return;
    }
    int atual = 0, end = 0;
    char status;
    cabecalho *header = makeHeader();
    readBinHeader(fileInB, header);
    status = header->status;
    readBinHeader(fileInA, header);
    if(header->status == '0' || status == '0') {//verifica se arquivos de entrada estao consistentes
        printf("Falha no processamento do arquivo.");
        return;
    }
    dados *regA = makeRegister();
    dados *regB = makeRegister();
    dados *ant = makeRegister();
    clearRegister(regA);
    clearRegister(regB);
    fseek(fileInA, PageSize, SEEK_SET);
    fseek(fileInB, PageSize, SEEK_SET);
    readBinRegister(fileInA, regA);
    readBinRegister(fileInB, regB);
    header->topoLista = -1;
    writeHeader(header, fileOut);     //escreve o cabecalho
    setStatus(fileOut, 0);            //muda status do arquivo de saida para 0
    fillAt((PageSize) - 214, fileOut);//completa pagina com arroba
    while(end == 0) {//enquanto nao chegar ao fim de um dos arquivos
        if(regA->idServidor < regB->idServidor) {//se id do registro lido de arq A for menor, le o prox desse arquivo
            if(fgetc(fileInA) == EOF) end = 1;
            else {
                fseek(fileInA, -1, SEEK_CUR);
                clearRegister(regA);
                readBinRegister(fileInA, regA);
            }
        }
        if(regA->idServidor > regB->idServidor) {//se o id do reg lido de arq B for menor, le o prox desse arquivo
            if(fgetc(fileInB) == EOF) end = 1;
            else {
                fseek(fileInB, -1, SEEK_CUR);
                clearRegister(regB);
                readBinRegister(fileInB, regB);
            }
        }
        if(regA->idServidor == regB->idServidor) {//se forem iguais, faz as devidas verificacoes e imprime o registro do arquivo A
            updateField(regA,"telefoneServidor", regA->telefoneServidor);
            if(atual + regA->tamanhoRegistro + 5 > PageSize) 
                atual = updatePrev(fileOut, regA, ant, atual);
            printBinRegister(regA, fileOut);
            atual += regA->tamanhoRegistro + 5;
            freeRegister(ant);
            ant = regA;
            if(fgetc(fileInA) == EOF) end = 1;
            else {                              //le o proximo registro de ambos arquivos
                regA = makeRegister();
                clearRegister(regA);
                fseek(fileInA, -1, SEEK_CUR);
                readBinRegister(fileInA, regA);
            }
            if(fgetc(fileInB) == EOF) end = 1;
            else {
                clearRegister(regB);
                fseek(fileInB, -1, SEEK_CUR);
                readBinRegister(fileInB, regB);
            }
        }
    }
    setStatus(fileOut, 1);
    freeRegister(regA);
    freeRegister(regB);
    free(header);
    fclose(fileInA);
    fclose(fileInB);
    fclose(fileOut);
    return;
}