import re

with open('shader.comp', 'r') as f:
    code = f.read()

# Change result_index to an array with a counter
code = code.replace("layout(std430,set=0,binding=2) buffer ResultBuffer                   { int result_index; };",
                    "layout(std430,set=0,binding=2) buffer ResultBuffer { uint result_count; int result_indices[]; };")

# Change atomicCompSwap to atomicAdd
code = re.sub(
r'''        if \(check_prefix\(final_Y\)\) \{
            atomicCompSwap\(result_index, -1, int\(id\)\);
        \}''',
r'''        if (check_prefix(final_Y)) {
            uint slot = atomicAdd(result_count, 1u);
            if (slot < 255u) {
                result_indices[slot] = int(id);
            }
        }''', code)

with open('shader.comp', 'w') as f:
    f.write(code)


with open('main.c', 'r') as f:
    code = f.read()

# Change result_size to hold a counter + array
code = code.replace("size_t result_size = sizeof(int);", "size_t result_size = sizeof(uint32_t) + 256 * sizeof(int);")

# Change initialization to clear the counter to 0 instead of setting result_index to -1
code = code.replace("int init_res = -1;", "uint32_t init_res = 0;")
code = code.replace("memcpy(mappedResult[0], &init_res, sizeof(int));", "memcpy(mappedResult[0], &init_res, sizeof(uint32_t));")

code = code.replace("int init_res2 = -1;", "uint32_t init_res2 = 0;")
code = code.replace("memcpy(mappedResult[next_frame], &init_res2, sizeof(int));", "memcpy(mappedResult[next_frame], &init_res2, sizeof(uint32_t));")

code = code.replace("int clr_res = -1;", "uint32_t clr_res = 0;")
code = code.replace("memcpy(mappedResult[cur_frame], &clr_res, sizeof(int));", "memcpy(mappedResult[cur_frame], &clr_res, sizeof(uint32_t));")

# Replace processing logic to loop over multiple results
old_processing = r'''        int result_index;
        memcpy\(&result_index, mappedResult\[cur_frame\], sizeof\(int\)\);

        total_checked \+= BATCH_SIZE;

        if \(print_stats\) \{
            double current_time = get_time_sec\(\);
            if \(current_time - last_print_time >= 1\.0\) \{
                double hps = total_checked / \(current_time - start_time\);
                printf\("\\rChecked %lu keys \| %\.2f H/s \| Found: %u    ", total_checked, hps, found_count\);
                fflush\(stdout\);
                last_print_time = current_time;
            \}
        \}

        if \(result_index != -1\) \{
            found_count\+\+;
            unsigned char\* h = h_scalars\[cur_frame\];
            /\* To get the actual secret, we take our base scalar `h` \*/
            /\* and add `result_index \* 8` to it\. \*/
            /\* We use sc_muladd to safely add in the scalar field\. \*/
            unsigned char offset_scalar\[32\] = \{0\};
            uint32_t offset_val = result_index \* 8;
            offset_scalar\[0\] = \(offset_val >> 0\) & 0xff;
            offset_scalar\[1\] = \(offset_val >> 8\) & 0xff;
            offset_scalar\[2\] = \(offset_val >> 16\) & 0xff;
            offset_scalar\[3\] = \(offset_val >> 24\) & 0xff;

            unsigned char final_scalar\[32\];
            /\* In orlp/ed25519 \(ref10\), sc_muladd\(s, a, b, c\) calculates s = \(a \* b \+ c\) mod L \*/
            /\* We want h \+ offset_scalar\. So a = offset_scalar, b = 1, c = h \*/
            const unsigned char one\[32\] = \{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\};
            sc_muladd\(final_scalar, offset_scalar, one, h\);

            /\* final_scalar is our valid Ed25519 private scalar\. \*/
            /\* We can now derive the full public key to double check \*/
            unsigned char match_pubkey\[32\];
            ge_p3 final_p3;
            ge_scalarmult_base\(&final_p3, final_scalar\);
            ge_p3_tobytes\(match_pubkey, &final_p3\);

            /\* Tor v3 encoding \*/
            char pubkey_b32\[64\];
            tor_base32_encode\(pubkey_b32, match_pubkey, 32\);

            char path\[256\];
            if \(snprintf\(path, sizeof\(path\), "%s/keys_%u", out_dir, \(uint32_t\)\(total_checked/BATCH_SIZE\)\) >= \(int\)sizeof\(path\)\) \{
                printf\("Path too long\\n"\);
            \} else \{
                FILE \*f = fopen\(path, "wb"\);
                if \(!f\) \{
                    printf\("Failed to save key to %s\\n", path\);
                \} else \{
                    /\* Tor secret keys are 64 bytes: 32 byte scalar \+ 32 byte pubkey \*/
                    unsigned char expanded_sk\[64\];
                    memcpy\(expanded_sk, final_scalar, 32\);
                    memcpy\(expanded_sk \+ 32, match_pubkey, 32\);
                    fwrite\(expanded_sk, 1, 64, f\);
                    fclose\(f\);

                    if \(!print_stats\) \{
                        printf\("%s\.onion\\n", pubkey_b32\);
                        fflush\(stdout\);
                    \}
                \}
            \}

            /\* DO NOT exit\. Just clear the result index and keep searching\. \*/
            uint32_t clr_res = 0;
            memcpy\(mappedResult\[cur_frame\], &clr_res, sizeof\(uint32_t\)\);
        \}'''

