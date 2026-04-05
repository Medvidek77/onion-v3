#include <stdio.h>
#include <stdint.h>
#include "fe.h"
#include "ge.h"

int main() {
    ge_p3 p;
    ge_scalarmult_base(&p, (const unsigned char*)"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

    unsigned char bytes[32];
    ge_p3_tobytes(bytes, &p);

    int recip[10];
    fe_invert(recip, p.Z);
    int x[10], y[10];
    fe_mul(x, p.X, recip);
    fe_mul(y, p.Y, recip);

    unsigned char glsl_bytes[32];
    fe_tobytes(glsl_bytes, y);

    int x0 = x[0], x1 = x[1], x2 = x[2], x3 = x[3], x4 = x[4];
    int x5 = x[5], x6 = x[6], x7 = x[7], x8 = x[8], x9 = x[9];
    int carry0 = (x0 + (1 << 25)) >> 26; x1 += carry0; x0 -= carry0 << 26;
    int carry1 = (x1 + (1 << 24)) >> 25; x2 += carry1; x1 -= carry1 << 25;
    int carry2 = (x2 + (1 << 25)) >> 26; x3 += carry2; x2 -= carry2 << 26;
    int carry3 = (x3 + (1 << 24)) >> 25; x4 += carry3; x3 -= carry3 << 25;
    int carry4 = (x4 + (1 << 25)) >> 26; x5 += carry4; x4 -= carry4 << 26;
    int carry5 = (x5 + (1 << 24)) >> 25; x6 += carry5; x5 -= carry5 << 25;
    int carry6 = (x6 + (1 << 25)) >> 26; x7 += carry6; x6 -= carry6 << 26;
    int carry7 = (x7 + (1 << 24)) >> 25; x8 += carry7; x7 -= carry7 << 25;
    int carry8 = (x8 + (1 << 25)) >> 26; x9 += carry8; x8 -= carry8 << 26;
    int carry9 = (x9 + (1 << 24)) >> 25; x0 += carry9 * 19; x9 -= carry9 << 25;
    carry0 = (x0 + (1 << 25)) >> 26; x1 += carry0; x0 -= carry0 << 26;

    int q0 = x0 + 19;
    int q1 = x1 + (q0 >> 26); q0 &= 0x3ffffff;
    int q2 = x2 + (q1 >> 25); q1 &= 0x1ffffff;
    int q3 = x3 + (q2 >> 26); q2 &= 0x3ffffff;
    int q4 = x4 + (q3 >> 25); q3 &= 0x1ffffff;
    int q5 = x5 + (q4 >> 26); q4 &= 0x3ffffff;
    int q6 = x6 + (q5 >> 25); q5 &= 0x1ffffff;
    int q7 = x7 + (q6 >> 26); q6 &= 0x3ffffff;
    int q8 = x8 + (q7 >> 25); q7 &= 0x1ffffff;
    int q9 = x9 + (q8 >> 26); q8 &= 0x3ffffff;
    q9 = (q9 >> 25) & 1;
    x0 += 19 * q9;
    x1 += x0 >> 26; x0 &= 0x3ffffff;

    glsl_bytes[31] ^= (uint8_t)((x0 & 1) << 7);

    int ok = 1;
    for(int i=0; i<32; i++) if(bytes[i] != glsl_bytes[i]) ok = 0;
    printf("ge_p3_tobytes match? %d\n", ok);
    return 0;
}
