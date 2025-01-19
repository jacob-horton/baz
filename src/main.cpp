#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "parser.h"
#include "scanner.h"

int main() {
    //     std::string source = R"END(
    // struct TreeNode {
    // 	left: int?;
    // 	right: int?;
    // }
    //
    // fn main(): void {
    //     let num: int = 1234;
    //     println(num);
    //
    //     num.increment_leaves();
    //
    //     while (true) {
    //         if (num > 0) {
    //             print("yay I got here\n");
    //         } else {
    //             print("nu uh\n");
    //         }
    //
    //         for (let i: int = 0; i < 500; i = i + 1) {
    //             println(i);
    //         }
    //     }
    // }
    // )END";

    std::string source = R"END(
struct TreeNode {
	left: Tree?;
	right: Tree?;
}

enum Tree {
	Leaf(int);
	Node(TreeNode);

	fn sum(): int {
	}

	fn increment_leaves(): void {
	}
}

fn main(): void {
	let tree: Tree = Tree.Node(
		TreeNode{
			left: Tree.Leaf(1),
			right: Tree.Node(
				TreeNode {
					left: Tree.Leaf(2),
					right: null,
				}
			),
		}
	);

	println(tree.sum());

	tree.increment_leaves();
	println(tree.sum());
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
