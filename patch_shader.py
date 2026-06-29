import re

with open('shader.comp', 'r') as f:
    code = f.read()

code = code.replace("layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;", "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;")
code = code.replace("const uint WG_SIZE = 128u;", "const uint WG_SIZE = 64u;")
code = code.replace("shared int shared_P[10][WG_SIZE];", "shared int shared_P[WG_SIZE][10];")
code = code.replace("shared int shared_S[10][WG_SIZE];", "shared int shared_S[WG_SIZE][10];")
code = code.replace("shared_P[i][lid] = R_Z[i];", "shared_P[lid][i] = R_Z[i];")
code = code.replace("shared_S[i][lid] = R_Z[i];", "shared_S[lid][i] = R_Z[i];")
code = code.replace("shared_P[i][lid] = one[i];", "shared_P[lid][i] = one[i];")
code = code.replace("shared_S[i][lid] = one[i];", "shared_S[lid][i] = one[i];")

code = code.replace("P_curr[i] = shared_P[i][lid];", "P_curr[i] = shared_P[lid][i];")
code = code.replace("P_prev[i] = shared_P[i][lid - stride];", "P_prev[i] = shared_P[lid - stride][i];")

code = code.replace("S_curr[i] = shared_S[i][lid];", "S_curr[i] = shared_S[lid][i];")
code = code.replace("S_next[i] = shared_S[i][lid + stride];", "S_next[i] = shared_S[lid + stride][i];")

code = code.replace("shared_P[i][lid] = temp_P[i];", "shared_P[lid][i] = temp_P[i];")
code = code.replace("shared_S[i][lid] = temp_S[i];", "shared_S[lid][i] = temp_S[i];")

code = code.replace("last_P[i] = shared_P[i][WG_SIZE - 1u];", "last_P[i] = shared_P[WG_SIZE - 1u][i];")
code = code.replace("P_excl[i] = shared_P[i][lid - 1];", "P_excl[i] = shared_P[lid - 1][i];")
code = code.replace("S_excl[i] = shared_S[i][lid + 1];", "S_excl[i] = shared_S[lid + 1][i];")

with open('shader.comp', 'w') as f:
    f.write(code)

with open('main.c', 'r') as f:
    code = f.read()

code = code.replace("#define WORKGROUP_SIZE 128", "#define WORKGROUP_SIZE 64")
with open('main.c', 'w') as f:
    f.write(code)

print("Patched.")