new_processing = r'''        uint32_t result_count;
        memcpy(&result_count, mappedResult[cur_frame], sizeof(uint32_t));

        total_checked += BATCH_SIZE;

        if (print_stats) {
            double current_time = get_time_sec();
            if (current_time - last_print_time >= 1.0) {
                double hps = total_checked / (current_time - start_time);
                printf("\rChecked %lu keys | %.2f H/s | Found: %u    ", total_checked, hps, found_count);
                fflush(stdout);
                last_print_time = current_time;
            }
        }

        if (result_count > 0) {
            uint32_t iter = result_count > 255 ? 255 : result_count;
            int results[256];
            memcpy(results, (uint8_t*)mappedResult[cur_frame] + sizeof(uint32_t), iter * sizeof(int));

            for (uint32_t r = 0; r < iter; r++) {
                int result_index = results[r];
                found_count++;
                unsigned char* h = h_scalars[cur_frame];
                unsigned char offset_scalar[32] = {0};
                uint32_t offset_val = result_index * 8;
                offset_scalar[0] = (offset_val >> 0) & 0xff;
                offset_scalar[1] = (offset_val >> 8) & 0xff;
                offset_scalar[2] = (offset_val >> 16) & 0xff;
                offset_scalar[3] = (offset_val >> 24) & 0xff;

                unsigned char final_scalar[32];
                const unsigned char one[32] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
                sc_muladd(final_scalar, offset_scalar, one, h);

                unsigned char match_pubkey[32];
                ge_p3 final_p3;
                ge_scalarmult_base(&final_p3, final_scalar);
                ge_p3_tobytes(match_pubkey, &final_p3);

                char pubkey_b32[64];
                tor_base32_encode(pubkey_b32, match_pubkey, 32);

                char path[256];
                if (snprintf(path, sizeof(path), "%s/keys_%u_%u", out_dir, (uint32_t)(total_checked/BATCH_SIZE), r) >= (int)sizeof(path)) {
                    printf("Path too long\n");
                } else {
                    FILE *f = fopen(path, "wb");
                    if (!f) {
                        printf("Failed to save key to %s\n", path);
                    } else {
                        unsigned char expanded_sk[64];
                        memcpy(expanded_sk, final_scalar, 32);
                        memcpy(expanded_sk + 32, match_pubkey, 32);
                        fwrite(expanded_sk, 1, 64, f);
                        fclose(f);

                        if (!print_stats) {
                            printf("%s.onion\n", pubkey_b32);
                            fflush(stdout);
                        }
                    }
                }
            }

            uint32_t clr_res = 0;
            memcpy(mappedResult[cur_frame], &clr_res, sizeof(uint32_t));
        }'''

code = re.sub(old_processing, new_processing, code)

with open('main.c', 'w') as f:
    f.write(code)

print("Replaced result arrays.")
