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
#include <type_traits>
#include <utility>
#include <vector>

template<typename T>
struct map_compare {
	using is_transparent = void;
	auto operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const -> bool {
		return *lhs < *rhs;
	}
	auto operator()(const std::shared_ptr<T>& lhs, const T& rhs) const -> bool {
		return *lhs < rhs;
	}
	auto operator()(const T& lhs, const std::shared_ptr<T>& rhs) const -> bool {
		return lhs < *rhs;
	}
};
template<typename T, typename S>
struct link_struct {
	std::shared_ptr<T> dst;
	std::shared_ptr<S> edge;
};
template<typename T, typename S>
struct set_compare {
	using is_transparent = void;
	auto operator()(const link_struct<T, S>& lhs, const link_struct<T, S>& rhs) const -> bool {
		if (*(lhs.dst) < *(rhs.dst)) {
			return true;
		}
		if (*(lhs.dst) == *(rhs.dst)) {
			return (*(lhs.edge) < *(rhs.edge));
		}
		return false;
	}
	auto operator()(const link_struct<T, S>& lhs, const std::pair<const T, const S>& rhs) const
	   -> bool {
		if (*(lhs.dst) < rhs.first) {
			return true;
		}
		if (*(lhs.dst) == rhs.first) {
			return (*(lhs.edge) < rhs.second);
		}
		return false;
	}
	auto operator()(const std::pair<const T, const S>& lhs, const link_struct<T, S>& rhs) const
	   -> bool {
		if (lhs.first < *(rhs.dst)) {
			return true;
		}
		if (lhs.first == *(rhs.dst)) {
			return (lhs.second < *(rhs.edge));
		}
		return false;
	}
};
template<typename T, typename S>
using link_set = std::set<link_struct<T, S>, set_compare<T, S>>;
template<typename T, typename S>
using node_map = std::map<std::shared_ptr<T>, link_set<T, S>, map_compare<T>>;
template<typename T>
using isolation_set = std::set<std::shared_ptr<T>, map_compare<T>>;

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
			for (; !(first == last); ++first) {
				insert_node(*first); // if is duplicate value, InsertNode() will ignore it
			}
		}
		template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
		requires ranges::indirectly_copyable<I, value_type*> graph(I first, S last) {
			for (; first != last; ++first) {
				auto src = first->from;
				auto dst = first->to;
				auto weight = first->weight;
				insert_node(src);
				insert_node(dst);
				inner_insert_edge(src, dst, weight);
			}
		}
		graph(graph&& other) noexcept
		: from_to_map_{std::move(other.from_to_map_)}
		, isolation_node_{std::move(other.isolation_node_)} {}

		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				from_to_map_ = std::move(other.from_to_map_);
				isolation_node_ = std::move(other.isolation_node_);
			}
			return *this;
		}
		graph(graph const& other) {
			if (this != &other) {
				*this = other;
			}
		}
		auto operator=(graph const& other) -> graph& {
			if (this != &other) {
				for (auto& i : other.isolation_node_) {
					auto tmp = std::make_shared<N>(*i);
					isolation_node_.emplace(std::move(tmp));
				}
				for (auto& i : other.from_to_map_) {
					auto outer = get_node_ptr_in_link(*(i.first));
					if (outer == nullptr) {
						outer = std::make_shared<N>(*(i.first));
					}
					for (auto& j : i.second) {
						auto inner = get_node_ptr_in_link(*(j.dst));
						if (inner == nullptr) {
							inner = std::make_shared<N>(*(j.dst));
						}
						auto edge = std::make_shared<N>(*(j.edge));
						link_struct<N, E> pair_find{inner, edge};
						from_to_map_[outer].emplace(pair_find);
					}
				}
			}
			return *this;
		}

		// Modifiers
		auto insert_node(N const& value) -> bool {
			if (is_node(value)) {
				return false;
			}
			auto node = std::make_shared<N>(value);
			isolation_node_.emplace(node);
			return true;
		}
		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src "
				                         "or dst node does not exist");
			}
			return inner_insert_edge(src, dst, weight);
		}
		auto replace_node(N const& old_data, N const& new_data) -> bool {
			if (!is_node(old_data)) {
				throw std::runtime_error("Cannot call comp6771::graph<N, E>::replace_node on a node "
				                         "that doesn't exist");
			}
			if (is_node(new_data)) {
				return false;
			}
			auto it = from_to_map_.find(old_data);
			*(it->first) = new_data; // just need to replce the value of the pointer.
			auto new_map = from_to_map_; // copy constructor will resort the map
			from_to_map_ = std::move(new_map);
			return true;
		}
		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			if (!is_node(old_data) or !is_node(new_data)) {
				throw std::runtime_error("Cannot call comp6771::graph<N, E>::merge_replace_node on old "
				                         "or new data if they don't exist in the graph");
			}
			auto old_node_place = isolation_node_.find(old_data);
			if (old_node_place != isolation_node_.end()) {
				isolation_node_.erase(old_node_place);
				return;
			}
			auto new_node_place = isolation_node_.find(new_data);
			std::shared_ptr<N> new_ptr{};
			if (new_node_place != isolation_node_.end()) {
				new_ptr = *new_node_place;
			}
			else {
				new_ptr = get_node_ptr_in_link(new_data);
			}
			auto new_from_to = decltype(from_to_map_){};
			for (auto& i : from_to_map_) {
				std::map<N, std::set<E>> edge_pool{};
				for (auto& j : i->second) {
					edge_pool[*(j->dst) == old_data ? new_data : *(j->dst)].emplace(*(j->edge));
				}
				if (*(i->first) == old_data) {
					from_to_map_[*new_node_place] = from_to_map_[i->first];
				}
			}
			node_map<N, E> new_from_to{};
		}

		// auto new_ptr = from_to_map_.find(new_data)->first;
		// auto old_ptr = from_to_map_.find(old_data)->first;
		// reverse_node_in_map(from_to_map_, to_from_map_, old_ptr, new_ptr);
		// reverse_node_in_map(to_from_map_, from_to_map_, old_ptr, new_ptr);
