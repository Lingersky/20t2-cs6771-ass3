#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "gdwg/graph.hpp"
#include <concepts/concepts.hpp>

auto main() -> int {
	using graph2 = gdwg::graph<int, int>;
	auto const vt = std::vector<graph2::value_type>{
	   {4, 1, -4},
	   {3, 2, 2},
	   {2, 4, 2},
	   {2, 1, 1},
	   {6, 2, 5},
	   {6, 3, 10},
	   {1, 5, -1},
	   {3, 6, -8},
	   {4, 5, 3},
	   {5, 2, 7},
	};

	auto h = graph2(vt.begin(), vt.end());
	// h.change_struct_inner_value(2, 2, 10);
}

#if 0
auto main() -> int {
	using graph = gdwg::graph<std::string, int>;
	auto g = graph{};
	g.insert_node("how");
	if (!g.insert_node("how")) {
		std::cout << "fail insert" << std::endl;
	}
	else {
		std::cout << "success insert" << std::endl;
	}

	auto const v = std::vector<std::string>{"hello", "yes", "end"};
	auto k = graph(v.begin(), v.end());
	if (!k.insert_node("KO")) {
		std::cout << "fail insert" << std::endl;
	}
	else {
		std::cout << "success insert" << std::endl;
	}
	if (!k.insert_node("KO")) {
		std::cout << "fail insert, already a node" << std::endl;
	}
	else {
		std::cout << "success insert, and add the node" << std::endl;
	}

	g.insert_node("hello");
	g.insert_node("are");
	g.insert_node("you?");

	if (!g.insert_edge("hello", "how", 5)) {
		std::cout << "fail insert" << std::endl;
	}
	else {
		std::cout << "success insert" << std::endl;
	}
	if (!g.insert_edge("hello", "how", 5)) {
		std::cout << "fail insert, already an edge" << std::endl;
	}
	else {
		std::cout << "success insert, and add the edge" << std::endl;
	}
	g.insert_edge("hello", "are", 8);
	g.insert_edge("hello", "are", 2);
	g.insert_edge("how", "you?", 1);
	g.insert_edge("how", "hello", 4);
	g.insert_edge("are", "you?", 3);
	using graph2 = gdwg::graph<int, int>;
	auto const vt = std::vector<graph2::value_type>{
	   {4, 1, -4},
	   {3, 2, 2},
	   {2, 4, 2},
	   {2, 1, 1},
	   {6, 2, 5},
	   {6, 3, 10},
	   {1, 5, -1},
	   {3, 6, -8},
	   {4, 5, 3},
	   {5, 2, 7},
	};

	auto h = graph2(vt.begin(), vt.end());
	if (!h.insert_edge(5, 2, 7)) {
		std::cout << "fail insert, already an edge" << std::endl;
	}
	else {
		std::cout << "success insert, and add the edge" << std::endl;
	}
	auto hm{std::move(h)};
	if (!hm.insert_edge(5, 2, 7)) {
		std::cout << "fail insert, already an edge" << std::endl;
	}
	else {
		std::cout << "success insert, and add the edge" << std::endl;
	}
	decltype(hm) m;
	m = hm;
	for (const auto& i : vt) {
		std::cout << (m.find(i.from, i.to, i.weight) == m.end()) << std::endl;
	}
	auto n = graph2(m);
	for (const auto& i : vt) {
		std::cout << (n.find(i.from, i.to, i.weight) == n.end()) << std::endl;
	}
	std::cout << "replace" << std::endl;
	n.replace_node(1, 7);
	std::cout << n.is_node(7) << std::endl;
	std::cout << (n.find(1, 5, -1) == n.end()) << "\t" << (n.find(7, 5, -1) == n.end()) << std::endl;
	std::cout << (n.find(4, 1, -4) == n.end()) << "\t" << (n.find(4, 7, -4) == n.end()) << std::endl;
	n.replace_node(7, 1);
	n.insert_node(10);
	n.merge_replace_node(1, 4);
	std::cout << n.is_node(10) << std::endl;
	std::cout << (n.find(4, 5, -1) == n.end()) << std::endl;
	std::cout << (n.find(4, 4, -4) == n.end()) << std::endl;
	std::cout << (n.find(2, 4, 2) == n.end()) << std::endl;
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
#endif