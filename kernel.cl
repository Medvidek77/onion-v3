// OpenCL Kernel for Ed25519 point addition and Tor v3 onion address prefix matching.

typedef ulong uint64_t;
typedef uint uint32_t;
typedef uchar uint8_t;

// Keccak-f[1600] permutations
__constant uint64_t keccakf_rndc[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

#define ROL64(a, offset) ((offset != 0) ? ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64 - offset))) : a)

inline void keccakf(uint64_t st[25]) {
    int round;
    uint64_t t, bc[5];
    for (round = 0; round < 24; ++round) {
        // Theta
        bc[0] = st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20];
        bc[1] = st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21];
        bc[2] = st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22];
        bc[3] = st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23];
        bc[4] = st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24];

        t = bc[4] ^ ROL64(bc[1], 1); st[0] ^= t; st[5] ^= t; st[10] ^= t; st[15] ^= t; st[20] ^= t;
        t = bc[0] ^ ROL64(bc[2], 1); st[1] ^= t; st[6] ^= t; st[11] ^= t; st[16] ^= t; st[21] ^= t;
        t = bc[1] ^ ROL64(bc[3], 1); st[2] ^= t; st[7] ^= t; st[12] ^= t; st[17] ^= t; st[22] ^= t;
        t = bc[2] ^ ROL64(bc[4], 1); st[3] ^= t; st[8] ^= t; st[13] ^= t; st[18] ^= t; st[23] ^= t;
        t = bc[3] ^ ROL64(bc[0], 1); st[4] ^= t; st[9] ^= t; st[14] ^= t; st[19] ^= t; st[24] ^= t;

        // Rho Pi
        t = st[1];
        bc[0] = st[10]; st[10] = ROL64(t, 1); t = bc[0];
        bc[0] = st[7]; st[7] = ROL64(t, 3); t = bc[0];
        bc[0] = st[11]; st[11] = ROL64(t, 6); t = bc[0];
        bc[0] = st[17]; st[17] = ROL64(t, 10); t = bc[0];
        bc[0] = st[18]; st[18] = ROL64(t, 15); t = bc[0];
        bc[0] = st[3]; st[3] = ROL64(t, 21); t = bc[0];
        bc[0] = st[5]; st[5] = ROL64(t, 28); t = bc[0];
        bc[0] = st[16]; st[16] = ROL64(t, 36); t = bc[0];
        bc[0] = st[8]; st[8] = ROL64(t, 45); t = bc[0];
        bc[0] = st[21]; st[21] = ROL64(t, 55); t = bc[0];
        bc[0] = st[24]; st[24] = ROL64(t, 2); t = bc[0];
        bc[0] = st[4]; st[4] = ROL64(t, 14); t = bc[0];
        bc[0] = st[15]; st[15] = ROL64(t, 27); t = bc[0];
        bc[0] = st[23]; st[23] = ROL64(t, 41); t = bc[0];
        bc[0] = st[19]; st[19] = ROL64(t, 56); t = bc[0];
        bc[0] = st[13]; st[13] = ROL64(t, 8); t = bc[0];
        bc[0] = st[12]; st[12] = ROL64(t, 25); t = bc[0];
        bc[0] = st[2]; st[2] = ROL64(t, 43); t = bc[0];
        bc[0] = st[20]; st[20] = ROL64(t, 62); t = bc[0];
        bc[0] = st[14]; st[14] = ROL64(t, 18); t = bc[0];
        bc[0] = st[22]; st[22] = ROL64(t, 39); t = bc[0];
        bc[0] = st[9]; st[9] = ROL64(t, 61); t = bc[0];
        bc[0] = st[6]; st[6] = ROL64(t, 20); t = bc[0];
        st[1] = ROL64(t, 44);

        // Chi
        bc[0] = st[0]; bc[1] = st[1]; bc[2] = st[2]; bc[3] = st[3]; bc[4] = st[4];
        st[0] = bc[0] ^ (~bc[1] & bc[2]); st[1] = bc[1] ^ (~bc[2] & bc[3]); st[2] = bc[2] ^ (~bc[3] & bc[4]); st[3] = bc[3] ^ (~bc[4] & bc[0]); st[4] = bc[4] ^ (~bc[0] & bc[1]);
        bc[0] = st[5]; bc[1] = st[6]; bc[2] = st[7]; bc[3] = st[8]; bc[4] = st[9];
        st[5] = bc[0] ^ (~bc[1] & bc[2]); st[6] = bc[1] ^ (~bc[2] & bc[3]); st[7] = bc[2] ^ (~bc[3] & bc[4]); st[8] = bc[3] ^ (~bc[4] & bc[0]); st[9] = bc[4] ^ (~bc[0] & bc[1]);
        bc[0] = st[10]; bc[1] = st[11]; bc[2] = st[12]; bc[3] = st[13]; bc[4] = st[14];
        st[10] = bc[0] ^ (~bc[1] & bc[2]); st[11] = bc[1] ^ (~bc[2] & bc[3]); st[12] = bc[2] ^ (~bc[3] & bc[4]); st[13] = bc[3] ^ (~bc[4] & bc[0]); st[14] = bc[4] ^ (~bc[0] & bc[1]);
        bc[0] = st[15]; bc[1] = st[16]; bc[2] = st[17]; bc[3] = st[18]; bc[4] = st[19];
        st[15] = bc[0] ^ (~bc[1] & bc[2]); st[16] = bc[1] ^ (~bc[2] & bc[3]); st[17] = bc[2] ^ (~bc[3] & bc[4]); st[18] = bc[3] ^ (~bc[4] & bc[0]); st[19] = bc[4] ^ (~bc[0] & bc[1]);
        bc[0] = st[20]; bc[1] = st[21]; bc[2] = st[22]; bc[3] = st[23]; bc[4] = st[24];
        st[20] = bc[0] ^ (~bc[1] & bc[2]); st[21] = bc[1] ^ (~bc[2] & bc[3]); st[22] = bc[2] ^ (~bc[3] & bc[4]); st[23] = bc[3] ^ (~bc[4] & bc[0]); st[24] = bc[4] ^ (~bc[0] & bc[1]);

        // Iota
        st[0] ^= keccakf_rndc[round];
    }
}

