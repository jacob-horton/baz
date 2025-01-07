#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "parser.h"
#include "scanner.h"

int main() {
    std::string source = "struct Outer { fn func_name(jeff: int, bill: float,): str { 1 + 2 * false - (1.5 / 6 * id) ?? 5; 10; } prop: int; }";
    std::unique_ptr<Scanner> scan = std::make_unique<TextScanner>(source);
    Parser parser = Parser(std::move(scan));

    std::unique_ptr<Stmt> stmt = parser.parse_stmt();
    std::cout << &stmt << std::endl;
    return 0;
}
