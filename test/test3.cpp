#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool prepare_directory(const fs::path& dir) {
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

void wait_until_empty(const fs::path& dir, int timeout_ms = 5000) {
  auto start = std::chrono::steady_clock::now();
  while (!fs::is_empty(dir)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count() > timeout_ms)
      break;
  }
  std::cout << "Directory is empty: " << dir << "\n";
}

bool is_directory_empty(const fs::path& dir) {
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

void run_exe(const std::string& exe_path, std::vector<std::string>& args) {
  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof(si);

  std::string cmdline = "\"" + exe_path + "\"";
  for (const auto& a : args) {
    cmdline += " " + a;
  }

  std::cout << "cmdline : " << cmdline << "\n";

  std::vector<char> cmd(cmdline.begin(), cmdline.end());
  cmd.push_back('\0');  // null-terminated

  if (CreateProcessA(exe_path.c_str(), cmd.data(), NULL, NULL, FALSE, 0, NULL,
                     NULL, &si, &pi)) {
    std::cout << "Process started!\n";

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
  }
}

int main() {
  fs::path dir = "C:/Users/Admin/Desktop/cbct2ct";

  if (prepare_directory(dir)) {
    std::cout << "Directory ready: " << dir << "\n";
  } else {
    std::cerr << "Failed to prepare directory: " << dir << "\n";
  }

  wait_until_empty(dir, 5000);
  // algo-Client.exe -s 127.0.0.1:50051 -m cbct2ct -p param.json

  // exe Â·¾¶
  std::string exe_path = "D:\\icbct\\CBCT2CT-SDK\\algo-Client.exe";
  //std::string exe_path = "./test2.exe";
  std::vector<std::string> args = {"-s", "127.0.0.1:50051", "-m", "cbct2ct",
                                   "-p", "param.json"};
  run_exe(exe_path, args);

  if (is_directory_empty(dir)) {
    std::cout << "Directory is empty: " << dir << "\n";
  } else {
    std::cout << "Directory is not empty: " << dir << "\n";
  }
  return 0;
}
