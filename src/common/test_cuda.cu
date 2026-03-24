#include "test_cuda.h"

#include <iostream>

CudaQueryResult query_cuda_devices(const CudaRuntimeApi& api) {
  CudaQueryResult result;

  const cudaError_t count_status = api.get_device_count(&result.device_count);
  if (count_status != cudaSuccess) {
    result.status = count_status;
    result.device_count = 0;
    return result;
  }

  result.devices.reserve(result.device_count);
  for (int index = 0; index < result.device_count; ++index) {
    cudaDeviceProp prop{};
    const cudaError_t prop_status = api.get_device_properties(&prop, index);
    if (prop_status != cudaSuccess) {
      result.status = prop_status;
      return result;
    }

    result.devices.push_back({index, prop.name});
  }

  return result;
}

void print_cuda_devices(const CudaQueryResult& result, std::ostream& os) {
  if (result.status != cudaSuccess) {
    os << "CUDA error: " << cudaGetErrorString(result.status) << std::endl;
    return;
  }

  os << "GPU: " << result.device_count << std::endl;
  for (const auto& device : result.devices) {
    os << "GPU " << device.index << ": " << device.name << std::endl;
  }
}

int check_cuda() {
  const CudaQueryResult result = query_cuda_devices();
  print_cuda_devices(result, std::cout);

  if (result.status != cudaSuccess) {
    return -1;
  }

  return result.device_count;
}