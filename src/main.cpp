#include "file_reader.hpp"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std::chrono;
int main(int argc, char* argv[]) {
    std::cout << "Hello, World!" << std::endl;
  std::filesystem::path current_path = std::filesystem::current_path();
  std::string file("/home/ezio/ClassProject3/adaptec1");
  std::cout << "Reading file: " << file << std::endl;
  
  auto fr = std::make_shared<FileReader>(file);
  fr->test_print();
  fr->get_pdata()->print_info();
  std::cout << "File read successfully!" << std::endl;

  return 0;
}
