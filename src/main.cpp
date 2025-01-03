#include <iostream>
#include <ostream>
#include <string>

#include "parser.h"
#include "scanner.h"

int main() {
    std::string source = "struct Outer { fn func_name(jeff: int, bill: float,): str { 1 + 2 * false - (1.5 / 6 * id) ?? 5; 10; } prop: int; }";
    Scanner scan = Scanner(source);
    Parser parser = Parser(scan);

    Stmt *stmt = parser.parse_stmt();
    std::cout << stmt << std::endl;
    return 0;
}
