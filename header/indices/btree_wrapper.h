#ifndef _BTREE_WRAPPER_H_
#define _BTREE_WRAPPER_H_

#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <valarray>
#include <vector>

#include <tlx/container/btree_set.hpp>

#include "../helper_functions.h"

template <typename... ColumnTypes> class BTreeAdapter {
public:
  BTreeAdapter() { size = 0; }

  BTreeAdapter(std::vector<std::tuple<ColumnTypes...>> const& table) {
    size = 0;
    for(auto i = 0; i < table.size(); i++) {
      insert(table[i]);
    }
  }

  void insert(std::tuple<ColumnTypes...> const& input_tuple) {
    btree.insert(input_tuple);
    size++;
  }

  size_t pointLookup(std::tuple<ColumnTypes...> const& input_tuple) const {
    return btree.exists(input_tuple);
  }

  template <typename... PrefixColumns>
  std::vector<std::tuple<ColumnTypes...>>
  prefixLookup(std::tuple<PrefixColumns...> const& prefix_tuple) const {
    std::vector<std::tuple<ColumnTypes...>> result;

    auto lowerBound = btree.lower_bound(
        prefixLowerBound(prefix_tuple, make_index_sequence<sizeof...(PrefixColumns)>()));
    auto upperBound = btree.upper_bound(
        prefixUpperBound(prefix_tuple, make_index_sequence<sizeof...(PrefixColumns)>()));

    for(auto it = lowerBound; it != upperBound; it++) {
      result.emplace_back(*it);
    }

    return result;
  }

  template <typename... PrefixColumns>
  size_t countPrefix(std::tuple<PrefixColumns...> const& prefix_tuple) const {
    auto result = 0;

    auto lowerBound = btree.lower_bound(
        prefixLowerBound(prefix_tuple, make_index_sequence<sizeof...(PrefixColumns)>()));
    auto upperBound = btree.upper_bound(
        prefixUpperBound(prefix_tuple, make_index_sequence<sizeof...(PrefixColumns)>()));

    for(auto it = lowerBound; it != upperBound; it++) {
      result++;
    }

    return result;
  }

  size_t getSize() const { return size; }

  auto scan() const {
    std::vector<std::tuple<ColumnTypes...>> result;
    result.reserve(size);
    for(auto it = btree.begin(); it != btree.end(); it++) {
      result.emplace_back(*it);
    }
    return result;
  }

private:
  size_t size;
  tlx::btree_set<std::tuple<ColumnTypes...>> btree;

  template <typename Tuple, size_t... I>
  auto prefixLowerBound(Tuple t, index_sequence<I...>) const {
    auto result = tupleDefaultValue<tuple<ColumnTypes...>>();
    tie(get<I>(result)...) = t;
    return result;
  }
  template <typename Tuple, size_t... I>
  auto prefixUpperBound(Tuple t, index_sequence<I...>) const {
    auto result = tupleMaxValue<tuple<ColumnTypes...>>();
    tie(get<I>(result)...) = t;
    return result;
  }
};

#endif