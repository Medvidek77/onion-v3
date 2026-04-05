#include <stdio.h>
#include <stdint.h>

int main() {
    uint8_t final_pubkey[32] = {0x60, 0xEA, 0xA1};

    // MSB-first RFC 4648 Base32 extract
    uint8_t fast_prefix[16];
    int bit_offset = 0;
    for (int i = 0; i < 5; ++i) {
        int bidx  = bit_offset / 8;
        int bsft  = bit_offset % 8;

        uint32_t val = (uint32_t)(final_pubkey[bidx]) << 8;
        if (bidx + 1 < 32) val |= (uint32_t)(final_pubkey[bidx + 1]);

        uint32_t shift = 11u - (uint32_t)(bsft);

        const char* b32 = "abcdefghijklmnopqrstuvwxyz234567";
        fast_prefix[i] = b32[(val >> shift) & 31u];
        bit_offset += 5;
    }
    fast_prefix[5] = 0;
    printf("prefix: %s\n", fast_prefix);
    return 0;
}
