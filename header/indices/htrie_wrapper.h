#ifndef _HTRIE_WRAPPER_H_
#define _HTRIE_WRAPPER_H_

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <tsl/htrie_map.h>

#include "../helper_functions.h"

template <typename... ColumnTypes> class HTrieAdapter {
public:
  HTrieAdapter() { size = 0; }

  HTrieAdapter(std::vector<std::tuple<ColumnTypes...>> const& table) {
    size = 0;
    for(auto i = 0; i < table.size(); i++) {
      insert(table[i]);
    }
  }

  void insert(std::tuple<ColumnTypes...> const& input_tuple) {
    auto key = tupleToString(input_tuple);
    htrie.insert(key, input_tuple);
    size++;
  }

  size_t pointLookup(std::tuple<ColumnTypes...> const& input_tuple) {
    auto key = tupleToString(input_tuple);

    return (htrie.find(key) != htrie.end());
  }

  template <typename... PrefixColumns>
  std::vector<std::tuple<ColumnTypes...>>
  prefixLookup(std::tuple<PrefixColumns...> const& prefix_tuple) const {
    std::vector<std::tuple<ColumnTypes...>> result;
    auto key = tupleToString(prefix_tuple);

    auto it_range = htrie.equal_prefix_range(key);
    for(auto it = it_range.first; it != it_range.second; it++) {
      result.emplace_back(it.value());
    }

    return result;
  }

  template <typename... PrefixColumns>
  size_t countPrefix(std::tuple<PrefixColumns...> const& prefix_tuple) const {
    auto result = 0;
    auto key = tupleToString(prefix_tuple);

    auto it_range = htrie.equal_prefix_range(key);
    for(auto it = it_range.first; it != it_range.second; it++) {
      result++;
    }
    return result;
  }

  auto scan() const {
    std::vector<std::tuple<ColumnTypes...>> results;
    for(auto it = htrie.begin(); it != htrie.end(); it++) {
      results.emplace_back(*it);
    }
    return results;
  }

  size_t getSize() const { return size; }

private:
  size_t size;
  /// only supports char
  tsl::htrie_map<char, std::tuple<ColumnTypes...>, CRCHashHTrie> htrie;
};

#endif