#include <iostream>
#include <ostream>
#include <string>

#include "parser.h"
#include "scanner.h"

int main() {
  std::string source = "fn func_name(jeff: int): str { 1 + 2 * false - (1.5 / "
                       "6 * id) ?? 5; 10; }";
  Scanner scan = Scanner(source.c_str());
  Parser parser = Parser(scan);

  Stmt *stmt = parser.parse_stmt();
  std::cout << stmt << std::endl << std::flush;
  return 0;
}
