#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <vulkan/vulkan.h>
#include <sodium.h>
#include "ed25519.h"
#include "ge.h"
#include "sc.h"

#define BATCH_SIZE (1 << 20) // 1 million keys per batch
#define NUM_THREADS 12       // Max CPU threads for Ryzen 5 5600

// Internal CPU structs mapping directly to GPU buffers
typedef struct {
    int32_t X[10];
    int32_t Y[10];
    int32_t Z[10];
    int32_t T[10];
} gpu_ge_p3;

typedef struct {
    int32_t YplusX[10];
    int32_t YminusX[10];
    int32_t Z[10];
    int32_t T2d[10];
} gpu_ge_cached;

#ifdef DEBUG
    #define DEBUG_PRINT(...) printf("[DEBUG] " __VA_ARGS__)
#else
    #define DEBUG_PRINT(...) do {} while (0)
#endif

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

void fe_copy_to_gpu(int32_t* dst, const fe src) {
    for (int i=0; i<10; i++) dst[i] = src[i];
}

void convert_p3_to_gpu(gpu_ge_p3* dst, const ge_p3* src) {
    fe_copy_to_gpu(dst->X, src->X);
    fe_copy_to_gpu(dst->Y, src->Y);
    fe_copy_to_gpu(dst->Z, src->Z);
    fe_copy_to_gpu(dst->T, src->T);
}

void convert_cached_to_gpu(gpu_ge_cached* dst, const ge_cached* src) {
    fe_copy_to_gpu(dst->YplusX, src->YplusX);
    fe_copy_to_gpu(dst->YminusX, src->YminusX);
    fe_copy_to_gpu(dst->Z, src->Z);
    fe_copy_to_gpu(dst->T2d, src->T2d);
}

void generate_offsets(gpu_ge_cached* offsets, uint32_t count) {
    printf("Precomputing %u point additions for GPU... ", count);
    fflush(stdout);

    // We want offsets O[i] = i * 8 * G
    // G is the base point
    ge_p3 G, cur_p3;
    ge_scalarmult_base(&G, (const unsigned char*)"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"); // G

    // Calculate 8*G
    ge_p1p1 p1;
    ge_p2 p2;
    ge_p3_to_p2(&p2, &G);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1); // 2G
    ge_p3_to_p2(&p2, &cur_p3);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1); // 4G
    ge_p3_to_p2(&p2, &cur_p3);
    ge_p2_dbl(&p1, &p2); ge_p1p1_to_p3(&cur_p3, &p1); // 8G

    ge_p3 step_p3 = cur_p3;
    ge_cached step_cached;
    ge_p3_to_cached(&step_cached, &step_p3);

    ge_p3 sum;
    ge_p3_0(&sum); // Start at 0*G

    for (uint32_t i = 0; i < count; i++) {
        ge_cached c;
        ge_p3_to_cached(&c, &sum);
        convert_cached_to_gpu(&offsets[i], &c);

        ge_add(&p1, &sum, &step_cached);
        ge_p1p1_to_p3(&sum, &p1);
    }
    printf("Done.\n");
}