#if 0
			// if (from_to_map_.find(old_data)->second.size() != 0) {
			// 	for (auto& i = from_to_map_[old_data].begin(); i != from_to_map_[old_data].end(); i++) {
			// 		auto pattern = std::make_pair(*(i->dst), *(i->edge));
			// 		if (from_to_map_[node_ptr].find(pattern) == from_to_map_[node_ptr].end()) {
			// 			from_to_map_[node_ptr].emplace(*i);
			// 		}
			// 	}
			// }
			// from_to_map_.erase(from_to_map_.find(old_data)->first);
			// for (auto& i = to_from_map_.begin(); i != to_from_map_.end(); i++) {
			// 	for (auto& j = (i->second).begin(); j != (i->second).end(); j++) {
			// 		if (*(j->dst) == old_data) {
			// 			j->dst = node_ptr;
			// 		}
			// 	}
			// }
			// if (to_from_map_.find(old_data) != to_from_map_.end()) {
			// 	for (auto& i = to_from_map_[old_data].begin(); i != to_from_map_[old_data].end(); i++) {
			// 		auto pattern = std::make_pair(*(i->dst), *(i->edge));
			// 		if (to_from_map_[node_ptr].find(pattern) == to_from_map_[node_ptr].end()) {
			// 			to_from_map_[node_ptr].emplace(*i);
			// 		}
			// 	}
			// }
			// to_from_map_.erase(to_from_map_.find(old_data)->first);
			// for (auto& i = from_to_map_.begin(); i != from_to_map_.end(); i++) {
			// 	for (auto& j = (i->second).begin(); j != (i->second).end(); j++) {
			// 		if (*(j->dst) == old_data) {
			// 			j->dst = node_ptr;
			// 		}
			// 	}
			// }


			// auto node = node_map_.find(new_data)->first;
			// decltype(node_map_) new_map;
			// for (auto it = begin(); it != end(); it++) {
			// 	auto share_ptr_tuple = it.get_shared();
			// 	if (std::get<1>(share_ptr_tuple) == nullptr) {
			// 		if (std::get<0>(*it) == old_data) {
			// 			if (new_map.find(new_data) == new_map.end()) {
			// 				new_map[node];
			// 			}
			// 		}
			// 	}
			// 	else {
			// 		if (std::get<0>(*it) == old_data) {
			// 			std::get<0>(share_ptr_tuple) = node;
			// 		}
			// 		if (std::get<1>(*it) == old_data) {
			// 			std::get<1>(share_ptr_tuple) = node;
			// 		}
			// 		link_struct<N, E> pair{std::get<1>(share_ptr_tuple), std::get<2>(share_ptr_tuple)};
			// 		if (new_map[std::get<0>(share_ptr_tuple)].find(pair)
			// 		    == new_map[std::get<0>(share_ptr_tuple)].end()) {
			// 			new_map[std::get<0>(share_ptr_tuple)].emplace(pair);
			// 		}
			// 	}
			// }
			// node_map_.clear();
			// node_map_ = std::move(new_map);
