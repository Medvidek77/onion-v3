import re

with open('shader.comp', 'r') as f:
    code = f.read()

# Let's fix the early return inside the loop to perfectly match "divergence-free"
code = re.sub(
r'''    for \(uint p = 0; p < pc\.num_patterns; p\+\+\) \{
        bool match = true;
        for \(uint i = 0; i < patterns\[p\]\.valid_bytes; i\+\+\) \{
            if \(\(out_bytes\[i\] & patterns\[p\]\.byte_mask\[i\]\) != patterns\[p\]\.byte_target\[i\]\) \{
                match = false;
                break; // Break here is fine because it divergence depends on the match status which is usually rare, though it might still be better to avoid early break\? Actually, since one pattern match is sufficient, we could do full iteration or break early\. The user wanted to remove early return\.
            \}
        \}
        if \(match\) return true;
    \}

    return false;''',
r'''    bool any_match = false;
    for (uint p = 0; p < pc.num_patterns; p++) {
        bool match = true;
        for (uint i = 0; i < 16; i++) {
            if (i < patterns[p].valid_bytes) {
                match = match && ((out_bytes[i] & patterns[p].byte_mask[i]) == patterns[p].byte_target[i]);
            }
        }
        any_match = any_match || match;
    }

    return any_match;''', code)

with open('shader.comp', 'w') as f:
    f.write(code)

print("Removed early returns completely in check_prefix.")
