
#include <stdio.h>
#include <stdlib.h>
#include "arq.h"
#include "escrevernatela.h"
#include "index.h"

int main() {
    int menu, n;
    char name[40];
    char campo[40];
    char valor[40];
    char valorIndex[40];
    char campoNew[40];
    char valorNew[40];
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

            n = returnNumReg(valorIndex);  // obtem o numReg

            regI* arrayIndex = alocaArrayRegI2d(arrayIndex, n);  // aloca o arrayIndex

            int nPaginaCarregar = returnArrayIndex(valorIndex, arrayIndex, n);  // aloca os dados do index num array
            printf("nPaginaCarregar = %d\n", nPaginaCarregar);

            FILE* binarioEntrada = fopen(name, "rb");
            FILE* fileIndex = fopen(valorIndex, "rb");
            if (binarioEntrada == NULL || fileIndex == NULL) {
                printf("Falha no processamento do arquivo.");
                return 0;
            }

            fgetc(fileIndex);
            fread(&n, sizeof(int), 1, fileIndex);  //  obtem o tamanho do array de registros do index

            long int* byteOffset = alocaArrayInt2d(byteOffset, n);
            int nPaginaAcessar = buscaNomeIndex(arrayIndex, valorNew, n, byteOffset);

            printRegisterIndex(binarioEntrada, byteOffset, n);
            fclose(binarioEntrada);

            printf("\n");
            printf(
                "Número de páginas de disco para carregar o arquivo de índice: "
                "%d\n",
                nPaginaCarregar);
            printf(
                "Número de páginas de disco para acessar o arquivo de dados: "
                "%d\n",
                nPaginaAcessar);
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

        default:
            break;
    }
    return 0;
}