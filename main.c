
#include <stdio.h>
#include <stdlib.h>
#include "arq.h"
#include "escrevernatela.h"
#include "index.h"

/*
*Codigo escrito com base no programa de Leandro Satoshi
* Dupla:
* Pedro Henrique Nieuwenhoff 10377729
* Leandro Satoshi de Siqueira 10893103
*/

int main() {
    int menu, n;
    char name[40];
    char campo[40];
    char valor[40];
    char valorIndex[40];
    char campoNew[40];
    char valorNew[40];
    int pagA,pagB;
    scanf("%d", &menu);
    scanf("%s", name);
    switch (menu) {
        case 1:
            makeBin(name);
            break;

        case 2:
            readBin(name);
            break;

        case 3:
            scanf("%s", campo);
            scan_quote_string(valor);
            trim(campo);
            trim(valor);
            searchBin(name, campo, valor, menu);
            break;

        case 4:
            scanf("%d", &n);
            for (int i = 0; i < n; i++) {
                scanf("%s", campo);
                scan_quote_string(valor);
                trim(valor);
                searchBin(name, campo, valor, menu);
            }
            binarioNaTela2(name);
            break;

        case 5:
            scanf("%d", &n);
            for (int i = 0; i < n; i++) {
                addRegister(name);
            }
            binarioNaTela2(name);
            break;

        case 6:
            scanf("%d", &n);
            for (int i = 0; i < n; i++) {
                scanf("%s", campo);
                scan_quote_string(valor);
                trim(valor);
                scanf("%s", campoNew);
                scan_quote_string(valorNew);
                trim(valorNew);
                updateRegister(name, campo, valor, campoNew, valorNew);
            }
            binarioNaTela2(name);
            break;

        case 7:
            scanf("%s", valor);
            sortFile(name, valor);
            binarioNaTela2(valor);
            break;

        case 8:
            scanf("%s", valor);
            scanf("%s", valorNew);
            mergeFiles(name, valor, valorNew);
            binarioNaTela2(valorNew);
            break;

        case 9:
            scanf("%s", valor);
            scanf("%s", valorNew);
            intersecFiles(name, valor, valorNew);
            binarioNaTela2(valorNew);
            break;

        case 10:
            scanf("%s", valor);
            makeIndex(name, valor);
            binarioNaTela2(valor);
            break;

        case 11:
            scanf("%s", valorIndex);  // le o arquivo index
            scanf("%s", campoNew);    // le o campo nomeServidor
            scanf(" %[^\n]s", valorNew);  // le o valor do campo de entrada (nomeServidor)
            searchBinwithIndex(name, valorIndex, campoNew, valorNew);
            break;

        case 12:
            scanf("%s", valorIndex); //nome do arquivo de indice
            scanf("%d", &n);          //numero de remocoes
            for(int i = 0; i < n; i++) {
                scanf("%s", campo);      //campo de busca, sempre nomeSevidor para indice
                scan_quote_string(valor);//nome do servidor
                trim(valor);
                removeRegisterI(name, valorIndex, valor);
            }
            binarioNaTela2(valorIndex);
            break;

        case 13:
            scanf("%s", valorIndex);    // le o arquivo de indice
            scanf("%d", &n); // le o numero de insercoes
            for(int i = 0; i < n; i++) {
                addRegisterIndex(name, valorIndex);
            }
            binarioNaTela2(valorIndex);
            break;

        case 14:
            scanf("%s", valorIndex); //arquivo de indice
            scanf("%s", campo);      //campo a ser buscado
            scanf(" %[^\n]s", valor);//valor do campo
            trim(campo);
            trim(valor);
            printf("*** Realizando a busca sem o auxílio de índice\n");
            pagA = searchBin(name, campo, valor, 3);
            if(pagA < 0) {
                pagA *= -1;
                printf("\nNúmero de páginas de disco acessadas: %d", pagA);
            }
            printf("\n*** Realizando a busca com o auxílio de um índice secundário fortemente ligado\n");
            pagB = searchBinwithIndex(name, valorIndex, campo, valor);
            if(pagB < 0) {
                pagB *= -1;
                printf("\nNúmero de páginas de disco para carregar o arquivo de índice: %d\n",pagB);
                printf("Número de páginas de disco para acessar o arquivo de dados: 1\n");
                pagB = 1;
            }
            printf("\nA diferença no número de páginas de disco acessadas: %d", (pagA-pagB));
            break;

        default:
            break;
    }
    return 0;
}