#endif

		// auto erase_node(N const& value) -> bool {
		// 	auto it = node_map_.find(value);
		// 	if (it != node_map_.end()) {
		// 		node_map_.erase(it);
		// 		for (auto& flevel = node_map_.beging(); flevel != node_map_.end(); flevel++) {
		// 			for (auto& slevel = flevel->second.begin(); slevel != flevel->second.end();
		// slevel++)
		// 			{
		// 			}
		// 		}
		// 	}
		// 	return false;
		// }
		auto erase_edge(N const& src, N const& dst, E const& weight) -> bool;
		auto erase_edge(iterator i) -> iterator;
		auto erase_edge(iterator i, iterator s) -> iterator;
		auto clear() noexcept -> void;

		// Accessors
		[[nodiscard]] auto is_node(N const& value) -> bool {
			if (isolation_node_.find(value) != isolation_node_.end()) {
				return true;
			}
			return get_node_ptr_in_link(value) != nullptr;
		}
		[[nodiscard]] auto empty() -> bool {
			return (from_to_map_.size() == 0 and isolation_node_.size() == 0);
		}
		[[nodiscard]] auto is_connected(N const& src, N const& dst) -> bool;
		[[nodiscard]] auto nodes() -> std::vector<N>;
		[[nodiscard]] auto weights(N const& from, N const& to) -> std::vector<E>;
		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) -> iterator {
			auto map_pos = from_to_map_.find(src);
			if (map_pos == from_to_map_.end()) {
				return end();
			}
			auto link = map_pos->second;
			auto set_pos = link.find(std::make_pair(dst, weight));
			if (set_pos == link.end()) {
				return end();
			}
			return iterator(isolation_node_, from_to_map_, isolation_node_.end(), map_pos, set_pos);
		}
		[[nodiscard]] auto connections(N const& src) -> std::vector<N>;

		// Range access
		[[nodiscard]] auto begin() const -> iterator {
			auto inner = decltype(from_to_map_.begin()->second.begin()){};
			return iterator(isolation_node_,
			                from_to_map_,
			                isolation_node_.begin(),
			                from_to_map_.end(),
			                std::move(inner));
		}
		[[nodiscard]] auto end() const -> iterator {
			auto inner = decltype(from_to_map_.begin()->second.begin()){};
			return iterator(isolation_node_,
			                from_to_map_,
			                isolation_node_.end(),
			                from_to_map_.end(),
			                std::move(inner));
		}

		// Comparisons
		[[nodiscard]] auto operator==(graph const& other) -> bool;

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream&;

	private:
		node_map<N, E> from_to_map_{};
		isolation_set<N> isolation_node_{};
		auto inner_insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (find(src, dst, weight) != end()) {
				return false;
			}
			auto node_src = decltype(from_to_map_.begin()->first){};
			auto node_dst = decltype(from_to_map_.begin()->first){};
			auto it = isolation_node_.find(src);
			if (it == isolation_node_.end()) {
				node_src = get_node_ptr_in_link(src);
			}
			else {
				node_src = *it;
				isolation_node_.erase(it);
			}
			it = isolation_node_.find(dst);
			if (it == isolation_node_.end()) {
				node_dst = get_node_ptr_in_link(dst);
			}
			else {
				node_dst = *it;
				isolation_node_.erase(it);
			}
			auto edge = std::make_shared<E>(weight);
			link_struct<N, E> pair_find{node_dst, edge};
			from_to_map_[node_src].emplace(pair_find);
			return true;
		}
		[[nodiscard]] auto get_node_ptr_in_link(N const& value) -> std::shared_ptr<N> {
			for (auto& i : from_to_map_) {
				if (*(i.first) == value) {
					return i.first;
				}
				for (auto& j : i.second) {
					if (*(j.dst) == value) {
						return j.dst;
					}
				}
			}
			return nullptr;
		}
	};
	template<concepts::regular N, concepts::regular E>
	requires concepts::totally_ordered<N>and concepts::totally_ordered<E> class graph<N, E>::iterator {
		using isolation_iterator = typename isolation_set<N>::const_iterator;
		using outer_iterator = typename node_map<N, E>::const_iterator;
		using inner_iterator = typename link_set<N, E>::const_iterator;

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
		explicit iterator(isolation_set<N> const& pointee_isolate,
		                  node_map<N, E> const& pointee_linked,
		                  isolation_iterator isoiter,
		                  outer_iterator outer,
		                  inner_iterator inner) noexcept
		: pointee_isolate_{&pointee_isolate}
		, pointee_linked_{&pointee_linked}
		, isoiter_{isoiter}
		, outer_{outer}
		, inner_{inner} {}
		auto operator*() -> ranges::common_tuple<N const&, N const&, E const&> {
			return ranges::common_tuple<N, N, E>(*(outer_->first), *(inner_->dst), *(inner_->edge));
		} //????

		// Iterator traversal
		auto operator++() -> iterator& {
			if (isoiter_ != isolation_iterator()) {
				isoiter_++;
				if (isoiter_ != pointee_isolate_->end()) {
					return *this;
				}
				isoiter_ = isolation_iterator();
			}
			if (inner_ != outer_->second.end()) {
				++inner_;

				if (inner_ != outer_->second.end()) {
					return *this;
				}
			}
			++outer_;
			inner_ = outer_ == pointee_linked_->end() ? inner_iterator() : outer_->second.begin();
			return *this;
		}
		auto operator++(int) -> iterator {
			auto temp = *this;
			++*this;
			return temp;
		}
		auto operator--() -> iterator& {
			if (isoiter_ != isolation_iterator()) {
				if (isoiter_ != pointee_isolate_->begin()) {
					isoiter_--;
					return *this;
				}
			}
			if (inner_ == inner_iterator()) {
				outer_ = ranges::prev(pointee_linked_->end());
				inner_ = ranges::prev(outer_->second.end());
				return *this;
			}

			if (inner_ != outer_->second.begin()) {
				--inner_;
				return *this;
			}
			if (outer_ != pointee_linked_->begin()) {
				--outer_;
				inner_ = ranges::prev(outer_->second.end());

				return *this;
			}
			isoiter_ = ranges::prev(pointee_isolate_->end());
			return *this;
		}
		auto operator--(int) -> iterator {
			auto temp = *this;
			--*this;
			return temp;
		}

		// Iterator comparison
		auto operator==(iterator const& other) const -> bool {
			if (other.isoiter_ != other.pointee_isolate_->end() and isoiter_ != pointee_isolate_->end())
			{
				return static_cast<bool>(*(isoiter_) == *(other.isoiter_));
			}
			if (other.isoiter_ == other.pointee_isolate_->end() and isoiter_ == pointee_isolate_->end())
			{
				if (other.outer_ == other.pointee_linked_->end() or outer_ == pointee_linked_->end()) {
					return static_cast<bool>(other.outer_ == other.pointee_linked_->end()
					                         and outer_ == pointee_linked_->end());
				}
				return !static_cast<bool>(*(outer_->first) != *(other.outer_->first)
				                          || *(inner_->dst) != *(other.inner_->dst)
				                          || *(inner_->edge) != *(other.inner_->edge));
			}
			return false;
		}
		// auto get_shared()
		//    -> ranges::common_tuple<std::shared_ptr<N>, std::shared_ptr<N>, std::shared_ptr<E>> {
		// 	if (inner_ == inner_iterator()) {
		// 		return ranges::common_tuple<std::shared_ptr<N>, std::shared_ptr<N>,
		// std::shared_ptr<E>>( 		   outer_->first, 		   nullptr, 		   nullptr);
		// 	}
		// 	return ranges::common_tuple<std::shared_ptr<N>, std::shared_ptr<N>, std::shared_ptr<E>>(
		// 	   outer_->first,
		// 	   inner_->dst,
		// 	   inner_->edge);
		// }

	private:
		isolation_set<N> const* pointee_isolate_;
		node_map<N, E> const* pointee_linked_;
		isolation_iterator isoiter_;
		outer_iterator outer_;
		inner_iterator inner_;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_HPP
