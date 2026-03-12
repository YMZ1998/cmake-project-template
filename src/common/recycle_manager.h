#pragma once
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct RecycleItem {
  std::string id;
  std::string original_path;
  std::string original_name;
  std::time_t delete_time;
};

class RecycleManager {
 public:
  RecycleManager(const fs::path& recycle_dir);

  bool remove(const fs::path& file);
  bool restore(const std::string& id);

  std::vector<RecycleItem> list();

  void clean(int days);

 private:
  fs::path recycle_dir_;
  fs::path files_dir_;
  fs::path metadata_file_;

  std::string generate_id();

  void append_metadata(const RecycleItem& item);
  std::vector<RecycleItem> load_metadata();
  void save_metadata(const std::vector<RecycleItem>& items);
};