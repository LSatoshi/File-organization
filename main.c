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
    char campoNew[40];
    char valorNew[40];
    scanf("%d", &menu);
    scanf("%s", name);
    switch(menu) {
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
            for(int i = 0; i < n; i++) {
                scanf("%s", campo);
                scan_quote_string(valor);
                trim(valor);
                searchBin(name, campo, valor, menu);
            }
            binarioNaTela2(name);
            break;

        case 5:
            scanf("%d", &n);
            for(int i = 0; i < n; i++) {
                addRegister(name);
            }
            binarioNaTela2(name);
            break;

        case 6:
            scanf("%d", &n);
            for(int i = 0; i < n; i++) {
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

        default:
            break;

    }
    return 0;
}