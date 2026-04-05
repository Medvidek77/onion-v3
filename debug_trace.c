#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fe.h"
#include "ge.h"
#include "sc.h"
#include <sodium.h>

extern void fe_sq_glsl(int h[10], const int f[10]);
extern void fe_mul_glsl(int h[10], const int f[10], const int g[10]);
extern void fe_invert_glsl(int out_fe[10], const int z[10]);
extern void ge_madd_precomp_glsl(int r_X[10], int r_Y[10], int r_Z[10], int r_T[10],
                          const int p_YplusX[10], const int p_YminusX[10], const int p_Z2[10], const int p_T[10],
                          const int q_YplusX[10], const int q_YminusX[10], const int q_Z[10], const int q_T2d[10]);

int main() {
    unsigned char seed[32] = {0}; // deterministic
    unsigned char h[64];
    crypto_hash_sha512(h, seed, 32);
    h[0] &= 248; h[31] &= 127; h[31] |= 64;

    ge_p3 base_p3;
    ge_scalarmult_base(&base_p3, h);

    ge_p3 G, sum;
    const unsigned char one[32] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ge_scalarmult_base(&G, one);
    ge_p1p1 p1; ge_p2 p2; ge_p3 cur_p3;
    ge_p3_to_p2(&p2, &G);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1);
    ge_p3_to_p2(&p2, &cur_p3);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1);
    ge_p3_to_p2(&p2, &cur_p3);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1); // 8*G

    ge_cached step_cached;
    ge_p3_to_cached(&step_cached, &cur_p3);

    ge_p3_0(&sum);
    // Add 5 times
    for(int i=0; i<5; i++) {
        ge_add(&p1, &sum, &step_cached);
        ge_p1p1_to_p3(&sum, &p1);
    }

    // CPU correct madd
    ge_cached q_cpu;
    ge_p3_to_cached(&q_cpu, &sum); // sum is 5*8*G
    ge_add(&p1, &base_p3, &q_cpu);
    ge_p3 R_cpu;
    ge_p1p1_to_p3(&R_cpu, &p1);

    // GLSL emulation madd
    int p_YplusX[10], p_YminusX[10], p_Z2[10], p_T[10];
    fe_add(p_YplusX, base_p3.Y, base_p3.X);
    fe_sub(p_YminusX, base_p3.Y, base_p3.X);
    fe_add(p_Z2, base_p3.Z, base_p3.Z);
    for(int i=0; i<10; i++) p_T[i] = base_p3.T[i];

    int R_X[10], R_Y[10], R_Z[10], R_T[10];
    ge_madd_precomp_glsl(R_X, R_Y, R_Z, R_T, p_YplusX, p_YminusX, p_Z2, p_T,
                         q_cpu.YplusX, q_cpu.YminusX, q_cpu.Z, q_cpu.T2d);

    printf("Match X? %d\n", memcmp(R_cpu.X, R_X, 40) == 0);
    printf("Match Y? %d\n", memcmp(R_cpu.Y, R_Y, 40) == 0);
    printf("Match Z? %d\n", memcmp(R_cpu.Z, R_Z, 40) == 0);
    printf("Match T? %d\n", memcmp(R_cpu.T, R_T, 40) == 0);

    // If points match, then issue is in ge_p3_tobytes!
    return 0;
}
