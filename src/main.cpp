#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "cpp_generator.h"
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
		match (this) {
			Tree.Leaf(value): { return value; },
			Tree.Node(tree_node): {
				return tree_node.left?.sum() ?? 0 + tree_node.right?.sum() ?? 0;
			}
		}
	}

	fn increment_leaves(): void {
		match (this) {
			Tree.Leaf(value): {
				value = value + 1;
			},
			Tree.Node(tree_node): {
				tree_node.left?.increment_leaves();
				tree_node.right?.increment_leaves();
			}
		}
	}
}

fn main(): void {
	let tree: Tree = Tree.Node(
		TreeNode {
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

    source = "fn main(): void { if (true) { 1 + 3 * 4 / (5 + -variable && !other.something.that(1,2,3,4) + another.value); } else { 5; } }";

    std::unique_ptr<Scanner>
        scan = std::make_unique<TextScanner>(source);
    Parser parser = Parser(std::move(scan));

    auto stmt = parser.parse_stmt();
    auto visitor = CppGenerator();
    stmt->get()->accept(visitor);
    std::cout << std::endl;

    // while (stmt) {
    //     std::cout << &stmt << std::endl;
    //     stmt = parser.parse_stmt();
    // }

    return 0;
}
