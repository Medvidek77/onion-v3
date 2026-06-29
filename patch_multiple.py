import re

with open('main.c', 'r') as f:
    code = f.read()

# Change CLI parsing
code = re.sub(
r'''    if \(argc - arg_idx < 2\) \{
        printf\("Usage: %s \[-s\] <prefix> <output_dir>\\n", argv\[0\]\);
        printf\("  -s : Show hashing statistics \(H/s\)\\n"\);
        return 1;
    \}

    const char\* prefix = argv\[arg_idx\];
    const char\* out_dir = argv\[arg_idx \+ 1\];
    size_t prefix_len = strlen\(prefix\);

    if \(prefix_len > 16\) \{
        printf\("Prefix too long\.\\n"\);
        return 1;
    \}''',
r'''    if (argc - arg_idx < 2) {
        printf("Usage: %s [-s] <output_dir> <prefix1> [prefix2] ...\n", argv[0]);
        printf("  -s : Show hashing statistics (H/s)\n");
        return 1;
    }

    const char* out_dir = argv[arg_idx];
    int num_prefixes = argc - arg_idx - 1;
    char** prefixes = &argv[arg_idx + 1];

    for (int i=0; i<num_prefixes; i++) {
        if (strlen(prefixes[i]) > 16) {
            printf("Prefix %s too long (max 16 chars).\n", prefixes[i]);
            return 1;
        }
    }''', code)

with open('main.c', 'w') as f:
    f.write(code)

print("Replaced CLI arguments.")