double get_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
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
    DEBUG_PRINT("Vulkan instance created successfully.\n");

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "Failed to find GPUs with Vulkan support.\n");
        return 1;
    }
    DEBUG_PRINT("Found %d physical device(s).\n", deviceCount);

    VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    // Attempt to prefer Discrete GPUs, fallback to first available
    VkPhysicalDevice physicalDevice = physicalDevices[0];
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
        DEBUG_PRINT("Device %d: %s (Type: %d)\n", i, props.deviceName, props.deviceType);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = physicalDevices[i];
            break;
        }
    }

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
        DEBUG_PRINT("Queue Family %d: flags 0x%08X, count %d\n", i, queueFamilies[i].queueFlags, queueFamilies[i].queueCount);
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
    DEBUG_PRINT("Selected Compute Queue Family Index: %d\n", computeFamily);

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
    size_t basepoint_size = sizeof(gpu_ge_p3);
    size_t offsets_size = BATCH_SIZE * sizeof(gpu_ge_cached);
    size_t result_size = sizeof(int);

    // Allocate host buffers
    gpu_ge_p3 host_basepoint;
    gpu_ge_cached* host_offsets = malloc(offsets_size);
    generate_offsets(host_offsets, BATCH_SIZE);

    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    VkMemoryRequirements memReqs;

    // BasePoint buffer
    bufferInfo.size = basepoint_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer basepointBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &basepointBuffer));
    vkGetBufferMemoryRequirements(device, basepointBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory basepointMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &basepointMemory));
    vkBindBufferMemory(device, basepointBuffer, basepointMemory, 0);

    // Offsets buffer
    bufferInfo.size = offsets_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBuffer offsetsBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &offsetsBuffer));
    vkGetBufferMemoryRequirements(device, offsetsBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory offsetsMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &offsetsMemory));
    vkBindBufferMemory(device, offsetsBuffer, offsetsMemory, 0);

    // Write Offsets once
    void* mappedOffsets;
    vkMapMemory(device, offsetsMemory, 0, offsets_size, 0, &mappedOffsets);
    memcpy(mappedOffsets, host_offsets, offsets_size);
    vkUnmapMemory(device, offsetsMemory);

    // Prefix buffer
    bufferInfo.size = prefix_len;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBuffer prefixBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, &prefixBuffer));
    vkGetBufferMemoryRequirements(device, prefixBuffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory prefixMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, &prefixMemory));
    vkBindBufferMemory(device, prefixBuffer, prefixMemory, 0);

    // Write prefix once
    void* mappedPrefix;
    vkMapMemory(device, prefixMemory, 0, prefix_len, 0, &mappedPrefix);
    memcpy(mappedPrefix, prefix, prefix_len);
    vkUnmapMemory(device, prefixMemory);

    // Result buffer
    bufferInfo.size = result_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
    VkDescriptorSetLayoutBinding bindings[4] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
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
    VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 };
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

    VkDescriptorBufferInfo baseBufInfo = { basepointBuffer, 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo offsetBufInfo = { offsetsBuffer, 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo prefixBufInfo = { prefixBuffer, 0, VK_WHOLE_SIZE };
    VkDescriptorBufferInfo resultBufInfo = { resultBuffer, 0, VK_WHOLE_SIZE };

    VkWriteDescriptorSet descriptorWrites[4] = {
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &baseBufInfo, NULL },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &offsetBufInfo, NULL },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &prefixBufInfo, NULL },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL, descriptorSet, 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &resultBufInfo, NULL }
    };
    vkUpdateDescriptorSets(device, 4, descriptorWrites, 0, NULL);

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

    void* mappedBasepoint;
    vkMapMemory(device, basepointMemory, 0, basepoint_size, 0, &mappedBasepoint);
    void* mappedResult;
    vkMapMemory(device, resultMemory, 0, result_size, 0, &mappedResult);

    uint8_t base_secret[32];

    double start_time = get_time_sec();
    double last_print_time = start_time;

    while (!found) {
        // Only ONE key generation per million on CPU now!
        randombytes_buf(base_secret, 32);

        unsigned char h[64];
        crypto_hash_sha512(h, base_secret, 32);
        h[0] &= 248;
        h[31] &= 127;
        h[31] |= 64;

        ge_p3 base_p3;
        ge_scalarmult_base(&base_p3, h);

        convert_p3_to_gpu(&host_basepoint, &base_p3);

        memcpy(mappedBasepoint, &host_basepoint, basepoint_size);

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

        double current_time = get_time_sec();
        if (current_time - last_print_time >= 1.0) {
            double hps = total_checked / (current_time - start_time);
            printf("\rChecked %lu keys... %.2f H/s", total_checked, hps);
            fflush(stdout);
            last_print_time = current_time;
        }

        if (result_index != -1) {
            double final_time = get_time_sec();
            double hps = total_checked / (final_time - start_time);
            printf("\n\nFound match at index %d!\n", result_index);
            printf("Total keys checked: %lu\n", total_checked);
            printf("Average speed: %.2f H/s\n", hps);

            // To get the actual secret, we take our base scalar `h`
            // and add `result_index * 8` to it.
            // We use sc_muladd to safely add in the scalar field.
            unsigned char offset_scalar[32] = {0};
            uint32_t offset_val = result_index * 8;
            offset_scalar[0] = (offset_val >> 0) & 0xff;
            offset_scalar[1] = (offset_val >> 8) & 0xff;
            offset_scalar[2] = (offset_val >> 16) & 0xff;
            offset_scalar[3] = (offset_val >> 24) & 0xff;

            unsigned char final_scalar[32];
            // final_scalar = offset_scalar * 1 + h
            sc_muladd(final_scalar, offset_scalar, (const unsigned char*)"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", h);

            // We need the matching public key to save it in Tor format
            ge_p3 match_p3;
            ge_scalarmult_base(&match_p3, final_scalar);
            unsigned char match_pubkey[32];
            ge_p3_tobytes(match_pubkey, &match_p3);

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
                memcpy(expanded_sk, final_scalar, 32); // Lower 32 is the scalar
                memcpy(expanded_sk + 32, match_pubkey, 32); // Upper 32 is the pubkey

                fwrite(expanded_sk, 1, 64, f);
                fclose(f);
                printf("Secret key written to: %s\n", key_path);
            } else {
                printf("Failed to open %s for writing.\n", key_path);
            }
            found = true;
        }
    }

    vkUnmapMemory(device, basepointMemory);
    vkUnmapMemory(device, resultMemory);

    // Cleanup
    free(host_offsets);

    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyPipeline(device, computePipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyShaderModule(device, computeShaderModule, NULL);

    vkDestroyBuffer(device, basepointBuffer, NULL);
    vkFreeMemory(device, basepointMemory, NULL);
    vkDestroyBuffer(device, offsetsBuffer, NULL);
    vkFreeMemory(device, offsetsMemory, NULL);
    vkDestroyBuffer(device, prefixBuffer, NULL);
    vkFreeMemory(device, prefixMemory, NULL);
    vkDestroyBuffer(device, resultBuffer, NULL);
    vkFreeMemory(device, resultMemory, NULL);

    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}
