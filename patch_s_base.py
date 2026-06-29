import re

with open('shader.comp', 'r') as f:
    code = f.read()

code = re.sub(r'shared int s_base_YplusX\[10\];\nshared int s_base_YminusX\[10\];\nshared int s_base_Z2\[10\];\nshared int s_base_T\[10\];\n', '', code)

# Remove the block that loads s_base_*
code = re.sub(r'    if \(lid == 0\) \{\n        for \(int i = 0; i < 10; i\+\+\) \{\n            s_base_YplusX\[i\] = base_point.YplusX\[i\];\n            s_base_YminusX\[i\] = base_point.YminusX\[i\];\n            s_base_Z2\[i\] = base_point.Z2\[i\];\n            s_base_T\[i\] = base_point.T\[i\];\n        \}\n    \}\n', '', code)

# ge_add usage of s_base_*
code = code.replace("s_base_YplusX", "base_point.YplusX")
code = code.replace("s_base_YminusX", "base_point.YminusX")
code = code.replace("s_base_Z2", "base_point.Z2")
code = code.replace("s_base_T", "base_point.T")

with open('shader.comp', 'w') as f:
    f.write(code)

print("Removed s_base_* from shared memory and replaced with direct accesses.")
