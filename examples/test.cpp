#include <windows.h>
#include <iostream>
#include <taskflow.hpp>  // Taskflow is header-only
#include "argparse.hpp"
#include "common.h"
#include "json.hpp"
#include "logger.hpp"
#include "benchmark/benchmark.h"

int main1(int argc, char* argv[]) {
  argparse::ArgumentParser program("program_name");

  program.add_argument("square")
      .help("display the square of a given integer")
      .scan<'i', int>();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  auto input = program.get<int>("square");
  std::cout << (input * input) << std::endl;

  return 0;
}

void test_spdlog() {
  spdlog::info("Welcome to spdlog!");
  spdlog::error("Some error message with arg: {}", 1);

  spdlog::warn("Easy padding in numbers like {:08d}", 12);
  spdlog::critical(
      "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
  spdlog::info("Support for floats {:03.2f}", 1.23456);
  spdlog::info("Positional args are {1} {0}..", "too", "supported");
  spdlog::info("{:<30}", "left aligned");

  spdlog::set_level(spdlog::level::debug);  // Set global log level to debug
  spdlog::debug("This message should be displayed..");

  // change log pattern
  spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

  // Compile time log levels
  // Note that this does not change the current log level, it will only
  // remove (depending on SPDLOG_ACTIVE_LEVEL) the call on the release code.
  SPDLOG_TRACE("Some trace message with param {}", 42);
  SPDLOG_DEBUG("Some debug message");
}

int test_wlog() {
  using namespace wlog;

  auto log_dir = GetExePath() / "logs";
  std::filesystem::create_directories(log_dir);

  auto log_file = log_dir / "test.log";
  std::cout << "Log file path: " << log_file << std::endl;

  if (!logger::get().init(log_file.string())) {
    std::cerr << "Failed to init logger" << std::endl;
    return 1;
  }

  STM_DEBUG() << "STM_DEBUG " << 1;
  PRINT_WARN("PRINT_WARN %d", 1);
  LOG_INFO("LOG_INFO {}", 1);

  logger::get().set_level(spdlog::level::info);
  STM_DEBUG() << "STM_DEBUG " << 2;
  PRINT_WARN("PRINT_WARN %d", 2);
  LOG_INFO("LOG_INFO {}", 2);

  logger::get().shutdown();
  return 0;
}

int test_taskflow() {

  tf::Executor executor;
  tf::Taskflow taskflow;

  auto [A, B, C, D] = taskflow.emplace(  // create four tasks
      []() { std::cout << "TaskA\n"; }, []() { std::cout << "TaskB\n"; },
      []() { std::cout << "TaskC\n"; }, []() { std::cout << "TaskD\n"; });

  A.precede(B, C);  // A runs before B and C
  D.succeed(B, C);  // D runs after  B and C

  executor.run(taskflow).wait();

  return 0;
}

int test_json() {
  using json = nlohmann::json;

  json person = {{"name", "Alice"},
                 {"age", 30},
                 {"is_student", false},
                 {"skills", {"C++", "Python", "CUDA"}}};

  std::cout << "Original JSON:\n" << person.dump(4) << std::endl;

  person["age"] = 31;
  person["skills"].push_back("Machine Learning");

  std::string name = person["name"];
  int age = person["age"];
  bool is_student = person["is_student"];

  std::cout << "\nUpdated name: " << name << "\nUpdated age: " << age
            << "\nIs student? " << std::boolalpha << is_student << std::endl;

  std::string json_str = person.dump();

  json parsed = json::parse(json_str);
  std::cout << "\nParsed from string:\n" << parsed.dump(2) << std::endl;

  return 0;
}

//int main(int argc, char** argv) {
//  std::cout << "Args:\n";
//  for (int i = 0; i < argc; ++i) {
//    std::cout << "  argv[" << i << "] = " << argv[i] << std::endl;
//  }
//  benchmark::Initialize(&argc, argv);
//  benchmark::RunSpecifiedBenchmarks();
//}

void SomeFunction() {
  std::vector<double> v(1000);
  for (int i = 0; i < 1000; ++i) {
    v[i] = std::sin(i) * std::cos(i);
  }
}

static void BM_SomeFunction(benchmark::State& state) {
  for (auto _ : state) {
    SomeFunction();
  }
}

BENCHMARK(BM_SomeFunction);
BENCHMARK_MAIN();
