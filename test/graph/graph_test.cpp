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

TEST_CASE("graph()") {
	auto g = gdwg::graph<int, std::string>{};
	auto out = std::ostringstream{};
	out << g;
	CHECK(out.str().empty());
	// static_assert(ranges::bidirectional_iterator<gdwg::graph<int, int>::iterator>);
}
TEST_CASE("graph(std::initializer_list<N> il)") {
	auto g = gdwg::graph<int, std::string>{2, 3, 4};
	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(2 (
)
3 (
)
4 (
)
)");
	CHECK(out.str() == expected_output);
}
TEST_CASE("node: graph(I first, S last)") {
	std::vector<int> vec{1, 2, 3, 4, 5};
	auto g = gdwg::graph<int, std::string>(++vec.begin(), --vec.end());
	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(2 (
)
3 (
)
4 (
)
)");
	CHECK(out.str() == expected_output);
}
TEST_CASE("value_type: graph(I first, S last)") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 2},
	   {2, 2, 3},
	   {2, 3, 4},
	   {2, 4, 5},
	};
	auto h = graph(++vt.begin(), --vt.end());
	auto out = std::ostringstream{};
	out << h;
	auto const expected_output = std::string_view(R"(1 (
  2 | 2
)
2 (
  2 | 3
  3 | 4
)
3 (
)
)");
	CHECK(out.str() == expected_output);
}

TEST_CASE("graph(graph&& other) noexcept") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 2},
	   {2, 2, 3},
	   {2, 3, 4},
	   {2, 4, 5},
	};
	auto h = graph(vt.begin(), vt.end());
	auto out1 = std::ostringstream{};
	out1 << h;
	auto g{std::move(h)};
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}

TEST_CASE("auto operator=(graph&& other) noexcept -> graph&") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 2},
	   {2, 2, 3},
	   {2, 3, 4},
	   {2, 4, 5},
	};
	auto h = graph(vt.begin(), vt.end());
	auto iter = h.begin();
	auto out1 = std::ostringstream{};
	out1 << h;
	auto g = std::move(h);
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
	CHECK(std::get<0>(*iter) == 1);
	CHECK(std::get<1>(*iter) == 1);
	CHECK(std::get<2>(*iter) == 1);
}

TEST_CASE("graph(graph const& other)") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 2},
	   {2, 2, 3},
	   {2, 3, 4},
	   {2, 4, 5},
	};
	auto h = graph(vt.begin(), vt.end());
	auto out1 = std::ostringstream{};
	out1 << h;
	auto g(h);
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
	g.replace_node(4, 5);
	CHECK(g.is_node(5));
	CHECK(!h.is_node(5));
}

TEST_CASE("auto operator=(graph const& other) -> graph&") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 2},
	   {2, 2, 3},
	   {2, 3, 4},
	   {2, 4, 5},
	};
	auto h = graph(vt.begin(), vt.end());
	auto out1 = std::ostringstream{};
	out1 << h;
	auto g = h;
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
	g.replace_node(4, 5);
	CHECK(g.is_node(5));
	CHECK(!h.is_node(5));
}

TEST_CASE("insert_node") {
	auto g = gdwg::graph<int, std::string>{2, 3, 4};
	CHECK(g.insert_node(5));
	CHECK(!g.insert_node(5));
	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(2 (
)
3 (
)
4 (
)
5 (
)
)");
	CHECK(out.str() == expected_output);
}

TEST_CASE("insert_edge") {
	using graph = gdwg::graph<int, int>;
	auto const vt = std::vector<graph::value_type>{
	   {1, 2, 2},
	   {2, 3, 4},
	};
	auto h = graph(vt.begin(), vt.end());
	CHECK(h.insert_edge(2, 2, 3));
	CHECK(!h.insert_edge(2, 2, 3));
	CHECK(!h.is_node(4));
	CHECK_THROWS_MATCHES(h.insert_edge(4, 2, 2),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge when "
	                                              "either src or dst node does not exist"));
	CHECK_THROWS_MATCHES(h.insert_edge(2, 4, 2),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge when "
	                                              "either src or dst node does not exist"));

	auto out = std::ostringstream{};
	out << h;
	auto const expected_output = std::string_view(R"(1 (
  2 | 2
)
2 (
  2 | 3
  3 | 4
)
3 (
)
)");
	CHECK(out.str() == expected_output);
}

