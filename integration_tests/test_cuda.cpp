#include "test_cuda.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <iostream>

#ifdef _DEBUG
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

int test_leak() {
  std::cout << "Program start\n";

  // 故意泄漏
  int* p1 = new int(10);

  // 再制造一个数组泄漏
  int* p2 = new int[100];

  // 正常释放（对比用）
  int* p3 = new int(20);
  delete p3;

  std::cout << "Program end\n";

  // 检测内存泄漏
  int leaks = _CrtDumpMemoryLeaks();
  printf("Leaks: %d\n", leaks);
  return 0;
}

int main() {
  check_cuda();
  test_leak();
}