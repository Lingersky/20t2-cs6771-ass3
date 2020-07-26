#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <concepts/concepts.hpp>
#include <concepts/type_traits.hpp>
#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <range/v3/iterator.hpp>
#include <range/v3/utility.hpp>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

template<typename T, typename S>
struct set_compare {
	using is_transparent = void;
	auto operator()(const std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>& lhs,
	                const std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>& rhs) const
	   -> bool {
		if (*(lhs.first) < *(rhs.first)) {
			return true;
		}
		if (*(lhs.first) == *(rhs.first)) {
			return (*(lhs.second) < *(rhs.second));
		}
		return false;
	}
	// auto operator()(const std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>& lhs,
	//                 const std::pair<const T, const S>& rhs) const -> bool {
	// 	if (*(lhs.first) < rhs.first) {
	// 		return true;
	// 	}
	// 	if (*(lhs.first) == rhs.first) {
	// 		return (*(lhs.second) < rhs.second);
	// 	}
	// 	return false;
	// }
	// auto operator()(const std::pair<const T, const S>& lhs,
	//                 const std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>& rhs)
	//                 const
	//    -> bool {
	// 	if (lhs.first < *(rhs.first)) {
	// 		return true;
	// 	}
	// 	if (lhs.first == *(rhs.first)) {
	// 		return (lhs.second < *(rhs.second));
	// 	}
	// 	return false;
	// }
};
template<typename T>
struct map_compare {
	using is_transparent = void;
	auto operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const -> bool {
		return *lhs < *rhs;
	}
	// auto operator()(const std::shared_ptr<T>& lhs, const T& rhs) const -> bool {
	// 	return *lhs < rhs;
	// }
	// auto operator()(const T& lhs, const std::shared_ptr<T>& rhs) const -> bool {
	// 	return lhs < *rhs;
	// }
};
template<typename T, typename S>
using link_set =
   std::set<std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>, set_compare<T, S>>;
template<typename T, typename S>
using node_map = std::map<const std::shared_ptr<T>, link_set<T, S>, map_compare<T>>;

