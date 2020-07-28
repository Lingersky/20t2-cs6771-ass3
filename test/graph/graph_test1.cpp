#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "gdwg/graph.hpp"
#include <concepts/concepts.hpp>

#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

TEST_CASE("basic test") {
	auto g = gdwg::graph<int, std::string>{};
	auto n = 5;
	g.insert_node(n);
	CHECK(g.is_node(n));
}
TEST_CASE("basic test 2") {
	using graph = gdwg::graph<std::string, int>;
	auto g = graph{};
	CHECK(g.insert_node("how"));
	CHECK(!g.insert_node("how"));

	CHECK(g.insert_node("hello"));
	CHECK(g.insert_node("are"));
	CHECK(g.insert_node("you?"));

	CHECK(g.insert_edge("hello", "how", 5));
	CHECK(g.insert_edge("hello", "are", 8));
	CHECK(g.insert_edge("hello", "are", 2));
	CHECK(g.insert_edge("how", "you?", 1));
	CHECK(g.insert_edge("how", "hello", 4));
	CHECK(g.insert_edge("are", "you?", 3));
	CHECK(!g.insert_edge("hello", "how", 5));
}
TEST_CASE("basic test 3") {
	using graph = gdwg::graph<std::string, int>;
	auto const v = std::vector<std::string>{"hello", "yes", "end"};
	auto k = graph(v.begin(), v.end());
	CHECK(k.insert_node("KO"));
	CHECK(!k.insert_node("KO"));
}
TEST_CASE("basic test 4") {
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
	CHECK(!h.insert_edge(5, 2, 7));
	auto hm{std::move(h)};
	CHECK(!hm.insert_edge(5, 2, 7));
	auto m = decltype(hm){};
	m = hm;
	hm.replace_node(1, 7);
	for (const auto& i : vt) {
		CHECK(m.find(i.from, i.to, i.weight) != m.end());
	}
	auto n = graph2(m);
	for (const auto& i : vt) {
		CHECK(n.find(i.from, i.to, i.weight) != n.end());
	}
	std::cout << "replace" << std::endl;
	n.replace_node(1, 7);
	CHECK(n.is_node(7));
	CHECK(n.find(1, 5, -1) == n.end());
	CHECK(n.find(7, 5, -1) != n.end());
	CHECK(n.find(4, 1, -4) == n.end());
	CHECK(n.find(4, 7, -4) != n.end());
	n.replace_node(7, 1);
	n.insert_node(10);
	n.merge_replace_node(1, 4);
	CHECK(n.is_node(10));
	CHECK(n.find(4, 5, -1) != n.end());
	CHECK(n.find(4, 4, -4) != n.end());
	CHECK(n.find(2, 4, 2) != n.end());
}