TEST_CASE("replace_node") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());

	CHECK_THROWS_MATCHES(h.replace_node(5, 6),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call comp6771::graph<N, E>::replace_node "
	                                              "on a node that doesn't exist"));
	CHECK(!h.replace_node(1, 2));
	CHECK(h.replace_node(1, 5));
	auto out1 = std::ostringstream{};
	out1 << h;
	auto const vt2 = std::vector<graph::value_type>{
	   {5, 5, 1},
	   {5, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}
TEST_CASE("merge_replace_node") {
	using graph = gdwg::graph<std::string, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {"A", "B", 3},
	   {"C", "B", 2},
	   {"C", "A", 2},
	   {"D", "B", 4},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK_THROWS_MATCHES(h.merge_replace_node("E", "A"),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call comp6771::graph<N, "
	                                              "E>::merge_replace_node on old or new data if "
	                                              "they don't exist in the graph"));
	CHECK_THROWS_MATCHES(h.merge_replace_node("B", "E"),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call comp6771::graph<N, "
	                                              "E>::merge_replace_node on old or new data if "
	                                              "they don't exist in the graph"));
	h.merge_replace_node("B", "A");
	auto out1 = std::ostringstream{};
	out1 << h;
	auto const vt2 = std::vector<graph::value_type>{
	   {"A", "A", 3},
	   {"C", "A", 2},
	   {"D", "A", 4},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}
TEST_CASE("erase_node") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 1, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK(h.erase_node(1));
	auto out1 = std::ostringstream{};
	out1 << h;
	CHECK(!h.erase_node(1));
	CHECK(!h.erase_node(5));
	auto const vt2 = std::vector<graph::value_type>{
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}

TEST_CASE("erase_edge(N, N, E)") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());

	CHECK_THROWS_MATCHES(h.erase_edge(5, 1, 1),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call comp6771::graph<N, E>::erase_edge on "
	                                              "src or dst if they don't exist in the graph"));
	CHECK_THROWS_MATCHES(h.erase_edge(1, 5, 1),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call comp6771::graph<N, E>::erase_edge on "
	                                              "src or dst if they don't exist in the graph"));
	CHECK(h.erase_edge(1, 2, 1));
	CHECK(!h.erase_edge(1, 2, 1));
	CHECK(!h.erase_edge(1, 1, 3));
	auto out1 = std::ostringstream{};
	out1 << h;
	auto const vt2 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}

TEST_CASE("erase_edge(iter)") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK(h.erase_edge(h.find(1, 2, 1)) == h.find(2, 2, 1));
	CHECK(h.erase_edge(h.end())==h.end());
	auto out1 = std::ostringstream{};
	out1 << h;
	auto const vt2 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
}

TEST_CASE("erase_edge(iter,iter)") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	   {2, 5, 1},
	   {3, 2, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK(h.erase_edge(h.find(1, 2, 1), h.find(2, 4, 1)) == h.find(2, 4, 1));
	auto out1 = std::ostringstream{};
	out1 << h;
	auto const vt2 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {2, 4, 1},
	   {2, 5, 1},
	   {3, 2, 1},
	};
	auto g = graph(vt2.begin(), vt2.end());
	auto out2 = std::ostringstream{};
	out2 << g;
	CHECK(out1.str() == out2.str());
	CHECK(h.erase_edge(h.find(2, 4, 1), h.end()) == h.end());
	CHECK(h.find(1, 1, 1) != h.end());
}
TEST_CASE("clear") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	h.clear();
	CHECK(h.empty());
}
TEST_CASE("is_node") {
	auto const g = gdwg::graph<int, std::string>{2, 3, 4};
	CHECK(g.is_node(2));
	CHECK(g.is_node(3));
	CHECK(g.is_node(4));
	CHECK(!g.is_node(5));
}
TEST_CASE("empty") {
	auto const g = gdwg::graph<int, std::string>{};
	CHECK(g.empty());
	auto const h = gdwg::graph<int, std::string>{2, 3, 4};
	CHECK(!h.empty());
}
TEST_CASE("is_connected") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto const h = graph(vt1.begin(), vt1.end());
	CHECK_THROWS_MATCHES(h.is_connected(1, 6),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::is_connected if "
	                                              "src or dst node don't exist in the graph"));
	CHECK_THROWS_MATCHES(h.is_connected(6, 1),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::is_connected if "
	                                              "src or dst node don't exist in the graph"));
	CHECK(h.is_connected(1, 1));
	CHECK(!h.is_connected(2, 1));
	CHECK(h.is_connected(2, 5));
	CHECK(!h.is_connected(1, 5));
}
TEST_CASE("nodes") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 1},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto const h = graph(vt1.begin(), vt1.end());
	std::vector<int> vec{1, 2, 3, 4, 5};
	CHECK(h.nodes() == vec);
	auto const g = gdwg::graph<int, std::string>{};
	CHECK(g.nodes().empty());
	auto k = gdwg::graph<int, std::string>{2, 3, 4};
	CHECK(k.nodes() == decltype(vec){2, 3, 4});
}
TEST_CASE("weights") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 5},
	   {2, 3, 3},
	   {2, 3, 7},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto const h = graph(vt1.begin(), vt1.end());
	CHECK_THROWS_MATCHES(h.weights(1, 6),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if src or "
	                                              "dst node don't exist in the graph"));
	CHECK_THROWS_MATCHES(h.weights(6, 1),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if src or "
	                                              "dst node don't exist in the graph"));
	CHECK(h.weights(2, 3) == std::vector<int>{3, 5, 7});
	CHECK(h.weights(3, 2).empty());
}
TEST_CASE("find") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 5},
	   {2, 3, 3},
	   {2, 3, 7},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK(h.find(2, 3, 3) != h.end());
	CHECK(*(h.find(2, 3, 3)) == ranges::common_tuple<int, int, int>{2, 3, 3});
	CHECK(h.find(2, 2, 3) == h.end());
}

