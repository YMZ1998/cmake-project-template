#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "recycle_manager.h"

namespace fs = std::filesystem;

namespace {

fs::path MakeUniqueTempPath() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  return fs::temp_directory_path() / ("cmake_project_template_" + std::to_string(ticks));
}

class RecycleManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    workspace_ = MakeUniqueTempPath();
    fs::create_directories(workspace_);
  }

  void TearDown() override {
    std::error_code error;
    fs::remove_all(workspace_, error);
  }

  fs::path CreateFile(const std::string& relative_path,
                      const std::string& content = "payload") {
    const fs::path file_path = workspace_ / relative_path;
    fs::create_directories(file_path.parent_path());
    std::ofstream out(file_path);
    out << content;
    return file_path;
  }

  fs::path recycle_root() const { return workspace_ / "recycle_bin"; }

  fs::path workspace_;
};

TEST_F(RecycleManagerTest, RemoveListAndRestoreRoundTrip) {
  RecycleManager manager(recycle_root());
  const fs::path source_file = CreateFile("data/sample.txt", "hello");

  ASSERT_TRUE(manager.remove(source_file));
  EXPECT_FALSE(fs::exists(source_file));

  const auto items = manager.list();
  ASSERT_EQ(items.size(), 1u);
  EXPECT_EQ(items.front().original_name, "sample.txt");
  EXPECT_EQ(items.front().original_path, source_file.string());
  EXPECT_TRUE(fs::exists(recycle_root() / "files" / items.front().id));

  EXPECT_TRUE(manager.restore(items.front().id));
  EXPECT_TRUE(fs::exists(source_file));
  EXPECT_TRUE(manager.list().empty());
}

TEST_F(RecycleManagerTest, RemoveMissingFileReturnsFalse) {
  RecycleManager manager(recycle_root());

  EXPECT_FALSE(manager.remove(workspace_ / "missing.txt"));
  EXPECT_TRUE(manager.list().empty());
}

TEST_F(RecycleManagerTest, CleanRemovesExpiredEntriesAndKeepsRecentOnes) {
  RecycleManager manager(recycle_root());

  const fs::path files_dir = recycle_root() / "files";
  fs::create_directories(files_dir);

  std::ofstream(files_dir / "expired-id") << "old payload";
  std::ofstream(files_dir / "recent-id") << "new payload";

  const std::time_t now = std::time(nullptr);
  std::ofstream metadata(recycle_root() / "metadata.txt");
  metadata << "expired-id|C:/tmp/expired.txt|expired.txt|"
           << (now - 3 * 86400) << "\n";
  metadata << "recent-id|C:/tmp/recent.txt|recent.txt|" << now << "\n";
  metadata.close();

  manager.clean(1);

  EXPECT_FALSE(fs::exists(files_dir / "expired-id"));
  EXPECT_TRUE(fs::exists(files_dir / "recent-id"));

  const auto items = manager.list();
  ASSERT_EQ(items.size(), 1u);
  EXPECT_EQ(items.front().id, "recent-id");
}

}  // namespace
