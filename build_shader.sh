#!/bin/sh
set -e
SHADER="shader.comp"

printf '#version 450\n#define _PASS_1\n' > /tmp/pass1.comp
tail -n +2 "$SHADER" >> /tmp/pass1.comp
glslangValidator -V --target-env vulkan1.1 /tmp/pass1.comp -o scan.spv > /dev/null

printf '#version 450\n#define _PASS_2\n' > /tmp/pass2.comp
tail -n +2 "$SHADER" >> /tmp/pass2.comp
glslangValidator -V --target-env vulkan1.1 /tmp/pass2.comp -o invert.spv > /dev/null

printf '#version 450\n#define _PASS_3\n' > /tmp/pass3.comp
tail -n +2 "$SHADER" >> /tmp/pass3.comp
glslangValidator -V --target-env vulkan1.1 /tmp/pass3.comp -o finalize.spv > /dev/null

for f in scan.spv invert.spv finalize.spv; do
    spirv-dis "$f" -o /tmp/dis.txt
    sed -E 's/(%fe_mul_[a-zA-Z0-9_]* = OpFunction %[a-zA-Z0-9_]*) None/\1 DontInline/' /tmp/dis.txt > /tmp/patched.txt
    sed -E 's/(%fe_sq_[a-zA-Z0-9_]* = OpFunction %[a-zA-Z0-9_]*) None/\1 DontInline/' /tmp/patched.txt > /tmp/patched.2.txt
    spirv-as /tmp/patched.2.txt -o "$f"
done
