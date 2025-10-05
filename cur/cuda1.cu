
#include <cuda_runtime.h>
#include <stdio.h>

__global__ void helloFromGPU() { printf("Hello World from GPU!\n"); }

int main() {
    // Launch kernel with 1 block and 1 thread
    helloFromGPU<<<1, 1>>>();

    // Wait for GPU to finish before accessing on host
    cudaDeviceSynchronize();

    printf("Hello World from CPU in main function!\n");
    return 0;
}
