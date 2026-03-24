#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

using ordered_json = nlohmann::ordered_json;

struct SeriesInfo {
  int series_num = 101;
  std::string series_desc = "test";
  std::string code_type = "hn";
  std::string input_dir = "C:/Users/Admin/Desktop/export-patient";
  std::string output_dir = "C:/Users/Admin/Desktop/cbct2ct";
};

class JsonConfig {
 public:
  // 从文件读取 JSON（保持顺序）
  static bool read(const std::string& filename, SeriesInfo& info) {
    try {
      std::ifstream in(filename);
      if (!in.is_open())
        return false;

      ordered_json j;
      in >> j;

      info.series_num = j.value("series_num", 0);
      info.series_desc = j.value("series_desc", "");
      info.code_type = j.value("code_type", "");
      info.input_dir = j.value("input_dir", "");
      info.output_dir = j.value("output_dir", "");

      return true;
    } catch (...) { return false; }
  }

  // 写入 JSON 文件（保持顺序）
  static bool write(const std::string& filename, const SeriesInfo& info) {
    try {
      ordered_json j;
      j["series_num"] = info.series_num;
      j["series_desc"] = info.series_desc;
      j["code_type"] = info.code_type;
      j["input_dir"] = info.input_dir;
      j["output_dir"] = info.output_dir;

      std::ofstream out(filename);
      if (!out.is_open())
        return false;
      out << j.dump(2);  // 4 空格缩进
      return true;
    } catch (...) { return false; }
  }

  // 转 JSON 字符串（保持顺序）
  static std::string to_json_string(const SeriesInfo& info) {
    ordered_json j;
    j["series_num"] = info.series_num;
    j["series_desc"] = info.series_desc;
    j["code_type"] = info.code_type;
    j["input_dir"] = info.input_dir;
    j["output_dir"] = info.output_dir;
    return j.dump(4);
  }
};

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

    //char commandLine[1024];
    //sprintf_s(commandLine, "%s -s %s -m %s -p %s", exePath.c_str(), "127.0.0.1:50051",
    //          "cbct2ct",
    //          "param.json");

    std::string cmdline = exePath;
    for (const auto& a : args) {
      cmdline += " " + a;
    }

    std::cout << "cmdline: " << cmdline << std::endl;

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
  SeriesInfo info;
  std::cout << JsonConfig::to_json_string(info) << std::endl;

  info.series_num += 1;
  info.code_type = "hn";
  info.input_dir = "C:/Users/Admin/Desktop/export-patient";
  if (JsonConfig::write("param_new.json", info)) {
    std::cout << JsonConfig::to_json_string(info) << std::endl;
    std::cout << "JSON written successfully\n";
  }

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
                                   "-p", "param_new.json"};

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
