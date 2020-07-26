#include <iostream>
#include <string>

#include "gdwg/graph.hpp"
#include <concepts/concepts.hpp>

auto main() -> int {
	auto g = gdwg::graph<std::string, int>{};
	g.insert_node("hello");
	using graph = gdwg::graph<std::string, int>;
	auto const v = std::vector<std::string>{"hello", "yes", "end"};

	auto k = graph(v.begin(), v.end());
	if (!k.insert_node("KO")) {
		std::cout << "fail insert" << std::endl;
	}
	else {
		std::cout << "success insert" << std::endl;
	}
	if (!k.insert_node("KO")) {
		std::cout << "yes, already a node" << std::endl;
	}
	else {
		std::cout << "no, and add the node" << std::endl;
	}

	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you?");

	g.insert_edge("hello", "how", 5);
	// g.insert_edge("hello", "are", 8);
	// g.insert_edge("hello", "are", 2);

	// g.insert_edge("how", "you?", 1);
	// g.insert_edge("how", "hello", 4);

	// g.insert_edge("are", "you?", 3);

	// std::cout << g << "\n";

	// auto g2 = gdwg::graph<std::string, int>{g};

	// std::cout << g2 << "\n";

	// // This is a structured binding.
	// // https://en.cppreference.com/w/cpp/language/structured_binding
	// // It allows you to unpack your tuple.
	// for (auto const& [from, to, weight] : g) {
	// 	std::cout << from << " -> " << to << " (weight " << weight << ")\n";
	// }
	auto a = 1;
	return a;
}