TEST_CASE("connections") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 5},
	   {2, 3, 3},
	   {2, 3, 7},
	   {2, 4, 1},
	   {2, 5, 1},
	   {3, 5, 5},
	};
	auto h = graph(vt1.begin(), vt1.end());
	h.insert_node(10);
	CHECK_THROWS_MATCHES(h.connections(6),
	                     std::runtime_error,
	                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::connections if "
	                                              "src doesn't exist in the graph"));
	CHECK(h.connections(2) == std::vector<int>{2, 3, 4, 5});
	CHECK(h.connections(10).empty());
}
TEST_CASE("begin") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 1},
	   {2, 2, 1},
	   {2, 3, 5},
	   {2, 3, 3},
	   {2, 3, 7},
	   {2, 4, 1},
	   {2, 5, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	CHECK(*(h.begin()) == ranges::common_tuple<int, int, int>{1, 1, 1});
	h.erase_edge(1, 1, 1);
	CHECK(*(h.begin()) == ranges::common_tuple<int, int, int>{1, 2, 1});
}
TEST_CASE("end") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {2, 1, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	auto iter = h.begin();
	iter++;
	iter++;
	CHECK(iter == h.end());
}
TEST_CASE("operator==") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 3},
	   {2, 1, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	auto g = graph(vt1.begin(), vt1.end());
	CHECK(h == g);
	h.insert_node(3);
	CHECK_FALSE(h == g);
	h.erase_node(3);
	CHECK(h == g);
	h.insert_edge(1, 1, 0);
	CHECK_FALSE(h == g);
}

TEST_CASE("operator*") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 3},
	   {2, 1, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	auto iter = h.begin();
	CHECK(*iter == ranges::common_tuple<int, int, int>{1, 1, 1});
	iter++;
	CHECK(*iter == ranges::common_tuple<int, int, int>{1, 2, 3});
	iter++;
	CHECK(*iter == ranges::common_tuple<int, int, int>{2, 1, 1});
}

TEST_CASE("operator--") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 3},
	   {2, 1, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	auto iter = h.end();
	iter--;
	CHECK(*iter == ranges::common_tuple<int, int, int>{2, 1, 1});
	iter--;
	CHECK(*iter == ranges::common_tuple<int, int, int>{1, 2, 3});
	iter--;
	CHECK(*iter == ranges::common_tuple<int, int, int>{1, 1, 1});
}
TEST_CASE("iter: operator==") {
	using graph = gdwg::graph<int, int>;
	auto const vt1 = std::vector<graph::value_type>{
	   {1, 1, 1},
	   {1, 2, 3},
	   {2, 1, 1},
	};
	auto h = graph(vt1.begin(), vt1.end());
	auto iter_1 = h.begin();
	auto iter_2 = h.end();
	iter_1++;
	iter_2--;
	iter_2--;
	CHECK(iter_2 == iter_1);
}
