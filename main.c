#include <stdio.h>
#include <stdlib.h>
#include "arq.h"
#include "extra.h"

int main() {
    int menu, n;
    scanf("%d", &menu);
    char name[40];
    char campo[40];
    char valor[40];
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
        break;

    case 5:
        break;

    default:
        break;

    }
}