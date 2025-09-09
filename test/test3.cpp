#include <windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

class ProcessHelper {
 public:
  // 清空或创建目录
  static bool PrepareDirectory(const fs::path& dir) {
    try {
      if (fs::exists(dir)) {
        for (auto& entry : fs::directory_iterator(dir)) {
          fs::remove_all(entry);
        }
      } else {
        fs::create_directories(dir);
      }
      return true;
    } catch (const fs::filesystem_error& e) {
      std::cerr << "Filesystem error: " << e.what() << "\n";
      return false;
    }
  }

  // 检查目录是否为空
  static bool IsDirectoryEmpty(const fs::path& dir) {
    try {
      if (!fs::exists(dir)) {
        std::cerr << "Directory does not exist: " << dir << "\n";
        return true;
      }
      return fs::is_empty(dir);
    } catch (const fs::filesystem_error& e) {
      std::cerr << "Filesystem error: " << e.what() << "\n";
      return false;
    }
  }

  // 等待目录变空（带超时）
  static bool WaitUntilEmpty(const fs::path& dir, int timeout_ms = 5000) {
    auto start = std::chrono::steady_clock::now();
    while (!IsDirectoryEmpty(dir)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      auto now = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
              .count() > timeout_ms) {
        return false;
      }
    }
    return true;
  }

  // 运行外部 exe
  static bool RunExe(const std::string& exePath,
                     const std::vector<std::string>& args) {
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmdline = exePath;
    for (const auto& a : args) {
      cmdline += " " + a;
    }

    std::vector<char> cmd(cmdline.begin(), cmdline.end());
    cmd.push_back('\0');  // null-terminated

    if (CreateProcessA(exePath.c_str(), cmd.data(), NULL, NULL, FALSE, 0, NULL,
                       NULL, &si, &pi)) {
      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return true;
    } else {
      std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
      return false;
    }
  }
};

// ====================== 使用示例 ======================

int main() {
  using Clock = std::chrono::steady_clock;
  auto t_start = Clock::now();

  fs::path dir = "C:/Users/Admin/Desktop/cbct2ct";

  if (ProcessHelper::PrepareDirectory(dir)) {
    std::cout << "Directory ready: " << dir << "\n";
  }

  if (!ProcessHelper::WaitUntilEmpty(dir, 5000)) {
    std::cout << "Timeout waiting for empty directory: " << dir << "\n";
  }

  std::string exePath = "D:\\icbct\\CBCT2CT-SDK\\algo-Client.exe";
  std::vector<std::string> args = {"-s", "127.0.0.1:50051", "-m", "cbct2ct",
                                   "-p", "param.json"};

  if (ProcessHelper::RunExe(exePath, args)) {
    std::cout << "Process finished!\n";
  }

  if (ProcessHelper::IsDirectoryEmpty(dir)) {
    std::cout << "Directory is empty: " << dir << "\n";
  } else {
    std::cout << "Directory is not empty: " << dir << "\n";
  }

  auto t_end = Clock::now();
  auto elapsed_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start)
          .count();
  std::cout << "Total elapsed time: " << elapsed_ms << " ms\n";

  return 0;
}
