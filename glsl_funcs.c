void fe_sq_glsl(int h[10], const int f[10]) {
    int64_t f0 = f[0], f1 = f[1], f2 = f[2], f3 = f[3], f4 = f[4];
    int64_t f5 = f[5], f6 = f[6], f7 = f[7], f8 = f[8], f9 = f[9];
    int64_t f0_2 = 2L*f0, f1_2 = 2L*f1, f2_2 = 2L*f2, f3_2 = 2L*f3, f4_2 = 2L*f4;
    int64_t f5_2 = 2L*f5, f6_2 = 2L*f6, f7_2 = 2L*f7, f8_2 = 2L*f8;
    int64_t f5_38 = 38L*f5, f6_19 = 19L*f6, f7_19 = 19L*f7, f7_38 = 38L*f7, f8_19 = 19L*f8, f9_19 = 19L*f9, f9_38 = 38L*f9;
    int64_t f0f0    = f0 * f0;
    int64_t f0f1_2  = f0_2 * f1;
    int64_t f0f2_2  = f0_2 * f2;
    int64_t f0f3_2  = f0_2 * f3;
    int64_t f0f4_2  = f0_2 * f4;
    int64_t f0f5_2  = f0_2 * f5;
    int64_t f0f6_2  = f0_2 * f6;
    int64_t f0f7_2  = f0_2 * f7;
    int64_t f0f8_2  = f0_2 * f8;
    int64_t f0f9_2  = f0_2 * f9;
    int64_t f1f1_2  = f1_2 * f1;
    int64_t f1f2_2  = f1_2 * f2;
    int64_t f1f3_4  = f1_2 * f3_2;
    int64_t f1f4_2  = f1_2 * f4;
    int64_t f1f5_4  = f1_2 * f5_2;
    int64_t f1f6_2  = f1_2 * f6;
    int64_t f1f7_4  = f1_2 * f7_2;
    int64_t f1f8_2  = f1_2 * f8;
    int64_t f1f9_76 = f1_2 * f9_38;
    int64_t f2f2    = f2   * f2;
    int64_t f2f3_2  = f2_2 * f3;
    int64_t f2f4_2  = f2_2 * f4;
    int64_t f2f5_2  = f2_2 * f5;
    int64_t f2f6_2  = f2_2 * f6;
    int64_t f2f7_2  = f2_2 * f7;
    int64_t f2f8_38 = f2_2 * f8_19;
    int64_t f2f9_38 = f2   * f9_38;
    int64_t f3f3_2  = f3_2 * f3;
    int64_t f3f4_2  = f3_2 * f4;
    int64_t f3f5_4  = f3_2 * f5_2;
    int64_t f3f6_2  = f3_2 * f6;
    int64_t f3f7_76 = f3_2 * f7_38;
    int64_t f3f8_38 = f3_2 * f8_19;
    int64_t f3f9_76 = f3_2 * f9_38;
    int64_t f4f4    = f4   * f4;
    int64_t f4f5_2  = f4_2 * f5;
    int64_t f4f6_38 = f4_2 * f6_19;
    int64_t f4f7_38 = f4   * f7_38;
    int64_t f4f8_38 = f4_2 * f8_19;
    int64_t f4f9_38 = f4   * f9_38;
    int64_t f5f5_38 = f5   * f5_38;
    int64_t f5f6_38 = f5_2 * f6_19;
    int64_t f5f7_76 = f5_2 * f7_38;
    int64_t f5f8_38 = f5_2 * f8_19;
    int64_t f5f9_76 = f5_2 * f9_38;
    int64_t f6f6_19 = f6   * f6_19;
    int64_t f6f7_38 = f6_2 * f7_19;
    int64_t f6f8_19 = f6   * f8_19;
    int64_t f6f9_38 = f6_2 * f9_19;
    int64_t f7f7_38 = f7   * f7_38;
    int64_t f7f8_38 = f7_2 * f8_19;
    int64_t f7f9_76 = f7_2 * f9_38;
    int64_t f8f8_19 = f8   * f8_19;
    int64_t f8f9_38 = f8_2 * f9_19;
    int64_t f9f9_38 = f9   * f9_38;
    int64_t h0 = f0f0  + f1f9_76 + f2f8_38 + f3f7_76 + f4f6_38 + f5f5_38;
    int64_t h1 = f0f1_2 + f2f9_38 + f3f8_38 + f4f7_38 + f5f6_38;
    int64_t h2 = f0f2_2 + f1f1_2 + f3f9_76 + f4f8_38 + f5f7_76 + f6f6_19;
    int64_t h3 = f0f3_2 + f1f2_2 + f4f9_38 + f5f8_38 + f6f7_38;
    int64_t h4 = f0f4_2 + f1f3_4 + f2f2   + f5f9_76 + f6f8_19 + f7f7_38;
    int64_t h5 = f0f5_2 + f1f4_2 + f2f3_2 + f6f9_38 + f7f8_38;
    int64_t h6 = f0f6_2 + f1f5_4 + f2f4_2 + f3f3_2 + f7f9_76 + f8f8_19;
    int64_t h7 = f0f7_2 + f1f6_2 + f2f5_2 + f3f4_2 + f8f9_38;
    int64_t h8 = f0f8_2 + f1f7_4 + f2f6_2 + f3f5_4 + f4f4   + f9f9_38;
    int64_t h9 = f0f9_2 + f1f8_2 + f2f7_2 + f3f6_2 + f4f5_2;
    int64_t carry0, carry1, carry2, carry3, carry4, carry5, carry6, carry7, carry8, carry9;
    carry0 = (h0 + (1LL << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    carry4 = (h4 + (1LL << 25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
    carry1 = (h1 + (1LL << 24)) >> 25; h2 += carry1; h1 -= carry1 << 25;
    carry5 = (h5 + (1LL << 24)) >> 25; h6 += carry5; h5 -= carry5 << 25;
    carry2 = (h2 + (1LL << 25)) >> 26; h3 += carry2; h2 -= carry2 << 26;
    carry6 = (h6 + (1LL << 25)) >> 26; h7 += carry6; h6 -= carry6 << 26;
    carry3 = (h3 + (1LL << 24)) >> 25; h4 += carry3; h3 -= carry3 << 25;
    carry7 = (h7 + (1LL << 24)) >> 25; h8 += carry7; h7 -= carry7 << 25;
    carry4 = (h4 + (1LL << 25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
    carry8 = (h8 + (1LL << 25)) >> 26; h9 += carry8; h8 -= carry8 << 26;
    carry9 = (h9 + (1LL << 24)) >> 25; h0 += carry9 * 19; h9 -= carry9 << 25;
    carry0 = (h0 + (1LL << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    h[0]=(int)h0; h[1]=(int)h1; h[2]=(int)h2; h[3]=(int)h3; h[4]=(int)h4;
    h[5]=(int)h5; h[6]=(int)h6; h[7]=(int)h7; h[8]=(int)h8; h[9]=(int)h9;
}

void fe_invert_glsl(int out_fe[10], const int z[10]) {
    int t0[10], t1[10], t2[10], t3[10];
    fe_sq_glsl(t0, z);
    fe_sq_glsl(t1, t0); for (int i = 1; i < 2; ++i) { fe_sq_glsl(t1, t1); }
    fe_mul_glsl(t1, z, t1);
    fe_mul_glsl(t0, t0, t1);
    fe_sq_glsl(t2, t0); for (int i = 1; i < 1; ++i) { fe_sq_glsl(t2, t2); }
    fe_mul_glsl(t1, t1, t2);
    fe_sq_glsl(t2, t1); for (int i = 1; i < 5; ++i) { fe_sq_glsl(t2, t2); }
    fe_mul_glsl(t1, t2, t1);
    fe_sq_glsl(t2, t1); for (int i = 1; i < 10; ++i) { fe_sq_glsl(t2, t2); }
    fe_mul_glsl(t2, t2, t1);
    fe_sq_glsl(t3, t2); for (int i = 1; i < 20; ++i) { fe_sq_glsl(t3, t3); }
    fe_mul_glsl(t2, t3, t2);
    fe_sq_glsl(t3, t2); for (int i = 1; i < 10; ++i) { fe_sq_glsl(t3, t3); }
    fe_mul_glsl(t1, t3, t1);
    fe_sq_glsl(t2, t1); for (int i = 1; i < 50; ++i) { fe_sq_glsl(t2, t2); }
    fe_mul_glsl(t2, t2, t1);
    fe_sq_glsl(t3, t2); for (int i = 1; i < 100; ++i) { fe_sq_glsl(t3, t3); }
    fe_mul_glsl(t2, t3, t2);
    fe_sq_glsl(t3, t2); for (int i = 1; i < 50; ++i) { fe_sq_glsl(t3, t3); }
    fe_mul_glsl(t1, t3, t1);
    fe_sq_glsl(t1, t1); for (int i = 1; i < 5; ++i) { fe_sq_glsl(t1, t1); }
    fe_mul_glsl(out_fe, t1, t0);
}

void ge_madd_precomp_glsl(int r_X[10], int r_Y[10], int r_Z[10], int r_T[10],
                          const int p_YplusX[10], const int p_YminusX[10], const int p_Z2[10], const int p_T[10],
                          const int q_YplusX[10], const int q_YminusX[10], const int q_Z[10], const int q_T2d[10])
{
    int A[10], B[10], C[10], D[10];
    fe_mul_glsl(C, p_YplusX, q_YplusX);
    fe_mul_glsl(B, p_YminusX, q_YminusX);
    fe_mul_glsl(D, q_T2d, p_T);
    int t1[10];
    fe_mul_glsl(t1, p_Z2, q_Z);
    fe_sub_glsl(A, C, B);
    fe_add_glsl(B, C, B);
    fe_add_glsl(C, t1, D);
    fe_sub_glsl(D, t1, D);
    fe_mul_glsl(r_X, A, D);
    fe_mul_glsl(r_Y, B, C);
    fe_mul_glsl(r_Z, C, D);
    fe_mul_glsl(r_T, A, B);
}

void ge_p3_tobytes_glsl(uint8_t s[32], const int X[10], const int Y[10], const int Z[10]) {
    int recip[10];
    fe_invert_glsl(recip, Z);
    int x[10], y[10];
    fe_mul_glsl(x, X, recip);
    fe_mul_glsl(y, Y, recip);

    int h0=y[0], h1=y[1], h2=y[2], h3=y[3], h4=y[4];
    int h5=y[5], h6=y[6], h7=y[7], h8=y[8], h9=y[9];

    int carry0 = (h0 + (1 << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    int carry1 = (h1 + (1 << 24)) >> 25; h2 += carry1; h1 -= carry1 << 25;
    int carry2 = (h2 + (1 << 25)) >> 26; h3 += carry2; h2 -= carry2 << 26;
    int carry3 = (h3 + (1 << 24)) >> 25; h4 += carry3; h3 -= carry3 << 25;
    int carry4 = (h4 + (1 << 25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
    int carry5 = (h5 + (1 << 24)) >> 25; h6 += carry5; h5 -= carry5 << 25;
    int carry6 = (h6 + (1 << 25)) >> 26; h7 += carry6; h6 -= carry6 << 26;
    int carry7 = (h7 + (1 << 24)) >> 25; h8 += carry7; h7 -= carry7 << 25;
    int carry8 = (h8 + (1 << 25)) >> 26; h9 += carry8; h8 -= carry8 << 26;
    int carry9 = (h9 + (1 << 24)) >> 25; h0 += carry9 * 19; h9 -= carry9 << 25;
    carry0 = (h0 + (1 << 25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
    carry1 = (h1 + (1 << 24)) >> 25; h2 += carry1; h1 -= carry1 << 25;
    carry2 = (h2 + (1 << 25)) >> 26; h3 += carry2; h2 -= carry2 << 26;
    carry3 = (h3 + (1 << 24)) >> 25; h4 += carry3; h3 -= carry3 << 25;
    carry4 = (h4 + (1 << 25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
    carry5 = (h5 + (1 << 24)) >> 25; h6 += carry5; h5 -= carry5 << 25;
    carry6 = (h6 + (1 << 25)) >> 26; h7 += carry6; h6 -= carry6 << 26;
    carry7 = (h7 + (1 << 24)) >> 25; h8 += carry7; h7 -= carry7 << 25;
    carry8 = (h8 + (1 << 25)) >> 26; h9 += carry8; h8 -= carry8 << 26;

    int q0 = h0 + 19;
    int q1 = h1 + (q0 >> 26); q0 &= 0x3ffffff;
    int q2 = h2 + (q1 >> 25); q1 &= 0x1ffffff;
    int q3 = h3 + (q2 >> 26); q2 &= 0x3ffffff;
    int q4 = h4 + (q3 >> 25); q3 &= 0x1ffffff;
    int q5 = h5 + (q4 >> 26); q4 &= 0x3ffffff;
    int q6 = h6 + (q5 >> 25); q5 &= 0x1ffffff;
    int q7 = h7 + (q6 >> 26); q6 &= 0x3ffffff;
    int q8 = h8 + (q7 >> 25); q7 &= 0x1ffffff;
    int q9 = h9 + (q8 >> 26); q8 &= 0x3ffffff;

    q9 = (q9 >> 25) & 1;
    h0 += 19 * q9;
    h1 += h0 >> 26; h0 &= 0x3ffffff;
    h2 += h1 >> 25; h1 &= 0x1ffffff;
    h3 += h2 >> 26; h2 &= 0x3ffffff;
    h4 += h3 >> 25; h3 &= 0x1ffffff;
    h5 += h4 >> 26; h4 &= 0x3ffffff;
    h6 += h5 >> 25; h5 &= 0x1ffffff;
    h7 += h6 >> 26; h6 &= 0x3ffffff;
    h8 += h7 >> 25; h7 &= 0x1ffffff;
    h9 += h8 >> 26; h8 &= 0x3ffffff;

    s[0] = (uint8_t)(h0 >> 0); s[1] = (uint8_t)(h0 >> 8); s[2] = (uint8_t)(h0 >> 16); s[3] = (uint8_t)((h0 >> 24) | (h1 << 2));
    s[4] = (uint8_t)(h1 >> 6); s[5] = (uint8_t)(h1 >> 14); s[6] = (uint8_t)((h1 >> 22) | (h2 << 3)); s[7] = (uint8_t)(h2 >> 5);
    s[8] = (uint8_t)(h2 >> 13); s[9] = (uint8_t)((h2 >> 21) | (h3 << 5)); s[10] = (uint8_t)(h3 >> 3); s[11] = (uint8_t)(h3 >> 11);
    s[12] = (uint8_t)((h3 >> 19) | (h4 << 6)); s[13] = (uint8_t)(h4 >> 2); s[14] = (uint8_t)(h4 >> 10); s[15] = (uint8_t)(h4 >> 18);
    s[16] = (uint8_t)(h5 >> 0); s[17] = (uint8_t)(h5 >> 8); s[18] = (uint8_t)(h5 >> 16); s[19] = (uint8_t)((h5 >> 24) | (h6 << 2));
    s[20] = (uint8_t)(h6 >> 6); s[21] = (uint8_t)(h6 >> 14); s[22] = (uint8_t)((h6 >> 22) | (h7 << 3)); s[23] = (uint8_t)(h7 >> 5);
    s[24] = (uint8_t)(h7 >> 13); s[25] = (uint8_t)((h7 >> 21) | (h8 << 5)); s[26] = (uint8_t)(h8 >> 3); s[27] = (uint8_t)(h8 >> 11);
    s[28] = (uint8_t)((h8 >> 19) | (h9 << 6)); s[29] = (uint8_t)(h9 >> 2); s[30] = (uint8_t)(h9 >> 10); s[31] = (uint8_t)(h9 >> 18);

    // Sign bit from x
    int x0 = x[0], x1 = x[1], x2 = x[2], x3 = x[3], x4 = x[4];
    int x5 = x[5], x6 = x[6], x7 = x[7], x8 = x[8], x9 = x[9];
    carry0 = (x0 + (1 << 25)) >> 26; x1 += carry0; x0 -= carry0 << 26;
    carry1 = (x1 + (1 << 24)) >> 25; x2 += carry1; x1 -= carry1 << 25;
