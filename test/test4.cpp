#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

using ordered_json = nlohmann::ordered_json;
namespace fs = std::filesystem;

// ====================== 数据结构 ======================
struct SeriesInfo {
  int series_num = 101;
  std::string series_desc = "test";
  std::string code_type = "hn";
  std::string input_dir = "C:/Users/Admin/Desktop/export-patient";
  std::string output_dir = "C:/Users/Admin/Desktop/cbct2ct";
};

// ====================== JSON 配置管理 ======================
class JsonConfig {
 public:
  static bool Read(const std::string& filename, SeriesInfo& info) {
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

  static bool Write(const std::string& filename, const SeriesInfo& info) {
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
      out << j.dump(2);
      return true;
    } catch (...) { return false; }
  }

  static std::string ToJsonString(const SeriesInfo& info) {
    ordered_json j;
    j["series_num"] = info.series_num;
    j["series_desc"] = info.series_desc;
    j["code_type"] = info.code_type;
    j["input_dir"] = info.input_dir;
    j["output_dir"] = info.output_dir;
    return j.dump(2);
  }
};

// ====================== 目录和进程工具类 ======================
class ProcessUtils {
 public:
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

  static bool IsDirectoryEmpty(const fs::path& dir) {
    try {
      if (!fs::exists(dir)) {
        return true;
      }
      return fs::is_empty(dir);
    } catch (...) { return false; }
  }

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

  static bool RunExe(const std::string& exe_path,
                     const std::vector<std::string>& args) {
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmdline = exe_path;
    for (const auto& arg : args) {
      cmdline += " " + arg;
    }

    std::cout << "cmdline: " << cmdline << std::endl;

    std::vector<char> cmd(cmdline.begin(), cmdline.end());
    cmd.push_back('\0');

    if (CreateProcessA(exe_path.c_str(), cmd.data(), NULL, NULL, FALSE, 0, NULL,
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

// ====================== CBCT 转 CT 工作流类 ======================
class CbctToCtWorkflow {
 public:
  explicit CbctToCtWorkflow(std::string exe_path,
                            std::string param_file = "param.json")
      : exe_path_(std::move(exe_path)), param_file_(std::move(param_file)) {}

  void SetSeriesInfo(const SeriesInfo& info) { info_ = info; }

  bool Run() {
    using Clock = std::chrono::steady_clock;
    auto t_start = Clock::now();

    // Step 1: 写 JSON 参数文件
    if (!JsonConfig::Write(param_file_, info_)) {
      std::cerr << "Failed to write param file: " << param_file_ << "\n";
      return false;
    }

    // Step 2: 准备输出目录
    fs::path dir = info_.output_dir;
    if (!ProcessUtils::PrepareDirectory(dir)) {
      return false;
    }

    if (!ProcessUtils::WaitUntilEmpty(dir, 5000)) {
      std::cerr << "Timeout waiting for empty directory\n";
      return false;
    }

    // Step 3: 运行外部 EXE
    std::vector<std::string> args = {"-s", "127.0.0.1:50051", "-m", "cbct2ct",
                                     "-p", param_file_};

    if (!ProcessUtils::RunExe(exe_path_, args)) {
      return false;
    }

    // Step 4: 检查输出
    if (ProcessUtils::IsDirectoryEmpty(dir)) {
      std::cerr << "Output directory is empty: " << dir << "\n";
      return false;
    }

    auto t_end = Clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start)
            .count();
    std::cout << "Workflow finished in " << elapsed_ms << " ms\n";

    return true;
  }

 private:
  SeriesInfo info_;
  std::string exe_path_;
  std::string param_file_;
};

// ====================== 示例 ======================
int main() {
  SeriesInfo info;
  info.series_num = 102;
  info.series_desc = "HN case";
  info.code_type = "hn";
  info.input_dir = "C:/Users/Admin/Desktop/export-patient";
  info.output_dir = "C:/Users/Admin/Desktop/cbct2ct";

  CbctToCtWorkflow workflow("D:\\icbct\\CBCT2CT-SDK\\algo-Client.exe",
                            "param_new.json");
  workflow.SetSeriesInfo(info);

  if (workflow.Run()) {
    std::cout << "CBCT → CT workflow succeeded\n";
  } else {
    std::cout << "CBCT → CT workflow failed\n";
  }

  return 0;
}
