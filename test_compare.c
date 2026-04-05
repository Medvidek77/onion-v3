#include <stdio.h>
#include <stdint.h>
#include "fe.h"

int main() {
    fe z = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    fe ref_c;
    fe_sq(ref_c, z);

    printf("REF : "); for(int i=0; i<10; i++) printf("%d ", ref_c[i]); printf("\n");
    return 0;
}
