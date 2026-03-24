#include <gtest/gtest.h>

#include <cstring>
#include <sstream>
#include <string>

#include "test_cuda.h"

namespace {

cudaError_t FakeGetDeviceCountSuccess(int* count) {
  *count = 2;
  return cudaSuccess;
}

cudaError_t FakeGetDeviceCountFailure(int* /*count*/) {
  return cudaErrorInsufficientDriver;
}

cudaError_t FakeGetDevicePropertiesSuccess(cudaDeviceProp* prop, int device) {
  std::memset(prop, 0, sizeof(*prop));
  const std::string name = "Fake GPU " + std::to_string(device);
  std::strncpy(prop->name, name.c_str(), sizeof(prop->name) - 1);
  return cudaSuccess;
}

cudaError_t FakeGetDevicePropertiesFailOnSecond(cudaDeviceProp* prop,
                                                int device) {
  if (device == 1) {
    return cudaErrorInvalidDevice;
  }

  return FakeGetDevicePropertiesSuccess(prop, device);
}

TEST(CudaQueryTests, ReturnsInjectedDeviceList) {
  CudaRuntimeApi api;
  api.get_device_count = &FakeGetDeviceCountSuccess;
  api.get_device_properties = &FakeGetDevicePropertiesSuccess;

  const CudaQueryResult result = query_cuda_devices(api);

  EXPECT_EQ(result.status, cudaSuccess);
  EXPECT_EQ(result.device_count, 2);
  ASSERT_EQ(result.devices.size(), 2u);
  EXPECT_EQ(result.devices[0].index, 0);
  EXPECT_EQ(result.devices[0].name, "Fake GPU 0");
  EXPECT_EQ(result.devices[1].index, 1);
  EXPECT_EQ(result.devices[1].name, "Fake GPU 1");
}

TEST(CudaQueryTests, PropagatesCountFailure) {
  CudaRuntimeApi api;
  api.get_device_count = &FakeGetDeviceCountFailure;

  const CudaQueryResult result = query_cuda_devices(api);

  EXPECT_EQ(result.status, cudaErrorInsufficientDriver);
  EXPECT_EQ(result.device_count, 0);
  EXPECT_TRUE(result.devices.empty());
}

TEST(CudaQueryTests, StopsWhenReadingDevicePropertiesFails) {
  CudaRuntimeApi api;
  api.get_device_count = &FakeGetDeviceCountSuccess;
  api.get_device_properties = &FakeGetDevicePropertiesFailOnSecond;

  const CudaQueryResult result = query_cuda_devices(api);

  EXPECT_EQ(result.status, cudaErrorInvalidDevice);
  EXPECT_EQ(result.device_count, 2);
  ASSERT_EQ(result.devices.size(), 1u);
  EXPECT_EQ(result.devices[0].name, "Fake GPU 0");
}

TEST(CudaQueryTests, PrintsFormattedDeviceList) {
  CudaQueryResult result;
  result.status = cudaSuccess;
  result.device_count = 2;
  result.devices = {{0, "Alpha"}, {1, "Beta"}};

  std::ostringstream os;
  print_cuda_devices(result, os);

  EXPECT_EQ(os.str(), "GPU数量: 2\nGPU 0: Alpha\nGPU 1: Beta\n");
}

TEST(CudaQueryTests, PrintsErrorMessageOnFailure) {
  CudaQueryResult result;
  result.status = cudaErrorInvalidDevice;

  std::ostringstream os;
  print_cuda_devices(result, os);

  EXPECT_NE(os.str().find("CUDA error:"), std::string::npos);
}

}  // namespace