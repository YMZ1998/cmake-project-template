#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using ordered_json = nlohmann::ordered_json;

struct SeriesInfo {
  int series_num;
  std::string series_desc;
  std::string code_type;
  std::string input_dir;
  std::string output_dir;
};

class JsonConfig {
 public:
  // ���ļ���ȡ JSON������˳��
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

  // д�� JSON �ļ�������˳��
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
      out << j.dump(2);  // 4 �ո�����
      return true;
    } catch (...) { return false; }
  }

  // ת JSON �ַ���������˳��
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

int main() {
  SeriesInfo info;

  // ��ȡ JSON
  if (JsonConfig::read("param.json", info)) {
    std::cout << "JSON content (ordered):\n";
    std::cout << JsonConfig::to_json_string(info) << std::endl;
  } else {
    std::cerr << "Failed to read JSON\n";
  }

  // �޸ĺ�д��
  info.series_num += 1;
  if (JsonConfig::write("param_new.json", info)) {
    std::cout << "JSON written successfully\n";
  }

  return 0;
}
