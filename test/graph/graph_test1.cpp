#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "gdwg/graph.hpp"
#include <concepts/concepts.hpp>

#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <sstream>

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
	CHECK(n.is_node(10));
	CHECK(n.find(4, 1, -4) != n.end());
	CHECK(n.erase_node(1));
	CHECK(n.find(4, 1, -4) == n.end());
}

TEST_CASE("basic test 5") {
	using graph = gdwg::graph<std::string, int>;
	auto const vt = std::vector<graph::value_type>{{"A", "B", 3}, {"C", "B", 2}, {"D", "B", 4}};
	auto h = graph(vt.begin(), vt.end());
	h.merge_replace_node("B", "A");
	CHECK(h.find("A", "A", 3) != h.end());
	CHECK(h.find("C", "A", 2) != h.end());
	CHECK(h.find("D", "A", 4) != h.end());
	CHECK(h.find("A", "B", 3) == h.end());
	CHECK(h.find("C", "B", 2) == h.end());
	CHECK(h.find("D", "B", 4) == h.end());
	CHECK(!h.erase_edge("A", "A", 2));
	CHECK(h.erase_edge("A", "A", 3));
	CHECK(h.find("A", "A", 3) == h.end());
	CHECK(h.insert_edge("A", "A", 3));
	auto g = h;
	CHECK(g == h);
	CHECK(h.erase_edge(h.find("C", "A", 2)) == h.find("D", "A", 4));
	CHECK(g != h);
	CHECK(h.insert_edge("C", "A", 2));
	CHECK(h.is_connected("C", "A"));
}
TEST_CASE("basic test 6") {
	using graph = gdwg::graph<int, int>;
	auto const v = std::vector<graph::value_type>{
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

	auto g = graph(v.begin(), v.end());
	g.insert_node(64);
	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(1 (
  5 | -1
)
2 (
  1 | 1
  4 | 2
)
3 (
  2 | 2
  6 | -8
)
4 (
  1 | -4
  5 | 3
)
5 (
  2 | 7
)
6 (
  2 | 5
  3 | 10
)
64 (
)
)");
	CHECK(out.str() == expected_output);
}