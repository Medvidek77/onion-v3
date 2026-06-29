import re

with open('shader.comp', 'r') as f:
    code = f.read()

# Change Pattern struct from uint8_t to uint to fix GLSL struct padding/alignment rules.
# GLSL std430 aligns arrays differently. Using full uint for arrays is safer.
code = re.sub(
r'''struct Pattern \{
    uint valid_bytes;
    uint8_t byte_target\[16\];
    uint8_t byte_mask\[16\];
\};''',
r'''struct Pattern {
    uint valid_bytes;
    uint byte_target[16];
    uint byte_mask[16];
};''', code)

with open('shader.comp', 'w') as f:
    f.write(code)

with open('main.c', 'r') as f:
    code = f.read()

# Change struct on C side
code = re.sub(
r'''    typedef struct \{
        uint32_t valid_bytes;
        uint8_t  byte_target\[16\];
        uint8_t  byte_mask\[16\];
    \} Pattern;''',
r'''    typedef struct {
        uint32_t valid_bytes;
        uint32_t byte_target[16];
        uint32_t byte_mask[16];
    } Pattern;''', code)


# Restore CLI args order so output_dir is LAST
code = re.sub(
r'''    if \(argc - arg_idx < 2\) \{
        printf\("Usage: %s \[-s\] <output_dir> <prefix1> \[prefix2\] \.\.\.\\n", argv\[0\]\);
        printf\("  -s : Show hashing statistics \(H/s\)\\n"\);
        return 1;
    \}

    const char\* out_dir = argv\[arg_idx\];
    int num_prefixes = argc - arg_idx - 1;
    char\*\* prefixes = &argv\[arg_idx \+ 1\];''',
r'''    if (argc - arg_idx < 2) {
        printf("Usage: %s [-s] <prefix1> [prefix2] ... <output_dir>\n", argv[0]);
        printf("  -s : Show hashing statistics (H/s)\n");
        return 1;
    }

    const char* out_dir = argv[argc - 1];
    int num_prefixes = argc - arg_idx - 1;
    char** prefixes = &argv[arg_idx];''', code)

with open('main.c', 'w') as f:
    f.write(code)

print("Applied fix.")
