#include <iostream>
#include "recycle_manager.h"

int main() {
  RecycleManager rm("D:\\tmp");

  rm.remove("D:\\tmp\\output.txt");

  auto items = rm.list();

  for (const auto& i : items) {
    std::cout << "ID: " << i.id << "\n";
    std::cout << "Original: " << i.original_path << "\n";
  }

  if (!items.empty()) {
    std::cout << "Restore first file\n";
    rm.restore(items[0].id);
  }

  rm.clean(30);

  return 0;
}