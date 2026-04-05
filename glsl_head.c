#include "fe.h"
#include "ge.h"
#include "sc.h"
#include <sodium.h>

void fe_copy_glsl(int dst[10], const int src[10]) {
    for (int i=0; i<10; i++) dst[i] = src[i];
}
void fe_add_glsl(int h[10], const int f[10], const int g[10]) {
    for (int i=0; i<10; i++) h[i] = f[i] + g[i];
}
void fe_sub_glsl(int h[10], const int f[10], const int g[10]) {
    for (int i=0; i<10; i++) h[i] = f[i] - g[i];
}

void fe_mul_glsl(int h[10], const int f[10], const int g[10]) {
    int64_t f0 = f[0], f1 = f[1], f2 = f[2], f3 = f[3], f4 = f[4];
    int64_t f5 = f[5], f6 = f[6], f7 = f[7], f8 = f[8], f9 = f[9];
    int64_t g0 = g[0], g1 = g[1], g2 = g[2], g3 = g[3], g4 = g[4];
    int64_t g5 = g[5], g6 = g[6], g7 = g[7], g8 = g[8], g9 = g[9];
    int64_t g1_19 = 19L * g1, g2_19 = 19L * g2, g3_19 = 19L * g3, g4_19 = 19L * g4;
    int64_t g5_19 = 19L * g5, g6_19 = 19L * g6, g7_19 = 19L * g7, g8_19 = 19L * g8, g9_19 = 19L * g9;

    int64_t h0 = f0*g0 + f1*g9_19 + f2*g8_19 + f3*g7_19 + f4*g6_19 + f5*g5_19 + f6*g4_19 + f7*g3_19 + f8*g2_19 + f9*g1_19;
    int64_t h1 = f0*g1 + f1*g0    + f2*g9_19 + f3*g8_19 + f4*g7_19 + f5*g6_19 + f6*g5_19 + f7*g4_19 + f8*g3_19 + f9*g2_19;
    int64_t h2 = f0*g2 + f1*g1    + f2*g0    + f3*g9_19 + f4*g8_19 + f5*g7_19 + f6*g6_19 + f7*g5_19 + f8*g4_19 + f9*g3_19;
    int64_t h3 = f0*g3 + f1*g2    + f2*g1    + f3*g0    + f4*g9_19 + f5*g8_19 + f6*g7_19 + f7*g6_19 + f8*g5_19 + f9*g4_19;
    int64_t h4 = f0*g4 + f1*g3    + f2*g2    + f3*g1    + f4*g0    + f5*g9_19 + f6*g8_19 + f7*g7_19 + f8*g6_19 + f9*g5_19;
    int64_t h5 = f0*g5 + f1*g4    + f2*g3    + f3*g2    + f4*g1    + f5*g0    + f6*g9_19 + f7*g8_19 + f8*g7_19 + f9*g6_19;
    int64_t h6 = f0*g6 + f1*g5    + f2*g4    + f3*g3    + f4*g2    + f5*g1    + f6*g0    + f7*g9_19 + f8*g8_19 + f9*g7_19;
    int64_t h7 = f0*g7 + f1*g6    + f2*g5    + f3*g4    + f4*g3    + f5*g2    + f6*g1    + f7*g0    + f8*g9_19 + f9*g8_19;
    int64_t h8 = f0*g8 + f1*g7    + f2*g6    + f3*g5    + f4*g4    + f5*g3    + f6*g2    + f7*g1    + f8*g0    + f9*g9_19;
    int64_t h9 = f0*g9 + f1*g8    + f2*g7    + f3*g6    + f4*g5    + f5*g4    + f6*g3    + f7*g2    + f8*g1    + f9*g0;

    int64_t carry0, carry1, carry2, carry3, carry4, carry5, carry6, carry7, carry8, carry9;
    carry0 = (h0 + (1LL << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    carry1 = (h1 + (1LL << 24)) >> 25; h2 += carry1; h1 -= carry1 << 25;
    carry2 = (h2 + (1LL << 25)) >> 26; h3 += carry2; h2 -= carry2 << 26;
    carry3 = (h3 + (1LL << 24)) >> 25; h4 += carry3; h3 -= carry3 << 25;
    carry4 = (h4 + (1LL << 25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
    carry5 = (h5 + (1LL << 24)) >> 25; h6 += carry5; h5 -= carry5 << 25;
    carry6 = (h6 + (1LL << 25)) >> 26; h7 += carry6; h6 -= carry6 << 26;
    carry7 = (h7 + (1LL << 24)) >> 25; h8 += carry7; h7 -= carry7 << 25;
    carry8 = (h8 + (1LL << 25)) >> 26; h9 += carry8; h8 -= carry8 << 26;
    carry9 = (h9 + (1LL << 24)) >> 25; h0 += carry9 * 19; h9 -= carry9 << 25;
    carry0 = (h0 + (1LL << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    h[0]=(int)h0; h[1]=(int)h1; h[2]=(int)h2; h[3]=(int)h3; h[4]=(int)h4;
    h[5]=(int)h5; h[6]=(int)h6; h[7]=(int)h7; h[8]=(int)h8; h[9]=(int)h9;
}
