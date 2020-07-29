#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <concepts/concepts.hpp>
#include <concepts/type_traits.hpp>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ostream>
#include <range/v3/iterator.hpp>
#include <range/v3/utility.hpp>
#include <range/v3/utility/common_tuple.hpp>
#include <set>
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
struct edge_struct {
	std::shared_ptr<T> src;
	std::shared_ptr<T> dst;
	std::shared_ptr<S> edge;
};
template<typename T, typename S>
struct edge_compare {
	auto operator()(edge_struct<T, S> const& lhs, edge_struct<T, S> const& rhs) const -> bool {
		if (*(lhs.src) < *(rhs.src)) {
			return true;
		}
		if (*(lhs.src) == *(rhs.src)) {
			if (*(lhs.dst) < *(rhs.dst)) {
				return true;
			}
			if (*(lhs.dst) == *(rhs.dst)) {
				return (*(lhs.edge) < *(rhs.edge));
			}
		}
		return false;
	}
};
template<typename T>
using nodes_set = std::set<std::shared_ptr<T>, map_compare<T>>;
template<typename T, typename S>
using edges_set = std::set<edge_struct<T, S>, edge_compare<T, S>>;
namespace gdwg {
	template<concepts::regular N, concepts::regular E>
	requires concepts::totally_ordered<N>and concepts::totally_ordered<E> class graph {
	public:
		class iterator {
			using edges_iterator = typename edges_set<N, E>::const_iterator;

		public:
			using value_type = ranges::common_tuple<N, N, E>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;
			// Iterator constructor
			iterator() = default;

			// Iterator source
			explicit iterator(edges_iterator begin, edges_iterator end, edges_iterator iter) noexcept
			: begin_{begin}
			, end_{end}
			, iter_{iter} {}
			auto operator*() const -> ranges::common_tuple<N const, N const, E const> {
				return ranges::common_tuple<N, N, E>(*(iter_->src), *(iter_->dst), *(iter_->edge));
			}
			// Iterator traversal
			// just use the set iterator,it is convenient
			auto operator++() -> iterator& {
				++iter_;
				return *this;
			}
			auto operator++(int) -> iterator {
				auto temp = *this;
				++*this;
				return temp;
			}
			auto operator--() -> iterator& {
				--iter_;
				return *this;
			}
			auto operator--(int) -> iterator {
				auto temp = *this;
				--*this;
				return temp;
			}
			// Iterator comparison
			auto operator==(iterator const& other) const -> bool {
				// consider about the iterator points to the end()
				if (other.iter_ == other.end_ or iter_ == end_) {
					return static_cast<bool>(other.iter_ == other.end_ and iter_ == end_);
				}
				return static_cast<bool>(*(iter_->src) == *(other.iter_->src)
				                         and *(iter_->dst) == *(other.iter_->dst)
				                         and *(iter_->edge) == *(other.iter_->edge));
			}
			friend class graph<N, E>;

		private:
			edges_iterator begin_;
			edges_iterator end_;
			edges_iterator iter_;
		};
		struct value_type {
			N from;
			N to;
			E weight;
		};
		// Constructors
		graph() noexcept = default;
		graph(std::initializer_list<N> il) noexcept {
			for (const auto& l : il) {
				insert_node(l);
			}
		}
		template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
		requires ranges::indirectly_copyable<I, N*> graph(I first, S last) noexcept {
			for (; !(first == last); ++first) {
				insert_node(*first);
			}
		}
		template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
		requires ranges::indirectly_copyable<I, value_type*> graph(I first, S last) noexcept {
			for (; first != last; ++first) {
				auto src = first->from;
				auto dst = first->to;
				auto weight = first->weight;
				insert_node(src); // insert_node will handle duplicate nodes
				insert_node(dst);
				inner_insert_edge(src, dst, weight); // inner_insert_edge will handle duplicate edges
			}
		}
		graph(graph&& other) noexcept
		: all_nodes_{std::move(other.all_nodes_)}
		, all_edges_{std::move(other.all_edges_)} {}

		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				all_edges_ = std::move(other.all_edges_);
				all_nodes_ = std::move(other.all_nodes_);
			}
			return *this;
		}
		graph(graph const& other) noexcept {
			if (this != &other) {
				*this = other;
			}
		}
		auto operator=(graph const& other) noexcept -> graph& {
			if (this != &other) {
				for (auto& i : other.all_nodes_) {
					auto tmp = std::make_shared<N>(*i); // make new entities of edge
					all_nodes_.emplace(std::move(tmp));
				}
				for (auto& i : other.all_edges_) {
					auto src = *(all_nodes_.find(i.src)); // find node entities in the nodes set
					auto dst = *(all_nodes_.find(i.dst));
					auto edge = std::make_shared<E>(*(i.edge)); // make new entities of edge
					edge_struct<N, E> edge_struct{src, dst, edge};
					all_edges_.emplace(edge_struct);
				}
			}
			return *this;
		}

		// Modifiers
		auto insert_node(N const& value) noexcept -> bool {
			if (is_node(value)) { // will not insert duplicate nodes
				return false;
			}
			auto node = std::make_shared<N>(value);
			all_nodes_.emplace(node);
			return true;
		}
		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (!is_node(src) or !is_node(dst)) {
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
			**(all_nodes_.find(old_data)) = new_data; // just modify the entity's value
			auto tmp1 = all_nodes_; // copy constructor will resort the set, it is an easy way
			auto tmp2 = all_edges_;
			all_nodes_ = std::move(tmp1);
			all_edges_ = std::move(tmp2);
			return true;
		}
		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			if (!is_node(old_data) or !is_node(new_data)) {
				throw std::runtime_error("Cannot call comp6771::graph<N, E>::merge_replace_node on old "
				                         "or new data if they don't exist in the graph");
			}
			all_nodes_.erase(all_nodes_.find(old_data)); // erase old nodes in nodes set
			auto new_ptr = *(all_nodes_.find(new_data));
			// create a new edge set, because copying value to shared_ptr in struct has some problems
			auto new_edges_set = decltype(all_edges_){};
			edge_struct<N, E> tmp{};
			for (auto& i : all_edges_) {
				tmp.src = *(i.src) == old_data ? new_ptr : i.src; // change old value to new
				tmp.dst = *(i.dst) == old_data ? new_ptr : i.dst;
				tmp.edge = i.edge;
				new_edges_set.emplace(tmp);
			}
			all_edges_ = std::move(new_edges_set);
		}
		auto erase_node(N const& value) noexcept -> bool {
			auto iter = all_nodes_.find(value);
			if (iter == all_nodes_.end()) {
				return false;
			}
			all_nodes_.erase(iter);
			auto new_edges_set = decltype(all_edges_){};
			for (auto& i : all_edges_) {
				if (*(i.src) == value or *(i.dst) == value) {
					continue; // if the edge has old value, will not add it to new edge set
				}
				new_edges_set.emplace(i);
			}
			all_edges_ = std::move(new_edges_set);
			return true;
		} // O(log(n) + e)
		auto erase_edge(N const& src, N const& dst, E const& weight) -> bool // O(log(n) + e)
		{
			if (!is_node(src) or !is_node(dst)) {
				throw std::runtime_error("Cannot call comp6771::graph<N, E>::erase_edge on src or dst "
				                         "if they don't exist in the graph");
			}
			edge_struct<N, E> value{std::make_shared<N>(src),
			                        std::make_shared<N>(dst),
			                        std::make_shared<E>(weight)};
			auto iter = all_edges_.find(value);
			if (iter != all_edges_.end()) {
				all_edges_.erase(iter);
				return true;
			}
			return false;
		}
		auto erase_edge(iterator i) noexcept -> iterator {
			if (i == end()) {
				return end();
			}
			// use set erase method, easy!
			return iterator(all_edges_.begin(), all_edges_.end(), all_edges_.erase(i.iter_));
		} // Amortised constant time.
		auto erase_edge(iterator i, iterator s) noexcept -> iterator {
			if (s == end()) {
				return end();
			}
			// use set erase method, easy!
			return iterator(all_edges_.begin(), all_edges_.end(), all_edges_.erase(i.iter_, s.iter_));
		} // O(d)
		auto clear() noexcept -> void {
			all_edges_.clear();
			all_nodes_.clear();
		}

		// Accessors
		[[nodiscard]] auto is_node(N const& value) const noexcept -> bool {
			return static_cast<bool>(all_nodes_.find(value) != all_nodes_.end());
		}
		[[nodiscard]] auto empty() const noexcept -> bool {
			return static_cast<bool>(all_nodes_.size() == 0);
		}
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			if (!is_node(src) or !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst "
				                         "node don't exist in the graph");
			}
			return std::any_of(all_edges_.begin(), all_edges_.end(), [src, dst](auto i) {
				return *(i.src) == src and *(i.dst) == dst;
			});
		}
		[[nodiscard]] auto nodes() const noexcept -> std::vector<N> {
			std::vector<N> vec{};
			for (auto& i : all_nodes_) {
				vec.emplace_back(*i);
			}
			return vec;
		}; // O(n)
		[[nodiscard]] auto weights(N const& src, N const& dst) const -> std::vector<E> {
			if (!is_node(src) or !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::weights if src or dst node "
				                         "don't exist in the graph");
			}
			std::vector<E> vec{};
			for (auto& i : all_edges_) {
				if (*(i.src) == src and *(i.dst) == dst) {
					vec.emplace_back(*(i.edge));
				}
			}
			return vec;
		} // O(log(n) + e)
		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) const noexcept
		   -> iterator { // O(log(n) + log(e))
			edge_struct<N, E> value{std::make_shared<N>(src),
			                        std::make_shared<N>(dst),
			                        std::make_shared<E>(weight)};
			return iterator(all_edges_.begin(), all_edges_.end(), all_edges_.find(value));
		}
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			if (!is_node(src)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't "
				                         "exist in the graph");
			}
			auto iter = binary_search(src); // find the edge iterator which has src as from.
			std::vector<N> vec{};
			if (iter == all_edges_.end()) {
				return vec;
			}
			auto tmp = iter;
			vec.emplace_back(*(iter->dst));
			while (tmp != all_edges_.begin() and *(tmp->src) == src) {
				--tmp;
				if (*(tmp->dst) != vec.back()) {
					vec.emplace_back(*(tmp->dst));
				}
			} // O(e)
			std::reverse(vec.begin(), vec.end()); // O(e)
			tmp = ++iter;
			while (tmp != all_edges_.end() and *(tmp->src) == src) {
				if (*(tmp->dst) != vec.back()) {
					vec.emplace_back(*(tmp->dst));
				}
				++tmp;
			} // O(e)
			return vec;
		} // O(log(n) + e)

		// Range access
		[[nodiscard]] auto begin() const noexcept -> iterator {
			return iterator(all_edges_.begin(), all_edges_.end(), all_edges_.begin());
		}
		[[nodiscard]] auto end() const noexcept -> iterator {
			return iterator(all_edges_.begin(), all_edges_.end(), all_edges_.end());
		}

		// Comparisons
		[[nodiscard]] auto operator==(graph const& other) const noexcept -> bool {
			// compare nodes set
			auto it_1 = all_nodes_.begin();
			auto it_2 = other.all_nodes_.begin();
			while (it_1 != all_nodes_.end() and it_2 != other.all_nodes_.end()) {
				if (**it_1 != **it_2) {
					return false;
				}
				it_1++;
				it_2++;
			}
			if (it_1 != all_nodes_.end() or it_2 != other.all_nodes_.end()) {
				return false;
			}
			// compare edges set
			auto it_3 = all_edges_.begin();
			auto it_4 = other.all_edges_.begin();
			while (it_3 != all_edges_.end() and it_4 != other.all_edges_.end()) {
				if (*(it_3->src) != *(it_4->src) and *(it_3->dst) != *(it_4->dst)
				    and *(it_3->edge) != *(it_4->edge))
				{
					return false;
				}
				it_3++;
				it_4++;
			}
			return !static_cast<bool>(it_3 != all_edges_.end() or it_4 != other.all_edges_.end());
		} // O(n + e)

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) noexcept -> std::ostream& {
			auto it_1 = g.all_nodes_.begin();
			auto it_2 = g.all_edges_.begin();
			while (it_1 != g.all_nodes_.end()) {
				os << **it_1 << " (\n";
				while (it_2 != g.all_edges_.end() and *(it_2->src) == **it_1) {
					os << "  " << *(it_2->dst) << " | " << *(it_2->edge) << "\n";
					it_2++;
				}
				os << ")\n";
				it_1++;
			}
			return os;
		}

	private:
		nodes_set<N> all_nodes_{};
		edges_set<N, E> all_edges_{};
		auto inner_insert_edge(N const& src, N const& dst, E const& weight) noexcept -> bool {
			if (find(src, dst, weight) != end()) { // check edge inside
				return false;
			}
			auto node_src = *(all_nodes_.find(src));
			auto node_dst = *(all_nodes_.find(dst));
			auto edge = std::make_shared<E>(weight);
			edge_struct<N, E> value{node_src, node_dst, edge};
			all_edges_.emplace(value);
			return true;
		}
		[[nodiscard]] auto binary_search(N const& value) const noexcept
		   -> decltype(all_edges_.begin()) {
			auto end = --all_edges_.end();
			auto start = all_edges_.begin();
			if (*(start->src) == value or *(end->src) == value) {
				return *(start->src) == value ? start : end;
			}
			auto mid = decltype(all_edges_.begin()){};
			while (*(start->src) < value and *(end->src) > value) {
				mid = start;
				std::advance(mid, std::distance(start, end) / 2);
				if (*(mid->src) < value) {
					start = mid;
					continue;
				}
				if (*(mid->src) > value) {
					end = mid;
					continue;
				}
				return mid;
			}
			return all_edges_.end();
		}
	};
} // namespace gdwg

#endif // GDWG_GRAPH_HPP
