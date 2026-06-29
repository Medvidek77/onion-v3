import re

with open('main.c', 'r') as f:
    code = f.read()

# Replace PushConstants struct setup and buffer setup

code = re.sub(
r'''    typedef struct \{
        uint32_t batch_size;
        uint32_t valid_bytes;
        uint8_t  byte_target\[16\];
        uint8_t  byte_mask\[16\];
    \} PushConstants;

    PushConstants pc_data = \{0\};
    pc_data\.batch_size = BATCH_SIZE;
    pc_data\.valid_bytes = valid_bytes;
    memcpy\(pc_data\.byte_target, byte_target, 16\);
    memcpy\(pc_data\.byte_mask, byte_mask, 16\);

    /\* Pipeline Layout \*/
    VkPushConstantRange pushConstantRange = \{
        \.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        \.offset = 0,
        \.size = sizeof\(PushConstants\)
    \};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = \{
        \.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        \.setLayoutCount = 1,
        \.pSetLayouts = &descriptorSetLayout,
        \.pushConstantRangeCount = 1,
        \.pPushConstantRanges = &pushConstantRange
    \};''',
r'''    typedef struct {
        uint32_t batch_size;
        uint32_t num_patterns;
    } PushConstants;

    PushConstants pc_data = {0};
    pc_data.batch_size = BATCH_SIZE;
    pc_data.num_patterns = num_prefixes;

    /* Pipeline Layout */
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(PushConstants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };''', code)

with open('main.c', 'w') as f:
    f.write(code)

print("Replaced PushConstants.")
