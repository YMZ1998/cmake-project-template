#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>

void run_exe() {
  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof(si);

  if (CreateProcessA("D:\\code\\cmake-project-template\\output\\Release\\test2.exe",
                     NULL,                                  // �����в���
                     NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    std::cout << "Process started!\n";

    // �ȴ����̽���
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
  }
}

void run_exe2() {
  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof(si);

  // exe ·��
  std::string exe_path =
      "D:\\icbct\\CBCT2CT-SDK\\algo-Client.exe";

  // �𿪵Ĳ���
  std::vector<std::string> args = {"-s", "127.0.0.1:50051", "-m", "cbct2ct",
                                   "-p", "param.json"};

  // ���������������ַ�����ÿ�����������Ű�������֧�ֿո�
  std::string cmdline = "\"" + exe_path + "\"";
  for (const auto& a : args) {
    cmdline += " \"" + a + "\"";
  }

  // Windows CreateProcess Ҫ�� lpCommandLine �ǿ��޸ĵ� char*
  std::vector<char> cmd(cmdline.begin(), cmdline.end());
  cmd.push_back('\0');  // null-terminated

  if (CreateProcessA(exe_path.c_str(),  // lpApplicationName
                     cmd.data(),        // lpCommandLine
                     NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    std::cout << "Process started!\n";

    // �ȴ����̽���
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
  }
}


int main() {
  // algo-Client.exe -s 127.0.0.1:50051 -m cbct2ct -p param.json
  run_exe2();

  //std::thread t(run_exe);
  //t.detach();  // ���� join()
  //std::cout << "Main thread continues...\n";
  //Sleep(5000);
  return 0;
}
