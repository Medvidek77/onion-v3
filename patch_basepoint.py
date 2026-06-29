import re

with open('shader.comp', 'r') as f:
    code = f.read()

# Remove the block that attempts to assign to readonly buffer base_point
code = re.sub(r'    if \(lid == 0\) \{\n        for\(int i=0; i<10; i\+\+\) \{\n            base_point\.YplusX\[i\] = base_point\.YplusX\[i\];\n            base_point\.YminusX\[i\] = base_point\.YminusX\[i\];\n            base_point\.Z2\[i\] = base_point\.Z2\[i\];\n            base_point\.T\[i\] = base_point\.T\[i\];\n        \}\n    \}\n', '', code)

with open('shader.comp', 'w') as f:
    f.write(code)

print("Fixed base_point assignment.")
