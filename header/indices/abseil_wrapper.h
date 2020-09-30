#ifndef _ABSEIL_WRAPPER_H_
#define _ABSEIL_WRAPPER_H_

#include <array>
#include <cstddef>
#include <valarray>
#include <vector>

#include <absl/container/flat_hash_set.h>

template <typename... ColumnTypes> class AbseilAdapter {
public:
  AbseilAdapter() { size = 0; }

  AbseilAdapter(std::vector<std::tuple<ColumnTypes...>> const& table) {
    size = 0;
    for(auto i = 0; i < table.size(); i++) {
      insert(table[i]);
      size++;
    }
  }

  void insert(std::tuple<ColumnTypes...> const& input_tuple) {
    hash_table.insert(input_tuple);
    size++;
  }

  size_t pointLookup(std::tuple<ColumnTypes...> const& input_tuple) {
    return hash_table.contains(input_tuple);
  }

  template <typename... PrefixColumns>
  std::vector<std::tuple<ColumnTypes...>>
  prefixLookup(std::tuple<PrefixColumns...> const& prefix_tuple) {
    std::vector<std::tuple<ColumnTypes...>> result;
    auto range_it = hash_table.equal_range(prefix_tuple);
    for(auto it = range_it.first; it != range_it.second; it++) {
      result.emplace_back(*it);
    }

    return result;
  }

  template <typename... PrefixColumns>
  size_t countPrefix(std::tuple<PrefixColumns...> const& prefix_tuple) {
    size_t result = 0;

    auto range_it = hash_table.equal_range(prefix_tuple);
    for(auto it = range_it.first; it != range_it.second; it++) {
      result++;
    }

    return result;
  }

  size_t getSize() { return size; }

private:
  size_t size;
  absl::flat_hash_set<std::tuple<ColumnTypes...>, CRCHash<std::tuple<ColumnTypes...>>> hash_table;
};

#endif