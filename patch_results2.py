import re

with open('main.c', 'r') as f:
    code = f.read()

# Let's fix the directory creation and loop in main.c

old_code = r'''            char path[512];
            if (snprintf(path, sizeof(path), "%s/keys_%u_%u", out_dir, (uint32_t)(total_checked/BATCH_SIZE), r) >= (int)sizeof(path)) {
                continue;
            }
            mkdir(path, 0700);

            char key_path[550];
            if (snprintf(key_path, sizeof(key_path), "%s/hs_ed25519_secret_key", path) >= (int)sizeof(key_path)) {
                continue;
            }

            for (uint32_t r = 0; r < iter; r++) {
                int result_index = results[r];'''

new_code = r'''            for (uint32_t r = 0; r < iter; r++) {
                char path[512];
                if (snprintf(path, sizeof(path), "%s/keys_%u_%u", out_dir, (uint32_t)(total_checked/BATCH_SIZE), r) >= (int)sizeof(path)) {
                    continue;
                }
                mkdir(path, 0700);

                char key_path[550];
                if (snprintf(key_path, sizeof(key_path), "%s/hs_ed25519_secret_key", path) >= (int)sizeof(key_path)) {
                    continue;
                }

                int result_index = results[r];'''

# Note: The code doesn't exactly match `old_code`. I need to inspect how it's actually structured.
