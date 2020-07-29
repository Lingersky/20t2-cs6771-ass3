#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <algorithm>
#include <concepts/concepts.hpp>
#include <concepts/type_traits.hpp>
#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <range/v3/iterator.hpp>
#include <range/v3/utility.hpp>
#include <range/v3/utility/common_tuple.hpp>
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
			friend class graph<N, E>;

		public:
			using edges_iterator = typename edges_set<N, E>::const_iterator;
			using value_type = ranges::common_tuple<N, N, E>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;
			// Iterator constructor
			iterator() = default;

			// Iterator source
			explicit iterator(edges_set<N, E> const& pointee, edges_iterator iter) noexcept
			: pointee_{&pointee}
			, iter_{iter} {}
			auto operator*() -> ranges::common_tuple<N const&, N const&, E const&> {
				return ranges::common_tuple<N, N, E>(*(iter_->src), *(iter_->dst), *(iter_->edge));
			}
			// Iterator traversal
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
				if (other.iter_ == other.pointee_->end() or iter_ == pointee_->end()) {
					return static_cast<bool>(other.iter_ == other.pointee_->end()
					                         and iter_ == pointee_->end());
				}
				return static_cast<bool>(*(iter_->src) == *(other.iter_->src)
				                         and *(iter_->dst) == *(other.iter_->dst)
				                         and *(iter_->edge) == *(other.iter_->edge));
			}
			auto get_set_iter() const -> edges_iterator {
				return iter_;
			}

		private:
			edges_set<N, E> const* pointee_;
			edges_iterator iter_;
		};
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
		: all_nodes_{std::move(other.all_nodes_)}
		, all_edges_{std::move(other.all_edges_)} {}

		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				all_edges_ = std::move(other.all_edges_);
				all_nodes_ = std::move(other.all_nodes_);
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
				for (auto& i : other.all_nodes_) {
					auto tmp = std::make_shared<N>(*i);
					all_nodes_.emplace(std::move(tmp));
				}
				for (auto& i : other.all_edges_) {
					auto src = *(all_nodes_.find(i.src));
					auto dst = *(all_nodes_.find(i.dst));
					auto edge = std::make_shared<E>(*(i.edge));
					edge_struct<N, E> edge_struct{src, dst, edge};
					all_edges_.emplace(edge_struct);
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
			all_nodes_.emplace(node);
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
			**(all_nodes_.find(old_data)) = new_data;
			auto tmp1 = all_nodes_;
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
			all_nodes_.erase(all_nodes_.find(old_data));
			auto new_ptr = *(all_nodes_.find(new_data));
			auto new_edges_set = decltype(all_edges_){};
			edge_struct<N, E> tmp{};
			for (auto& i : all_edges_) {
				tmp.src = *(i.src) == old_data ? new_ptr : i.src;
				tmp.dst = *(i.dst) == old_data ? new_ptr : i.dst;
				tmp.edge = i.edge;
				new_edges_set.emplace(tmp);
			}
			all_edges_ = std::move(new_edges_set);
		}
		auto erase_node(N const& value) -> bool {
			auto iter = all_nodes_.find(value);
			if (iter == all_nodes_.end()) {
				return false;
			}
			all_nodes_.erase(iter);
			auto new_edges_set = decltype(all_edges_){};
			for (auto& i : all_edges_) {
				if (*(i.src) == value or *(i.dst) == value) {
					continue;
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
		auto erase_edge(iterator i) -> iterator {
			return iterator(all_edges_, all_edges_.erase(i.get_set_iter()));
		} // Amortised constant time.
		auto erase_edge(iterator i, iterator s) -> iterator {
			return iterator(all_edges_, all_edges_.erase(i.get_set_iter(), s.get_set_iter()));
		} // O(d)
		auto clear() noexcept -> void {
			all_edges_.clear();
			all_nodes_.clear();
		}

		// Accessors
		[[nodiscard]] auto is_node(N const& value) const -> bool { // O(log?(n))
			return static_cast<bool>(all_nodes_.find(value) != all_nodes_.end());
		}
		[[nodiscard]] auto empty() const -> bool {
			return static_cast<bool>(all_nodes_.size() == 0);
		}
		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			return std::any_of(all_edges_.begin(), all_edges_.end(), [src, dst](auto i) {
				return *(i.src) == src and *(i.dst) == dst;
			});
		}
		[[nodiscard]] auto nodes() const -> std::vector<N> {
			std::vector<N> vec{};
			for (auto& i : all_nodes_) {
				vec.emplace_back(*i);
			}
			return vec;
		}; // O(n)
		[[nodiscard]] auto weights(N const& src, N const& dst) const -> std::vector<E> {
			std::vector<E> vec{};
			for (auto& i : all_edges_) {
				if (*(i.src) == src and *(i.dst) == dst) {
					vec.emplace_back(*(i.edge));
				}
			}
			return vec;
		} // O(log?(n)?+?e)
		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) const
		   -> iterator { // O(log?(n)?+?log?(e))
			edge_struct<N, E> value{std::make_shared<N>(src),
			                        std::make_shared<N>(dst),
			                        std::make_shared<E>(weight)};
			return iterator(all_edges_, all_edges_.find(value));
		}
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			std::vector<N> vec{};
			for (auto& i : all_edges_) {
				if (*(i.src) == src) {
					vec.emplace_back(*(i.dst));
				}
			}
			return vec;
		} // O(log?(n)?+?e)

		// Range access
		[[nodiscard]] auto begin() const -> iterator {
			return iterator(all_edges_, all_edges_.begin());
		}
		[[nodiscard]] auto end() const -> iterator {
			return iterator(all_edges_, all_edges_.end());
		}

		// Comparisons
		[[nodiscard]] auto operator==(graph const& other) const -> bool {
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
		} // O(n?+?e)

		// auto change_struct_inner_value(N const& src, N const& dst, E const& weight) -> void {
		// 	for (auto& i : all_edges_) {
		// 		i.src = std::make_shared<N>(src);
		// 		i.dst = std::make_shared<N>(dst);
		// 		i.edge = std::make_shared<N>(weight);
		// 	}
		// }

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
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
		auto inner_insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (find(src, dst, weight) != end()) {
				return false;
			}
			auto node_src = get_node_ptr(src);
			auto node_dst = get_node_ptr(dst);
			auto edge = std::make_shared<E>(weight);
			edge_struct<N, E> value{node_src, node_dst, edge};
			all_edges_.emplace(value);
			return true;
		}
		[[nodiscard]] auto get_node_ptr(N const& value) const -> std::shared_ptr<N> {
			for (auto& i : all_nodes_) {
				if (*i == value) {
					return i;
				}
			}
			return nullptr;
		}
	};
} // namespace gdwg

#endif // GDWG_GRAPH_HPP
