import re

with open('shader.comp', 'r') as f:
    code = f.read()

# Update struct and layout in shader.comp
code = re.sub(
r'''layout\(push_constant\) uniform PushConstants \{
    uint batch_size;
    uint valid_bytes;
    uint8_t byte_target\[16\];
    uint8_t byte_mask\[16\];
\} pc;''',
r'''struct Pattern {
    uint valid_bytes;
    uint8_t byte_target[16];
    uint8_t byte_mask[16];
};
layout(std430,set=0,binding=3) readonly buffer restrict PatternBuffer { Pattern patterns[]; };

layout(push_constant) uniform PushConstants {
    uint batch_size;
    uint num_patterns;
} pc;''', code)

code = re.sub(
r'''bool check_prefix\(in int h\[10\]\) \{
    int h0=h\[0\],h1=h\[1\],h2=h\[2\],h3=h\[3\],h4=h\[4\],h5=h\[5\],h6=h\[6\],h7=h\[7\],h8=h\[8\],h9=h\[9\];
    int c;''',
r'''bool check_prefix(in int h[10]) {
    int h0=h[0],h1=h[1],h2=h[2],h3=h[3],h4=h[4],h5=h[5],h6=h[6],h7=h[7],h8=h[8],h9=h[9];
    int c;''', code)

code = re.sub(
r'''    c=h0>>26; h1\+=c; h0-=c<<26;

    b = uint8_t\(h0\);
    match = match && \(pc\.valid_bytes <= 0 \|\| \(b & pc\.byte_mask\[0\]\) == pc\.byte_target\[0\]\);
    b = uint8_t\(h0 >> 8\);
    match = match && \(pc\.valid_bytes <= 1 \|\| \(b & pc\.byte_mask\[1\]\) == pc\.byte_target\[1\]\);
    b = uint8_t\(h0 >> 16\);
    match = match && \(pc\.valid_bytes <= 2 \|\| \(b & pc\.byte_mask\[2\]\) == pc\.byte_target\[2\]\);

    c=h1>>25; h2\+=c; h1-=c<<25;

    b = uint8_t\(\(h0 >> 24\) \| \(h1 << 2\)\);
    match = match && \(pc\.valid_bytes <= 3 \|\| \(b & pc\.byte_mask\[3\]\) == pc\.byte_target\[3\]\);
    b = uint8_t\(h1 >> 6\);
    match = match && \(pc\.valid_bytes <= 4 \|\| \(b & pc\.byte_mask\[4\]\) == pc\.byte_target\[4\]\);
    b = uint8_t\(h1 >> 14\);
    match = match && \(pc\.valid_bytes <= 5 \|\| \(b & pc\.byte_mask\[5\]\) == pc\.byte_target\[5\]\);

    c=h2>>26; h3\+=c; h2-=c<<26;

    b = uint8_t\(\(h1 >> 22\) \| \(h2 << 3\)\);
    match = match && \(pc\.valid_bytes <= 6 \|\| \(b & pc\.byte_mask\[6\]\) == pc\.byte_target\[6\]\);
    b = uint8_t\(h2 >> 5\);
    match = match && \(pc\.valid_bytes <= 7 \|\| \(b & pc\.byte_mask\[7\]\) == pc\.byte_target\[7\]\);
    b = uint8_t\(h2 >> 13\);
    match = match && \(pc\.valid_bytes <= 8 \|\| \(b & pc\.byte_mask\[8\]\) == pc\.byte_target\[8\]\);

    c=h3>>25; h4\+=c; h3-=c<<25;

    b = uint8_t\(\(h2 >> 21\) \| \(h3 << 5\)\);
    match = match && \(pc\.valid_bytes <= 9 \|\| \(b & pc\.byte_mask\[9\]\) == pc\.byte_target\[9\]\);
    b = uint8_t\(h3 >> 3\);
    match = match && \(pc\.valid_bytes <= 10 \|\| \(b & pc\.byte_mask\[10\]\) == pc\.byte_target\[10\]\);
    b = uint8_t\(h3 >> 11\);
    match = match && \(pc\.valid_bytes <= 11 \|\| \(b & pc\.byte_mask\[11\]\) == pc\.byte_target\[11\]\);

    c=h4>>26; h5\+=c; h4-=c<<26;

    b = uint8_t\(\(h3 >> 19\) \| \(h4 << 6\)\);
    match = match && \(pc\.valid_bytes <= 12 \|\| \(b & pc\.byte_mask\[12\]\) == pc\.byte_target\[12\]\);
    b = uint8_t\(h4 >> 2\);
    match = match && \(pc\.valid_bytes <= 13 \|\| \(b & pc\.byte_mask\[13\]\) == pc\.byte_target\[13\]\);
    b = uint8_t\(h4 >> 10\);
    match = match && \(pc\.valid_bytes <= 14 \|\| \(b & pc\.byte_mask\[14\]\) == pc\.byte_target\[14\]\);
    b = uint8_t\(h4 >> 18\);
    match = match && \(pc\.valid_bytes <= 15 \|\| \(b & pc\.byte_mask\[15\]\) == pc\.byte_target\[15\]\);

    return match;''',
r'''    c=h0>>26; h1+=c; h0-=c<<26;
    c=h1>>25; h2+=c; h1-=c<<25;
    c=h2>>26; h3+=c; h2-=c<<26;
    c=h3>>25; h4+=c; h3-=c<<25;
    c=h4>>26; h5+=c; h4-=c<<26;

    uint8_t out_bytes[16];
    out_bytes[0] = uint8_t(h0);
    out_bytes[1] = uint8_t(h0 >> 8);
    out_bytes[2] = uint8_t(h0 >> 16);
    out_bytes[3] = uint8_t((h0 >> 24) | (h1 << 2));
    out_bytes[4] = uint8_t(h1 >> 6);
    out_bytes[5] = uint8_t(h1 >> 14);
    out_bytes[6] = uint8_t((h1 >> 22) | (h2 << 3));
    out_bytes[7] = uint8_t(h2 >> 5);
    out_bytes[8] = uint8_t(h2 >> 13);
    out_bytes[9] = uint8_t((h2 >> 21) | (h3 << 5));
    out_bytes[10] = uint8_t(h3 >> 3);
    out_bytes[11] = uint8_t(h3 >> 11);
    out_bytes[12] = uint8_t((h3 >> 19) | (h4 << 6));
    out_bytes[13] = uint8_t(h4 >> 2);
    out_bytes[14] = uint8_t(h4 >> 10);
    out_bytes[15] = uint8_t(h4 >> 18);

    for (uint p = 0; p < pc.num_patterns; p++) {
        bool match = true;
        for (uint i = 0; i < patterns[p].valid_bytes; i++) {
            if ((out_bytes[i] & patterns[p].byte_mask[i]) != patterns[p].byte_target[i]) {
                match = false;
                break; // Break here is fine because it divergence depends on the match status which is usually rare, though it might still be better to avoid early break? Actually, since one pattern match is sufficient, we could do full iteration or break early. The user wanted to remove early return.
            }
        }
        if (match) return true;
    }

    return false;''', code)

with open('shader.comp', 'w') as f:
    f.write(code)

print("Replaced check_prefix logic.")
