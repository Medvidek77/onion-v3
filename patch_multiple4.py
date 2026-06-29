import re

with open('main.c', 'r') as f:
    code = f.read()

# Add pattern buffer and update descriptor sets logic
code = re.sub(
r'''    /\* Result buffer \(Double Buffered\) \*/
    bufferInfo\.size = result_size;
    bufferInfo\.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBuffer resultBuffer\[2\];
    VkDeviceMemory resultMemory\[2\];
    for \(int i=0; i<2; i\+\+\) \{
        VK_CHECK\(vkCreateBuffer\(device, &bufferInfo, NULL, &resultBuffer\[i\]\)\);
        vkGetBufferMemoryRequirements\(device, resultBuffer\[i\], &memReqs\);
        allocInfo\.allocationSize = memReqs\.size;
        allocInfo\.memoryTypeIndex = findMemoryType\(physicalDevice, memReqs\.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT \| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\);
        VK_CHECK\(vkAllocateMemory\(device, &allocInfo, NULL, &resultMemory\[i\]\)\);
        vkBindBufferMemory\(device, resultBuffer\[i\], resultMemory\[i\], 0\);
    \}''',
r'''    /* Result buffer (Double Buffered) */
    bufferInfo.size = result_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBuffer resultBuffer[2];
    VkDeviceMemory resultMemory[2];
    for (int i=0; i<2; i++) {
        VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &resultBuffer[i]));
        vkGetBufferMemoryRequirements(device, resultBuffer[i], &memReqs);
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &resultMemory[i]));
        vkBindBufferMemory(device, resultBuffer[i], resultMemory[i], 0);
    }

    /* Patterns Buffer */
    VkBuffer patternsBuffer;
    VkDeviceMemory patternsMemory;
    bufferInfo.size = sizeof(patterns);
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &patternsBuffer));
    vkGetBufferMemoryRequirements(device, patternsBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &patternsMemory));
    vkBindBufferMemory(device, patternsBuffer, patternsMemory, 0);

    void* mappedPatterns;
    VK_CHECK(vkMapMemory(device, patternsMemory, 0, sizeof(patterns), 0, &mappedPatterns));
    memcpy(mappedPatterns, patterns, sizeof(patterns));
    vkUnmapMemory(device, patternsMemory);''', code)

code = re.sub(
r'''    VkDescriptorSetLayoutBinding bindings\[3\] = \{
        \{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL\},
        \{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL\},
        \{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL\}
    \};
    VkDescriptorSetLayoutCreateInfo layoutInfo = \{
        \.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        \.bindingCount = 3,
        \.pBindings = bindings
    \};''',
r'''    VkDescriptorSetLayoutBinding bindings[4] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
        .pBindings = bindings
    };''', code)

code = re.sub(
r'''    for \(int i=0; i<2; i\+\+\) \{
        VkDescriptorBufferInfo baseBufInfo = \{ basepointBuffer\[i\], 0, VK_WHOLE_SIZE \};
        VkDescriptorBufferInfo resultBufInfo = \{ resultBuffer\[i\], 0, VK_WHOLE_SIZE \};

        VkWriteDescriptorSet descriptorWrites\[3\] = \{
            \{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets\[i\], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &baseBufInfo, NULL \},
            \{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets\[i\], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &offsetBufInfo, NULL \},
            \{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets\[i\], 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &resultBufInfo, NULL \}
        \};
        vkUpdateDescriptorSets\(device, 3, descriptorWrites, 0, NULL\);
    \}''',
r'''    for (int i=0; i<2; i++) {
        VkDescriptorBufferInfo baseBufInfo = { basepointBuffer[i], 0, VK_WHOLE_SIZE };
        VkDescriptorBufferInfo resultBufInfo = { resultBuffer[i], 0, VK_WHOLE_SIZE };
        VkDescriptorBufferInfo patternsBufInfo = { patternsBuffer, 0, VK_WHOLE_SIZE };

        VkWriteDescriptorSet descriptorWrites[4] = {
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets[i], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &baseBufInfo, NULL },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets[i], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &offsetBufInfo, NULL },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets[i], 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &resultBufInfo, NULL },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSets[i], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &patternsBufInfo, NULL }
        };
        vkUpdateDescriptorSets(device, 4, descriptorWrites, 0, NULL);
    }''', code)

with open('main.c', 'w') as f:
    f.write(code)

print("Updated buffers.")
