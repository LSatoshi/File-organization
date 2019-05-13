#include <stdio.h>
#include <stdlib.h>
#include "arq.h"
#include "extra.h"

int main() {
    int menu, n;
    char name[40];
    char campo[40];
    char valor[40];
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
            scanf("%s %[^\n]", campo, valor);
            searchBin(name, campo, valor, menu);
            break;

        case 4:
            scanf("%d", &n);
            for(int i = 0; i < n; i++) {
                scanf("%s %[^\n]", campo, valor);
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

        default:
            break;

    }
    return 0;
}