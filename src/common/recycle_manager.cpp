#include "recycle_manager.h"

#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

RecycleManager::RecycleManager(const fs::path& recycle_dir) {
  recycle_dir_ = recycle_dir;
  files_dir_ = recycle_dir / "files";
  metadata_file_ = recycle_dir / "metadata.txt";

  fs::create_directories(files_dir_);
  if (!fs::exists(metadata_file_)) {
    std::ofstream(metadata_file_);
  }
}

std::string RecycleManager::generate_id() {
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_int_distribution<int> dist(0, 15);

  std::stringstream ss;

  for (int i = 0; i < 16; i++)
    ss << std::hex << dist(rng);

  return ss.str();
}

bool RecycleManager::remove(const fs::path& file) {
  std::cout << "Delete file: " << file << "\n";
  if (!fs::exists(file))
    return false;

  std::string id = generate_id();

  fs::path target = files_dir_ / id;

  fs::rename(file, target);

  RecycleItem item;

  item.id = id;
  item.original_path = file.string();
  item.original_name = file.filename().string();
  item.delete_time = std::time(nullptr);

  append_metadata(item);

  return true;
}

void RecycleManager::append_metadata(const RecycleItem& item) {
  std::ofstream file(metadata_file_, std::ios::app);
  if (file.is_open()) {
    file << "\n";
    file << item.id << "|" << item.original_path << "|" << item.original_name
         << "|" << item.delete_time << "\n";
    file.close();
  } else {
    std::cerr << "Unable to open file: " << metadata_file_ << std::endl;
  }
}

std::vector<RecycleItem> RecycleManager::load_metadata() {
  std::vector<RecycleItem> items;

  std::ifstream in(metadata_file_);

  std::string line;

  while (std::getline(in, line)) {

    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string token;

    RecycleItem item;

    if (!std::getline(ss, item.id, '|'))
      continue;

    if (!std::getline(ss, item.original_path, '|'))
      continue;

    if (!std::getline(ss, item.original_name, '|'))
      continue;

    if (!std::getline(ss, token, '|'))
      continue;

    try {
      item.delete_time = std::stoll(token);
    } catch (...) { continue; }

    items.push_back(item);
  }

  return items;
}

void RecycleManager::save_metadata(const std::vector<RecycleItem>& items) {
  std::ofstream out(metadata_file_);

  for (const auto& item : items) {
    out << item.id << "|" << item.original_path << "|" << item.original_name
        << "|" << item.delete_time << "\n";
  }
}

bool RecycleManager::restore(const std::string& id) {
  auto items = load_metadata();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->id == id) {
      fs::path src = files_dir_ / id;
      fs::path dst = it->original_path;

      fs::create_directories(dst.parent_path());

      fs::rename(src, dst);

      items.erase(it);

      save_metadata(items);

      return true;
    }
  }

  return false;
}

std::vector<RecycleItem> RecycleManager::list() {
  return load_metadata();
}

void RecycleManager::clean(int days) {
  auto items = load_metadata();

  std::vector<RecycleItem> remain;

  auto now = std::time(nullptr);

  for (const auto& item : items) {
    double diff = std::difftime(now, item.delete_time);

    if (diff > days * 86400) {
      fs::remove(files_dir_ / item.id);
    } else {
      remain.push_back(item);
    }
  }

  save_metadata(remain);
}