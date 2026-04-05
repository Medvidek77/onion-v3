#include <stdio.h>
#include <stdint.h>
#include "fe.h"
#include "ge.h"

int main() {
    ge_p3 p;
    ge_scalarmult_base(&p, (const unsigned char*)"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

    ge_cached q;
    ge_p3 q3;
    const unsigned char two[32] = {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ge_scalarmult_base(&q3, two);
    ge_p3_to_cached(&q, &q3);

    // ge.c ge_add
    ge_p1p1 r;
    ge_add(&r, &p, &q);
    ge_p3 r3;
    ge_p1p1_to_p3(&r3, &r);

    // GLSL emulation
    fe C, B, D, t1, A;
    fe p_YplusX, p_YminusX, p_Z2, p_T;
    fe_add(p_YplusX, p.Y, p.X);
    fe_sub(p_YminusX, p.Y, p.X);
    fe_add(p_Z2, p.Z, p.Z);
    for(int i=0; i<10; i++) p_T[i] = p.T[i];

    fe_mul(C, p_YplusX, q.YplusX);
    fe_mul(B, p_YminusX, q.YminusX);
    fe_mul(D, q.T2d, p_T);
    fe_mul(t1, p_Z2, q.Z);
    fe_sub(A, C, B);
    fe_add(B, C, B);
    fe_add(C, t1, D);
    fe_sub(D, t1, D);

    fe glsl_X, glsl_Y, glsl_Z, glsl_T;
    fe_mul(glsl_X, A, D);
    fe_mul(glsl_Y, B, C);
    fe_mul(glsl_Z, C, D);
    fe_mul(glsl_T, A, B);

    int ok = 1;
    for(int i=0; i<10; i++) if(r3.X[i] != glsl_X[i]) ok = 0;
    for(int i=0; i<10; i++) if(r3.Y[i] != glsl_Y[i]) ok = 0;
    for(int i=0; i<10; i++) if(r3.Z[i] != glsl_Z[i]) ok = 0;
    for(int i=0; i<10; i++) if(r3.T[i] != glsl_T[i]) ok = 0;
    printf("ge_madd_precomp match? %d\n", ok);
    if(!ok) {
        printf("r3.Z: "); for(int i=0; i<10; i++) printf("%d ", r3.Z[i]); printf("\n");
        printf("gl_Z: "); for(int i=0; i<10; i++) printf("%d ", glsl_Z[i]); printf("\n");
    }
    return 0;
}
