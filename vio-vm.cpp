/**
 * Vio VM executable.
 */

#include <fstream>
#include <iostream>
#include <string>

// #include "src/Logger.h"
// #include "src/vm/VioValue.h"
#include "src/vm/VioVM.h"

void printHelp() {
  std::cout << "\nUsage: Vio-vm [options]\n\n"
            << "Options:\n"
            << "    -e, --expression  Expression to parse\n"
            << "    -f, --file        File to parse\n\n";
}

/**
 * Vio VM main executable.
 */
int main(int argc, char const *argv[]) {
  VioVM vm;

  if (argc != 3) {
    printHelp();
    return 0;
  }

  /**
   * Expression mode.
   */
  std::string mode = argv[1];

  /**
   * Program to execute.
   */
  std::string program;

  /**
   * Simple expression.
   */
  if (mode == "-e") {
    program = argv[2];
  }

  /**
   * Vio file.
   */
  else if (mode == "-f") {
    // Read the file:
    std::ifstream programFile(argv[2]);
    std::stringstream buffer;
    buffer << programFile.rdbuf() << "\n";

    // Program:
    program = buffer.str();
  }

  // auto result = vm.exec(R"(
  //   // (def square (x) (* x x))
  //   // (square 2)
  //   (def factorial (x)
  //     (if (== x 1)
  //       1
  //       (* x (factorial (- 1 x)))))
  //   (factorial 5)
  // )");

  // auto result = vm.exec(R"(

  //   (var x 5)
  //   (set x (+ x 10))
  //   x
  //   (begin 
  //     (var z 100)
  //     (begin
  //       (var z 200)
  //       (set x 700)
  //       x)
  //     x)
  //   x
  // )");
  auto result = vm.exec(program);
  std::cout << "\n";
  // log(AS_CPPSTRING(result));
  log(result);
  std::cout << "All done!\n";
  

  return 0;

  // /**
  //  * VM instance.
  //  */
  // VioVM vm;

  // /**
  //  * Violuation result.
  //  */
  // auto result = vm.exec(program);

  // std::cout << "\n";
  // log(result);
  // std::cout << "\n";

  // return 0;
}