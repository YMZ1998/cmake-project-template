#include "common.h"

#ifdef _WIN32
#include <windows.h>
#endif

std::filesystem::path GetExePath() {
#ifdef _WIN32
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return std::filesystem::path(path).parent_path();
#else
  return std::filesystem::current_path();
#endif
}
