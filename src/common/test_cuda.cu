#include <cuda_runtime.h>
#include <iostream>

int check_cuda() {
  int count;
  cudaGetDeviceCount(&count);
  std::cout << "GPUÊýÁ¿: " << count << std::endl;

  for (int i = 0; i < count; i++) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);
    std::cout << "GPU " << i << ": " << prop.name << std::endl;
  }
  return count;
}

