#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "parser.h"
#include "scanner.h"

int main() {
    std::string source = R"END(
struct TreeNode {
	left: int?;
	right: int?;
}

fn main(): void {
	let num: int = 1234;
	println(num);

	num.increment_leaves();
	
    while (true) {
        if (num > 0) {
            print("yay I got here\n");
        } else {
            print("nu uh\n");
        }

        for (let i: int = 0; i < 500; i = i + 1) {
            println(i);
        }
    }
}
)END";

    std::unique_ptr<Scanner> scan = std::make_unique<TextScanner>(source);
    Parser parser = Parser(std::move(scan));

    auto stmt = parser.parse_stmt();
    while (stmt) {
        std::cout << &stmt << std::endl;
        stmt = parser.parse_stmt();
    }
    return 0;
}
