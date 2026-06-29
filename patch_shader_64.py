import re

with open('shader.comp', 'r') as f:
    code = f.read()

code = code.replace("layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;", "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;")
code = code.replace("const uint WG_SIZE = 32u;", "const uint WG_SIZE = 64u;")

with open('shader.comp', 'w') as f:
    f.write(code)

with open('main.c', 'r') as f:
    code = f.read()

code = code.replace("#define WORKGROUP_SIZE 32", "#define WORKGROUP_SIZE 64")
with open('main.c', 'w') as f:
    f.write(code)

print("Patched back to 64.")