__constant char b32_alphabet[] = "abcdefghijklmnopqrstuvwxyz234567";
__constant char onion_prefix[] = ".onion checksum";

// Convert 32-byte public key to first few chars of base32
inline void pubkey_to_base32_prefix(const uint8_t* pubkey, uint8_t* out, int len) {
    uint64_t state[25] = {0};
    uint8_t* state_bytes = (uint8_t*)state;

    // TOR v3 onion address format:
    // checksum = sha3_256(".onion checksum" || pubkey || version)
    // address = base32(pubkey || checksum[0..1] || version)

    // Construct the input for sha3_256
    // ".onion checksum" (15 bytes)
    for(int i=0; i<15; ++i) state_bytes[i] = onion_prefix[i];
    // pubkey (32 bytes)
    for(int i=0; i<32; ++i) state_bytes[15+i] = pubkey[i];
    // version (1 byte, 0x03)
    state_bytes[47] = 0x03;

    // Keccak state padding (SHA3-256 uses 136 bytes rate)
    state_bytes[48] = 0x06;
    state_bytes[135] |= 0x80;

    // Endianness adjustment before keccakf
    // OpenCL usually uses little endian, which is correct for Keccak.
    keccakf(state);

    // The checksum is the first 2 bytes of the state.
    uint8_t checksum[2];
    checksum[0] = state_bytes[0];
    checksum[1] = state_bytes[1];

    // Now encode base32 (pubkey || checksum || version)
    // Only need the first 'len' characters (up to 32 usually)
    uint8_t full_data[35];
    for(int i=0; i<32; ++i) full_data[i] = pubkey[i];
    full_data[32] = checksum[0];
    full_data[33] = checksum[1];
    full_data[34] = 0x03;

    int bit_offset = 0;
    for(int i=0; i<len; ++i) {
        int byte_index = bit_offset / 8;
        int bit_shift = bit_offset % 8;

        uint32_t val = full_data[byte_index];
        if (byte_index + 1 < 35) val |= (full_data[byte_index+1] << 8);

        out[i] = b32_alphabet[(val >> bit_shift) & 31];
        bit_offset += 5;
    }
}

__kernel void vanity_search(
    __global const uint8_t* base_pubkeys,
    uint32_t batch_size,
    __global const uint8_t* target_prefix,
    uint32_t prefix_len,
    __global int* result_index)
{
    uint32_t id = get_global_id(0);
    if (id >= batch_size) return;

    const uint8_t* pubkey = &base_pubkeys[id * 32];

    uint8_t generated_prefix[16]; // Max prefix len to check
    pubkey_to_base32_prefix(pubkey, generated_prefix, prefix_len);

    bool match = true;
    for (uint32_t i = 0; i < prefix_len; ++i) {
        if (generated_prefix[i] != target_prefix[i]) {
            match = false;
            break;
        }
    }

    if (match) {
        atomic_xchg(result_index, id);
    }
}
