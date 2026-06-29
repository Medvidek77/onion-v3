import re

with open('main.c', 'r') as f:
    code = f.read()

# Fix newline literals in printf strings
code = code.replace('printf("Usage: %s [-s] <output_dir> <prefix1> [prefix2] ...\n", argv[0]);\n        printf("  -s : Show hashing statistics (H/s)\n");',
                    'printf("Usage: %s [-s] <output_dir> <prefix1> [prefix2] ...\\n", argv[0]);\n        printf("  -s : Show hashing statistics (H/s)\\n");')

code = code.replace('printf("Prefix %s too long (max 16 chars).\n", prefixes[i]);',
                    'printf("Prefix %s too long (max 16 chars).\\n", prefixes[i]);')

code = code.replace('printf("Too many prefixes (max %d)\n", MAX_PREFIXES);',
                    'printf("Too many prefixes (max %d)\\n", MAX_PREFIXES);')

code = code.replace('printf("Invalid character in prefix: %c\n", c);',
                    'printf("Invalid character in prefix: %c\\n", c);')

code = code.replace('printf("Starting search for prefix \'%s\' via Vulkan...\n\n", prefix);',
                    'printf("Starting search for %d prefixes via Vulkan...\\n\\n", num_prefixes);')

# Wait, replacing across lines using simple replace could fail if the regex replacement resulted in actual newlines.
# Let's just fix them with regex substitution.
code = re.sub(r'printf\("Usage: %s \[-s\] <output_dir> <prefix1> \[prefix2\] \.\.\.\n", argv\[0\]\);', r'printf("Usage: %s [-s] <output_dir> <prefix1> [prefix2] ...\\n", argv[0]);', code)
code = re.sub(r'printf\("  -s : Show hashing statistics \(H/s\)\n"\);', r'printf("  -s : Show hashing statistics (H/s)\\n");', code)
code = re.sub(r'printf\("Prefix %s too long \(max 16 chars\)\.\n", prefixes\[i\]\);', r'printf("Prefix %s too long (max 16 chars).\\n", prefixes[i]);', code)
code = re.sub(r'printf\("Too many prefixes \(max %d\)\n", MAX_PREFIXES\);', r'printf("Too many prefixes (max %d)\\n", MAX_PREFIXES);', code)
code = re.sub(r'printf\("Invalid character in prefix: %c\n", c\);', r'printf("Invalid character in prefix: %c\\n", c);', code)


with open('main.c', 'w') as f:
    f.write(code)
