import re

with open('main.c', 'r') as f:
    code = f.read()

code = re.sub(r'printf\("Usage: %s \[-s\] <prefix1> \[prefix2\] \.\.\. <output_dir>\n", argv\[0\]\);', r'printf("Usage: %s [-s] <prefix1> [prefix2] ... <output_dir>\\n", argv[0]);', code)
code = re.sub(r'printf\("  -s : Show hashing statistics \(H/s\)\n"\);', r'printf("  -s : Show hashing statistics (H/s)\\n");', code)

with open('main.c', 'w') as f:
    f.write(code)
