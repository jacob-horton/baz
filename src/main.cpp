#include <iostream>
#include <optional>
#include <ostream>
#include <string>

#include "scanner.h"
#include "token.h"

int main() {
  std::string source = "if (1234 == 0.9) {\nlet some_identifier1 = 1234\n}";
  Scanner scan = Scanner(source.c_str());

  std::optional<Token> next = scan.scan_token();
  while (next.has_value()) {
    std::cout << next.value() << std::endl << std::flush;
    next = scan.scan_token();
  }
  return 0;
}
