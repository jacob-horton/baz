#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "parser.h"
#include "resolver.h"
#include "scanner.h"
#include "type_checker.h"

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

    source = "struct Name { prop1: int; prop2: bool; fn method1(arg1: float): int { return 5; } fn method2(): void {} }\n fn main(): void { for (let i: int = 0; i < 100; i = i + 1) { 1 + 3 * 4 / (5 + -variable && !other.something.that(1,2,3,4) + another.value); } if (true) { println(\"hi\"); } }";
    source = "fn main(): void { let some_int: int = 4; let some_float: float = 1.0; let final: float = some_float; final = final + 1.0; }";

    auto scan = std::make_unique<TextScanner>(source);
    Parser parser = Parser(std::move(scan));

    auto stmt = parser.parse_stmt();
    auto resolver = Resolver();
    auto type_checker = TypeChecker();

    // Resolve types
    stmt->get()->accept(resolver);

    try {
        stmt->get()->accept(type_checker);
        std::cout << "Succeeded" << std::endl;
    } catch (TypeCheckerError) {
        std::cout << "Failed" << std::endl;
    }

    // std::ofstream file("output.cpp");
    //
    // auto stmt = parser.parse_stmt();
    // auto visitor = CppGenerator(file);
    //
    // while (stmt) {
    //     stmt->get()->accept(visitor);
    //     file << std::endl;
    //     stmt = parser.parse_stmt();
    // }

    return 0;
}
