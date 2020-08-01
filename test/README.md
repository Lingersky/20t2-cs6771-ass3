# __TEST README__

This is the TEST items and results of [UNSW COMP6771 Assignment3](https://gitlab.cse.unsw.edu.au/z5254499/20t2-cs6771-ass3) General Directed Weighted Graph.

## Constructors
- _Default Constructor_

```C++
graph();
 ```

|  ITEMS   | RESULTS |
|:--------:|:-------:|
| Is Empty | Passed  |

- _Nodes Initializer List Constructor_

```C++
graph(std::initializer_list<N> il)
```

|     ITEMS     | RESULTS |
|:-------------:|:-------:|
| Correct Graph | Passed  |

- _Nodes Iterator Constructor_

```C++
template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
requires ranges::indirectly_copyable<I, N*>
graph(I first, S last);
```
|     ITEMS     | RESULTS |
|:-------------:|:-------:|
| Correct Graph | Passed  |
| Correct Range | Passed  |

- _**Edges Iterator Constructor**_

```C++
template<ranges::forward_iterator I, ranges::sentinel_for<I> S>
requires ranges::indirectly_copyable<I, value_type*>
graph(I first, S last);
```
|     ITEMS     | RESULTS |
|:-------------:|:-------:|
| Correct Graph | Passed  |
| Correct Range | Passed  |

- 5. _**Move Constructor**_

```C++
graph(graph&& other) noexcept;
```
|          ITEMS           | RESULTS |
|:------------------------:|:-------:|
| Same With Previous Graph | Passed  |
|   other cannot be used   | Passed  |
| Iterator point to *this  | Passed  |
- 6. _**Move Assignment**_

```C++
auto operator=(graph&& other) noexcept -> graph&;
```
|          ITEMS           | RESULTS |
|:------------------------:|:-------:|
| Same With Previous Graph | Passed  |
|   other cannot be used   | Passed  |
- 7. _**Copy Constructor**_

```C++
graph(graph const& other) noexcept
```
|           ITEMS           | RESULTS |
|:-------------------------:|:-------:|
| Same With Previous Graph  | Passed  |
| Independent of each other | Passed  |

- 8. _**Copy Assignment**_

```C++
auto operator=(graph const& other) -> graph&
```
|           ITEMS           | RESULTS |
|:-------------------------:|:-------:|
| Same With Previous Graph  | Passed  |
| Independent of each other | Passed  |


## Modifier

- _**Insert Node**_
```C++
auto insert_node(N const& value) -> bool;
```
|           ITEMS            | RESULTS |
|:--------------------------:|:-------:|
|  Insert New Node Success   | Passed  |
|    Insert Old Node Fail    | Passed  |
| Correct Graph After Insert | Passed  |

- _**Insert Edge**_
```C++
auto insert_edge(N const& src, N const& dst, E const& weight) -> bool;
```
|           ITEMS            | RESULTS |
|:--------------------------:|:-------:|
|  Insert New Edge Success   | Passed  |
|    Insert Old Edge Fail    | Passed  |
|      Correctly Throw       | Passed  |
| Correct Graph After Insert | Passed  |

- _**Replace Node**_
```C++
auto replace_node(N const& old_data, N const& new_data) -> bool
```
|            ITEMS            | RESULTS |
|:---------------------------:|:-------:|
| Replace to New Node Success | Passed  |
|  Replace to Old Node Fail   | Passed  |
|       Correctly Throw       | Passed  |
| Correct Graph After Replace | Passed  |

- _**Merge Replace Node**_
```C++
auto merge_replace_node(N const& old_data, N const& new_data) -> void
```
|           ITEMS           | RESULTS |
|:-------------------------:|:-------:|
|      Correctly Throw      | Passed  |
| Correct Graph After Merge | Passed  |

- _**Erase Node**_
```C++
auto erase_node(N const& value) noexcept -> bool
```
|             ITEMS             | RESULTS |
|:-----------------------------:|:-------:|
|      Erase Node Success       | Passed  |
| Can't Erase Node not in graph | Passed  |
|   Correct Graph After Erase   | Passed  |

- _**Erase Edge**_
```C++
auto erase_edge(N const& src, N const& dst, E const& weight) -> bool
```
|             ITEMS             | RESULTS |
|:-----------------------------:|:-------:|
|        Correctly Throw        | Passed  |
|      Erase Edge Success       | Passed  |
| Can't Erase Edge not in graph | Passed  |
|   Correct Graph After Erase   | Passed  |

- _**Erase Edge By Iterator**_
```C++
auto erase_edge(iterator i) noexcept -> iterator
```
|            ITEMS             | RESULTS |
|:----------------------------:|:-------:|
|      Erase Edge Success      | Passed  |
| Iterator Correct After Erase | Passed  |
|  Erase Graph End Return End  | Passed  |
|  Correct Graph After Erase   | Passed  |

- _**Erase Edge By Iterator Range**_
```C++
auto erase_edge(iterator i, iterator s) noexcept -> iterator
```
|             ITEMS             | RESULTS |
|:-----------------------------:|:-------:|
|      Erase Edge Success       | Passed  |
| Iterator Correct After Erase  | Passed  |
|   Correct Graph After Erase   | Passed  |
| Erase to Graph End Return End | Passed  |


- _**Clear**_
```C++
auto erase_edge(iterator i) noexcept -> iterator
```
|       ITEMS       | RESULTS |
|:-----------------:|:-------:|
| Empty After Erase | Passed  |

## Accessors

- _**Is Node**_
```C++
[[nodiscard]] auto is_node(N const& value) const noexcept -> bool
```
|          ITEMS          | RESULTS |
|:-----------------------:|:-------:|
|   Node in Const Graph   | Passed  |
| Node not in Const Graph | Passed  |

- _**Empty**_
```C++
[[nodiscard]] auto empty() const noexcept -> bool
```
|             ITEMS              | RESULTS |
|:------------------------------:|:-------:|
|   Empty in Const Empty Graph   | Passed  |
| Empty in Const Not Empty Graph | Passed  |

- _**Is Connected**_
```C++
[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool
```
|            ITEMS            | RESULTS |
|:---------------------------:|:-------:|
|       Correctly Throw       | Passed  |
|  Connection in Const Graph  | Passed  |
| Inconnection in Const Graph | Passed  |

- _**Nodes**_
```C++
[[nodiscard]] auto nodes() const noexcept -> std::vector<N>
```
|         ITEMS         | RESULTS |
|:---------------------:|:-------:|
| Not Empty Const Graph | Passed  |
|   Empty Const Graph   | Passed  |
| Return Type Is Vector | Passed  |

- _**Weights**_
```C++
[[nodiscard]] auto weights(N const& src, N const& dst) const -> std::vector<E>
```
|              ITEMS               | RESULTS |
|:--------------------------------:|:-------:|
|         Correctly Throw          | Passed  |
|   Correctly Find Weights Exist   | Passed  |
| Correctly Find Weights Not Exist | Passed  |

- _**Find**_
```C++
[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) const -> iterator
```
|             ITEMS              | RESULTS |
|:------------------------------:|:-------:|
|    Correctly Find The Edge     | Passed  |
| The Return Iterator Is Correct | Passed  |

- _**Connections**_
```C++
[[nodiscard]] auto connections(N const& src) const -> std::vector<N>
```
|                 ITEMS                 | RESULTS |
|:-------------------------------------:|:-------:|
|            Correctly Throw            | Passed  |
|  Correctly Find The Connection Nodes  | Passed  |
| Correctly Handle the Independent Node | Passed  |

## Range Access

- _**Begin**_
```C++
[[nodiscard]] auto begin() const noexcept -> iterator
```
|                     ITEMS                     | RESULTS |
|:---------------------------------------------:|:-------:|
|         Correctly Return The Iterator         | Passed  |
| Correctly Return The Iterator After Modifying | Passed  |

- _**End**_
```C++
[[nodiscard]] auto end() const noexcept -> iterator
```
|             ITEMS             | RESULTS |
|:-----------------------------:|:-------:|
| Correctly Return The Iterator | Passed  |

## Comparisons

- _**Operator==**_
```C++
[[nodiscard]] auto operator==(graph const& other) const noexcept -> bool
```
|               ITEMS                | RESULTS |
|:----------------------------------:|:-------:|
| Correctly Compare Before Modifying | Passed  |
| Correctly Compare After Modifying  | Passed  |

## Iterators

- _**Operator***_
```C++
auto operator*() const noexcept-> ranges::common_tuple<N const, N const, E const>
```
|             ITEMS             | RESULTS |
|:-----------------------------:|:-------:|
| Correctly Get Edge From Graph | Passed  |

- _**Operator--**_
```C++
auto operator--() -> iterator&
```
|                        ITEMS                        | RESULTS |
|:---------------------------------------------------:|:-------:|
| Iterator Point to the Right Element After Decrement | Passed  |

- _**Iterator: Operator==**_
```C++
auto operator==(iterator const& other) const noexcept-> bool
```
|                  ITEMS                  | RESULTS |
|:---------------------------------------:|:-------:|
| Iterator Point to Same Element Is Equal | Passed  |

- _**Iterator Type**_

|        ITEMS        | RESULTS |
|:-------------------:|:-------:|
| Iterator Type Check | Passed  |

## Others Not Tested Separately
```C++
friend auto operator<<(std::ostream& os, graph const& g) noexcept -> std::ostream& //Extractor

auto operator++() noexcept-> iterator& //Iterator Increment
```
Because these function have been widely used in the previous tests. After different tests, they have already shown their correctness.