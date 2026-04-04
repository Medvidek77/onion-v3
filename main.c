#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <vulkan/vulkan.h>
#include <sodium.h>

#define BATCH_SIZE (1 << 20) // 1 million keys per batch

#define VK_CHECK(x) \
    do { \
        VkResult err = x; \
        if (err) { \
            fprintf(stderr, "Vulkan error %d at %s:%d\n", err, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    fprintf(stderr, "Failed to find suitable memory type\n");
    exit(1);
}

void get_pubkey(const unsigned char *sk, unsigned char *pk) {
    unsigned char h[64];
    crypto_hash_sha512(h, sk, 32);
    h[0] &= 248;
    h[31] &= 127;
    h[31] |= 64;
    crypto_scalarmult_ed25519_base_noclamp(pk, h);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <prefix> <output_dir>\n", argv[0]);
        return 1;
    }

    const char* prefix = argv[1];
    const char* out_dir = argv[2];
    size_t prefix_len = strlen(prefix);

    if (prefix_len > 16) {
        printf("Prefix too long.\n");
        return 1;
    }

    if (sodium_init() < 0) {
        printf("Failed to initialize libsodium\n");
        return 1;
    }

    mkdir(out_dir, 0700);

    // Vulkan Initialization
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "TorVanityVulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = 0
    };

    VkInstance instance;
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance.\n");
        return 1;
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "Failed to find GPUs with Vulkan support.\n");
        return 1;
    }
    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    // Just pick the first device for simplicity (usually the discrete GPU on a standard setup)
    VkPhysicalDevice physicalDevice = physicalDevices[0];
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    printf("Using Vulkan Device: %s\n", deviceProperties.deviceName);
    free(physicalDevices);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    uint32_t computeFamily = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            computeFamily = i;
            break;
        }
    }
    free(queueFamilies);
    if (computeFamily == (uint32_t)-1) {
        fprintf(stderr, "Failed to find compute queue family.\n");
        return 1;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = computeFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    const char* deviceExtensions[] = {
        VK_KHR_8BIT_STORAGE_EXTENSION_NAME
    };

    VkPhysicalDevice8BitStorageFeatures features8 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
        .storageBuffer8BitAccess = VK_TRUE
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features8,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions
    };

    VkDevice device;
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device));

    VkQueue computeQueue;
    vkGetDeviceQueue(device, computeFamily, 0, &computeQueue);

    // Buffers setup
    size_t pubkeys_size = BATCH_SIZE * 32;
    size_t result_size = sizeof(int);

    uint8_t* host_pubkeys = malloc(pubkeys_size);
    uint8_t* host_secrets = malloc(pubkeys_size);

    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    VkMemoryRequirements memReqs;

    // Pubkeys buffer
    bufferInfo.size = pubkeys_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer pubkeysBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &pubkeysBuffer));
    vkGetBufferMemoryRequirements(device, pubkeysBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory pubkeysMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &pubkeysMemory));
    vkBindBufferMemory(device, pubkeysBuffer, pubkeysMemory, 0);

    // Prefix buffer
    bufferInfo.size = prefix_len;
    VkBuffer prefixBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &prefixBuffer));
    vkGetBufferMemoryRequirements(device, prefixBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory prefixMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &prefixMemory));
    vkBindBufferMemory(device, prefixBuffer, prefixMemory, 0);

    // Write prefix
    void* mappedPrefix;
    vkMapMemory(device, prefixMemory, 0, prefix_len, 0, &mappedPrefix);
    memcpy(mappedPrefix, prefix, prefix_len);
    vkUnmapMemory(device, prefixMemory);

    // Result buffer
    bufferInfo.size = result_size;
    VkBuffer resultBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &resultBuffer));
    vkGetBufferMemoryRequirements(device, resultBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory resultMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &resultMemory));
    vkBindBufferMemory(device, resultBuffer, resultMemory, 0);

    // Load shader
    FILE* f = fopen("shader.spv", "rb");
    if (!f) {
        fprintf(stderr, "Failed to open shader.spv. Did you compile it?\n");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    size_t shaderSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint32_t* shaderCode = malloc(shaderSize);
    size_t read_bytes = fread(shaderCode, 1, shaderSize, f);
    fclose(f);
    if (read_bytes != shaderSize) {
        fprintf(stderr, "Failed to read full shader.spv\n");
        free(shaderCode);
        return 1;
    }

    VkShaderModuleCreateInfo shaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderSize,
        .pCode = shaderCode
    };
    VkShaderModule computeShaderModule;
    VK_CHECK(vkCreateShaderModule(device, &shaderInfo, NULL, &computeShaderModule));
    free(shaderCode);

    // Descriptor set layout
    VkDescriptorSetLayoutBinding bindings[3] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings = bindings
    };
    VkDescriptorSetLayout descriptorSetLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout));

    // Pipeline Layout
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(uint32_t) * 2 // batch_size and prefix_len
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout));

    // Compute Pipeline
    VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = computeShaderModule,
            .pName = "main"
        },
        .layout = pipelineLayout
    };
    VkPipeline computePipeline;
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &computePipeline));

    // Descriptor Pool
    VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 };
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool));

    // Descriptor Set
    VkDescriptorSetAllocateInfo allocSetInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };
    VkDescriptorSet descriptorSet;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocSetInfo, &descriptorSet));

    VkDescriptorBufferInfo pubkeysBufInfo = { pubkeysBuffer, 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo prefixBufInfo = { prefixBuffer, 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo resultBufInfo = { resultBuffer, 0, VK_WHOLE_SIZE };

    VkWriteDescriptorSet descriptorWrites[3] = {
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &pubkeysBufInfo, NULL },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &prefixBufInfo, NULL },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &resultBufInfo, NULL }
    };
    vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, NULL);

    // Command Pool & Buffer
    VkCommandPoolCreateInfo cmdPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = computeFamily
    };
    VkCommandPool commandPool;
    VK_CHECK(vkCreateCommandPool(device, &cmdPoolInfo, NULL, &commandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffer));

    // Record Command Buffer
    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    uint32_t pushConstants[2] = { BATCH_SIZE, (uint32_t)prefix_len };
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants), pushConstants);

    vkCmdDispatch(commandBuffer, BATCH_SIZE / 256, 1, 1);
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    printf("Starting search for prefix '%s' via Vulkan...\n", prefix);

    uint64_t total_checked = 0;
    bool found = false;

    void* mappedPubkeys;
    vkMapMemory(device, pubkeysMemory, 0, pubkeys_size, 0, &mappedPubkeys);
    void* mappedResult;
    vkMapMemory(device, resultMemory, 0, result_size, 0, &mappedResult);

    while (!found) {
        for (int i = 0; i < BATCH_SIZE; ++i) {
            randombytes_buf(&host_secrets[i * 32], 32);
            get_pubkey(&host_secrets[i * 32], &host_pubkeys[i * 32]);
        }
        memcpy(mappedPubkeys, host_pubkeys, pubkeys_size);

        int init_result = -1;
        memcpy(mappedResult, &init_result, sizeof(int));

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };
        VK_CHECK(vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(computeQueue));

        int result_index;
        memcpy(&result_index, mappedResult, sizeof(int));

        total_checked += BATCH_SIZE;
        if (total_checked % (BATCH_SIZE * 10) == 0) {
            printf("Checked %lu keys...\n", total_checked);
        }

        if (result_index != -1) {
            printf("\nFound match at index %d!\n", result_index);
            printf("Total keys checked: %lu\n", total_checked);

            unsigned char* secret = &host_secrets[result_index * 32];

            char path[512];
            if (snprintf(path, sizeof(path), "%s/%s_keys", out_dir, prefix) >= (int)sizeof(path)) {
                printf("Output path too long.\n");
                break;
            }
            mkdir(path, 0700);

            char key_path[550];
            if (snprintf(key_path, sizeof(key_path), "%s/hs_ed25519_secret_key", path) >= (int)sizeof(key_path)) {
                printf("Key path too long.\n");
                break;
            }

            FILE* f = fopen(key_path, "wb");
            if (f) {
                char header[32] = "== ed25519v1-secret: type0 ==\0\0\0";
                fwrite(header, 1, 32, f);

                unsigned char expanded_sk[64];
                crypto_hash_sha512(expanded_sk, secret, 32);
                expanded_sk[0] &= 248;
                expanded_sk[31] &= 127;
                expanded_sk[31] |= 64;

                fwrite(expanded_sk, 1, 64, f);
                fclose(f);
                printf("Secret key written to: %s\n", key_path);
            } else {
                printf("Failed to open %s for writing.\n", key_path);
            }
            found = true;
        }
    }

    vkUnmapMemory(device, pubkeysMemory);
    vkUnmapMemory(device, resultMemory);

    // Cleanup
    free(host_pubkeys);
    free(host_secrets);

    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyPipeline(device, computePipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyShaderModule(device, computeShaderModule, NULL);

    vkDestroyBuffer(device, pubkeysBuffer, NULL);
    vkFreeMemory(device, pubkeysMemory, NULL);
    vkDestroyBuffer(device, prefixBuffer, NULL);
    vkFreeMemory(device, prefixMemory, NULL);
    vkDestroyBuffer(device, resultBuffer, NULL);
    vkFreeMemory(device, resultMemory, NULL);

    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}
