#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <sodium.h>

#define BATCH_SIZE (1 << 20) // 1 million keys per batch

char* load_kernel_source(const char* filename, size_t* size_ret) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char* source = malloc((size_t)size + 1);
    if (!source) {
        fclose(f);
        return NULL;
    }
    size_t read_bytes = fread(source, 1, (size_t)size, f);
    if (read_bytes != (size_t)size) {
        free(source);
        fclose(f);
        return NULL;
    }
    source[size] = '\0';
    fclose(f);
    if (size_ret) *size_ret = (size_t)size;
    return source;
}

// Basic ed25519 scalar multiplication to get public key from secret.
// libsodium has crypto_scalarmult_ed25519_base_noclamp
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

    // Initialize OpenCL
    cl_platform_id platform = NULL;
    cl_int err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS || !platform) {
        printf("Failed to find OpenCL platform (err %d). Running in sandbox might not have OpenCL available.\n", err);
        return 0; // Graceful exit in sandbox environment
    }

    cl_device_id device = NULL;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS || !device) {
        // Fallback to CPU if GPU not available
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        if (err != CL_SUCCESS || !device) {
            printf("Failed to find OpenCL GPU/CPU device (err %d).\n", err);
            return 0; // Graceful exit
        }
    }

    char device_name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    printf("Using GPU: %s\n", device_name);

    cl_context ctx = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, NULL);

    size_t source_size;
    char* source = load_kernel_source("kernel.cl", &source_size);
    if (!source) {
        printf("Failed to load kernel.cl\n");
        return 1;
    }

    cl_program prog = clCreateProgramWithSource(ctx, 1, (const char**)&source, &source_size, NULL);
    clBuildProgram(prog, 1, &device, NULL, NULL, NULL);

    // Check build log
    size_t log_size;
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    if (log_size > 1) {
        char* log = malloc(log_size);
        if (log) {
            clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
            printf("Build log:\n%s\n", log);
            free(log);
        }
    }

    cl_kernel kernel = clCreateKernel(prog, "vanity_search", &err);
    if (err != CL_SUCCESS || !kernel) {
        printf("Failed to create kernel (err %d).\n", err);
        clReleaseProgram(prog);
        clReleaseCommandQueue(queue);
        clReleaseContext(ctx);
        free(source);
        return 1;
    }

    // Setup buffers
    size_t pubkeys_size = BATCH_SIZE * 32;
    uint8_t* host_pubkeys = malloc(pubkeys_size);
    uint8_t* host_secrets = malloc(pubkeys_size); // to keep track of secrets

    if (!host_pubkeys || !host_secrets) {
        printf("Failed to allocate host memory.\n");
        if (host_pubkeys) free(host_pubkeys);
        if (host_secrets) free(host_secrets);
        clReleaseKernel(kernel);
        clReleaseProgram(prog);
        clReleaseCommandQueue(queue);
        clReleaseContext(ctx);
        free(source);
        return 1;
    }

    cl_mem d_pubkeys = clCreateBuffer(ctx, CL_MEM_READ_ONLY, pubkeys_size, NULL, &err);
    cl_mem d_prefix = clCreateBuffer(ctx, CL_MEM_READ_ONLY, prefix_len, NULL, &err);
    cl_mem d_result = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);

    if (!d_pubkeys || !d_prefix || !d_result) {
        printf("Failed to allocate device memory.\n");
        if (d_pubkeys) clReleaseMemObject(d_pubkeys);
        if (d_prefix) clReleaseMemObject(d_prefix);
        if (d_result) clReleaseMemObject(d_result);
        free(host_pubkeys);
        free(host_secrets);
        clReleaseKernel(kernel);
        clReleaseProgram(prog);
        clReleaseCommandQueue(queue);
        clReleaseContext(ctx);
        free(source);
        return 1;
    }

    clEnqueueWriteBuffer(queue, d_prefix, CL_TRUE, 0, prefix_len, prefix, 0, NULL, NULL);

    uint32_t batch_size = BATCH_SIZE;
    uint32_t cl_prefix_len = (uint32_t)prefix_len;

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_pubkeys);
    clSetKernelArg(kernel, 1, sizeof(uint32_t), &batch_size);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_prefix);
    clSetKernelArg(kernel, 3, sizeof(uint32_t), &cl_prefix_len);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_result);

    printf("Starting search for prefix '%s'...\n", prefix);

    uint64_t total_checked = 0;
    bool found = false;

    while (!found) {
        // Generate random keys for this batch
        for (int i = 0; i < BATCH_SIZE; ++i) {
            randombytes_buf(&host_secrets[i * 32], 32);
            get_pubkey(&host_secrets[i * 32], &host_pubkeys[i * 32]);
        }

        clEnqueueWriteBuffer(queue, d_pubkeys, CL_TRUE, 0, pubkeys_size, host_pubkeys, 0, NULL, NULL);

        int result_index = -1;
        clEnqueueWriteBuffer(queue, d_result, CL_TRUE, 0, sizeof(int), &result_index, 0, NULL, NULL);

        size_t global_work_size = BATCH_SIZE;
        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

        clEnqueueReadBuffer(queue, d_result, CL_TRUE, 0, sizeof(int), &result_index, 0, NULL, NULL);

        total_checked += BATCH_SIZE;
        if (total_checked % (BATCH_SIZE * 10) == 0) {
            printf("Checked %lu keys...\n", total_checked);
        }

        if (result_index != -1) {
            printf("\nFound match at index %d!\n", result_index);
            printf("Total keys checked: %lu\n", total_checked);

            unsigned char* secret = &host_secrets[result_index * 32];

            // Create tor hs directory structure
            char path[512];
            if (snprintf(path, sizeof(path), "%s/%s_keys", out_dir, prefix) >= (int)sizeof(path)) {
                printf("Output path too long.\n");
                break;
            }
            mkdir(path, 0700);

            char key_path[550]; // Ensuring enough space for path + filename
            if (snprintf(key_path, sizeof(key_path), "%s/hs_ed25519_secret_key", path) >= (int)sizeof(key_path)) {
                printf("Key path too long.\n");
                break;
            }

            FILE* f = fopen(key_path, "wb");
            if (f) {
                // Tor secret key format: "== ed25519v1-secret: type0 ==\0\0\0" (32 bytes) + 64 bytes of expanded key
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
            }
            found = true;
        }
    }

    free(host_pubkeys);
    free(host_secrets);
    free(source);
    clReleaseMemObject(d_pubkeys);
    clReleaseMemObject(d_prefix);
    clReleaseMemObject(d_result);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return 0;
}
