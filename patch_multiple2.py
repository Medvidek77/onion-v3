import re

with open('main.c', 'r') as f:
    code = f.read()

# Replace the PushConstants block setting logic with multiple prefixes array logic

# Find the prefix parsing block to replace it
code = re.sub(
r'''    /\* Prefix Bitmask \(No GPU buffer needed, using push constants\) \*/
    uint8_t byte_target\[16\] = \{0\};
    uint8_t byte_mask\[16\] = \{0\};
    uint32_t total_bits = prefix_len \* 5;
    uint32_t full_bytes = total_bits / 8;
    uint32_t remainder_bits = total_bits % 8;

    uint8_t bits\[256\] = \{0\};
    for \(size_t i = 0; i < prefix_len; i\+\+\) \{
        int val = 0;
        char c = prefix\[i\];
        if \(c >= 'a' && c <= 'z'\) val = c - 'a';
        else if \(c >= '2' && c <= '7'\) val = c - '2' \+ 26;
        for \(int b = 4; b >= 0; b--\) \{
            bits\[i \* 5 \+ \(4 - b\)\] = \(val >> b\) & 1;
        \}
    \}

    uint32_t valid_bytes = full_bytes;
    for \(uint32_t i = 0; i < full_bytes; i\+\+\) \{
        for \(int b = 0; b < 8; b\+\+\) \{
            byte_target\[i\] \|= bits\[i \* 8 \+ b\] << \(7 - b\);
        \}
        byte_mask\[i\] = 0xFF;
    \}

    if \(remainder_bits > 0\) \{
        uint8_t partial_target = 0;
        uint8_t partial_mask = 0;
        for \(uint32_t b = 0; b < remainder_bits; b\+\+\) \{
            partial_target \|= bits\[full_bytes \* 8 \+ b\] << \(7 - b\);
            partial_mask \|= 1 << \(7 - b\);
        \}
        byte_target\[full_bytes\] = partial_target;
        byte_mask\[full_bytes\] = partial_mask;
        valid_bytes\+\+;
    \}''',
r'''    /* Prefix Bitmask handling for multiple prefixes */
    #define MAX_PREFIXES 256
    if (num_prefixes > MAX_PREFIXES) {
        printf("Too many prefixes (max %d)\n", MAX_PREFIXES);
        return 1;
    }

    typedef struct {
        uint32_t valid_bytes;
        uint8_t  byte_target[16];
        uint8_t  byte_mask[16];
    } Pattern;
    Pattern patterns[MAX_PREFIXES] = {0};

    for (int p = 0; p < num_prefixes; p++) {
        size_t plen = strlen(prefixes[p]);
        uint32_t total_bits = plen * 5;
        uint32_t full_bytes = total_bits / 8;
        uint32_t remainder_bits = total_bits % 8;
        uint8_t bits[256] = {0};
        uint8_t bit_mask[256] = {0};

        for (size_t i = 0; i < plen; i++) {
            int val = 0;
            char c = prefixes[p][i];
            if (c == '?') {
                for (int b = 4; b >= 0; b--) {
                    bits[i * 5 + (4 - b)] = 0;
                    bit_mask[i * 5 + (4 - b)] = 0;
                }
            } else {
                if (c >= 'a' && c <= 'z') val = c - 'a';
                else if (c >= '2' && c <= '7') val = c - '2' + 26;
                else {
                    printf("Invalid character in prefix: %c\n", c);
                    return 1;
                }
                for (int b = 4; b >= 0; b--) {
                    bits[i * 5 + (4 - b)] = (val >> b) & 1;
                    bit_mask[i * 5 + (4 - b)] = 1;
                }
            }
        }

        uint32_t v_bytes = full_bytes;
        for (uint32_t i = 0; i < full_bytes; i++) {
            for (int b = 0; b < 8; b++) {
                patterns[p].byte_target[i] |= bits[i * 8 + b] << (7 - b);
                patterns[p].byte_mask[i] |= bit_mask[i * 8 + b] << (7 - b);
            }
        }

        if (remainder_bits > 0) {
            uint8_t partial_target = 0;
            uint8_t partial_mask = 0;
            for (uint32_t b = 0; b < remainder_bits; b++) {
                partial_target |= bits[full_bytes * 8 + b] << (7 - b);
                partial_mask |= bit_mask[full_bytes * 8 + b] << (7 - b);
            }
            patterns[p].byte_target[full_bytes] = partial_target;
            patterns[p].byte_mask[full_bytes] = partial_mask;
            v_bytes++;
        }
        patterns[p].valid_bytes = v_bytes;
    }''', code)

with open('main.c', 'w') as f:
    f.write(code)

print("Replaced Prefix parsing block.")
