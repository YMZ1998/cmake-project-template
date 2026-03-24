#pragma once

#include <cuda_runtime.h>

#include <iosfwd>
#include <string>
#include <vector>

struct CudaDeviceInfo {
  int index = -1;
  std::string name;
};

struct CudaQueryResult {
  cudaError_t status = cudaSuccess;
  int device_count = 0;
  std::vector<CudaDeviceInfo> devices;
};

struct CudaRuntimeApi {
  cudaError_t (*get_device_count)(int* count) = &cudaGetDeviceCount;
  cudaError_t (*get_device_properties)(cudaDeviceProp* prop, int device) =
      &cudaGetDeviceProperties;
};

CudaQueryResult query_cuda_devices(const CudaRuntimeApi& api = {});
void print_cuda_devices(const CudaQueryResult& result, std::ostream& os);
int check_cuda();