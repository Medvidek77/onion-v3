#include <stdio.h>
#include <stdint.h>
#include "fe.h"

int main() {
    fe z = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    fe inv_c;
    fe_invert(inv_c, z);

    printf("C invert: ");
    for(int i=0; i<10; i++) printf("%d ", inv_c[i]);
    printf("\n");
    return 0;
}
