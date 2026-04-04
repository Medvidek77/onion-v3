#include <stdio.h>
#include <CL/cl.h>

int main() {
    cl_platform_id platform = NULL;
    clGetPlatformIDs(1, &platform, NULL);
    if (!platform) {
        printf("No OpenCL platform found.\n");
        return 1;
    }
    printf("Platform found.\n");
    return 0;
}