namespace gdwg {
	template<concepts::regular N, concepts::regular E>
	requires concepts::totally_ordered<N>and concepts::totally_ordered<E> class graph {
	public:
		class iterator;

		struct value_type {
			N from;
			N to;
			E weight;
		};
		// Constructors
		graph() noexcept = default;
		graph(std::initializer_list<N> il) {
			for (const auto& l : il) {
				insert_node(l);
			}
		}
		template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
		requires ranges::indirectly_copyable<I, N*> graph(I first, S last) {
			for (; first != last; ++first) {
				insert_node(*first); // if is duplicate value, InsertNode() will ignore it
			}
		}
		template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
		requires ranges::indirectly_copyable<I, value_type*> graph(I first, S last);
		graph(graph&& other) noexcept;
		auto operator=(graph&& other) noexcept -> graph&;
		graph(graph const& other);
		auto operator=(graph const& other) -> graph&;

		// Modifiers
		auto insert_node(N const& value) -> bool {
			if (is_node(value)) {
				return false;
			}
			auto node = std::make_shared<N>(value);
			node_map_[node];
			return true;
		}
		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call Graph::InsertEdge when either "
				                         "src or dst node does not exist");
			}
			if (find(src, dst, weight) != end()) {
				return false;
			}
			auto node_src = std::make_shared<N>(src);
			auto node_dst = std::make_shared<N>(dst);
			auto edge = std::make_unique<E>(weight);
			node_map_[node_src].emplace(std::make_pair(node_dst, std::move(edge)));
			return true;
		}

		// Accessors
		[[nodiscard]] auto is_node(N const& value) -> bool {
			auto node = std::make_shared<N>(value);
			return static_cast<bool>(node_map_.find(node) != node_map_.end());
		}
		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) -> iterator {
			auto node_src = std::make_shared<N>(src);
			auto map_pos = node_map_.find(node_src);
			if (map_pos == node_map_.end()) {
				return end();
			}
			auto link = map_pos->second;
			auto node_dst = std::make_shared<N>(dst);
			auto edge = std::make_unique<E>(weight);
			auto pair_find = std::make_pair(node_dst, edge);
			auto set_pos = link.find(pair_find);
			if (set_pos == link.end()) {
				return end();
			}
			return iterator(node_map_, map_pos, set_pos);
		}

		// Range access
		[[nodiscard]] auto begin() const -> iterator {
			for (auto it = node_map_.begin(); it != node_map_.end(); ++it) {
				if (!it->second.empty()) {
					return iterator(node_map_, it, it->second.begin());
				}
			}
			return end();
		}
		[[nodiscard]] auto end() const -> iterator {
			return iterator(node_map_, node_map_.end(), (node_map_.begin()->second).end());
		}

		// Comparisons
		[[nodiscard]] auto operator==(graph const& other) -> bool;

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream&;
		// Your member functions go here
	private:
		node_map<N, link_set<N, E>> node_map_{};
	};
	template<concepts::regular N, concepts::regular E>
	requires concepts::totally_ordered<N>and concepts::totally_ordered<E> class graph<N, E>::iterator {
		using outer_iterator = typename node_map<N, link_set<N, E>>::iterator;
		using inner_iterator = typename link_set<N, E>::iterator;
		// using node_map = std::map<const std::shared_ptr<T>, link_set<T, S>, map_compare<T>>;
		// using link_set =std::set<std::pair<const std::shared_ptr<T>, const std::unique_ptr<S>>,
		// set_compare<T, S>>;

	public:
		using value_type = ranges::common_tuple<N, N, E>;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;
		using reference = ranges::common_tuple<N const&, N const&, E const&>;
		using pointer = void;
		using iterator_concept = std::bidirectional_iterator_tag;
		// Iterator constructor
		iterator() = default;

		// Iterator source
		explicit iterator(node_map<N, link_set<N, E>>& pointee,
		                  outer_iterator outer,
		                  inner_iterator inner) noexcept
		: pointee_{&pointee}
		, outer_{outer}
		, inner_{inner} {}
		auto operator*() -> ranges::common_tuple<N const&, N const&, E const&> {
			return ranges::common_tuple(*(outer_->first), *(inner_->first), *(inner_->second));
		}

		// Iterator traversal
		auto operator++() -> iterator&;
		auto operator++(int) -> iterator;
		auto operator--() -> iterator&;
		auto operator--(int) -> iterator;

		// Iterator comparison
		auto operator==(iterator const& other) const -> bool {
			return !static_cast<bool>(*(outer_->first) != *other.outer_->first
			                          || *(inner_->first) != *(other.inner_->first)
			                          || *(inner_->second) != *(other.inner_->second));
			// return static_cast<bool>(*this == *other);
		}
		auto operator!=(iterator const& other) -> bool {
			return !(*this == other);
		}

	private:
		node_map<N, link_set<N, E>>* pointee_;
		outer_iterator outer_;
		inner_iterator inner_;

		//   explicit iterator(unspecified);
	};
} // namespace gdwg

// template<concepts::regular N, concepts::regular E>
// requires concepts::totally_ordered<N>and
//    concepts::totally_ordered<E> template<ranges::forward_iterator I, ranges::sentinel_for<I>
//    S> requires ranges::indirectly_copyable<I, graph<N, E>::value_type*> graph<N, E>::graph(I
//    first, S last) {
// 	for (; first != last; ++first) {
// 	   insert_node
// 	}
// }
// template<concepts::regular N, concepts::regular E>
// requires concepts::totally_ordered<N>and concepts::totally_ordered<E>
// gdwg::graph<N, E>::graph(graph&& other) noexcept
// : node_list_{std::move(other.node_list_)}
// , edge_list_{std::move(other.edge_list_)} {}

#endif // GDWG_GRAPH_HPP
