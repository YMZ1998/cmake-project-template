#pragma once
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

std::filesystem::path GetExePath() {
#ifdef _WIN32
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return std::filesystem::path(path).parent_path();
#else
  // Linux/macOS ���� readlink("/proc/self/exe", ...)
  return std::filesystem::current_path();  // �����������ƽ̨��ʵ��
#endif
}