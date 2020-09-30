
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../helper_functions.h"

using namespace std;

template <template <typename...> typename LayerTable, typename KeyListHead, typename... KeyListTail>
class HierarchicalMap {
  class LeafLevel {
    tuple<KeyListTail...> value;

  public:
    vector<tuple<KeyListTail...>> enumerate() const { return {value}; }
    auto insert(tuple<KeyListTail...> value) { this->value = value; }
    auto find(tuple<KeyListTail...> value) { return value; }
  };
  conditional_t<(sizeof...(KeyListTail) > 1),
                LayerTable<KeyListHead, HierarchicalMap<LayerTable, KeyListTail...>>,
                LayerTable<KeyListHead, LeafLevel>>
      table;

  size_t indexSize;

public:
  template <typename Tuple, size_t... Indices>
  auto tail(Tuple t, index_sequence<Indices...>) const {
    return tuple{get<Indices + 1>(t)...};
  }

  template <typename PrefixKeyListHead, typename... PrefixKeyListTail,
            typename = enable_if_t<(sizeof...(PrefixKeyListTail) > 0)>*>
  auto prefixLookup(tuple<PrefixKeyListHead, PrefixKeyListTail...> const& t) const {
    vector<tuple<KeyListHead, KeyListTail...>> result;
    try {
      for(auto const& resultFromNextLevel :
          table.at(get<0>(t)).template prefixLookup<PrefixKeyListTail...>(
              tail(t, make_index_sequence<sizeof...(PrefixKeyListTail)>()))) {
        result.push_back(tuple_cat(tuple{get<0>(t)}, resultFromNextLevel));
      }
    } catch(...) {
    }
    return result;
  }

  template <typename PrefixKeyListHead, typename... PrefixKeyListTail,
            typename = enable_if_t<(sizeof...(PrefixKeyListTail) == 0)>*>
  auto prefixLookup(tuple<PrefixKeyListHead> const& t) const {
    vector<tuple<KeyListHead, KeyListTail...>> result;
    try {
      for(auto const& resultFromNextLevel : table.at(get<0>(t)).enumerate()) {
        result.push_back(tuple_cat(tuple{get<0>(t)}, resultFromNextLevel));
      }
    } catch(...) {
    }
    return result;
  }

  template <typename PrefixKeyListHead, typename... PrefixKeyListTail,
            typename = enable_if_t<(sizeof...(PrefixKeyListTail) > 0)>*>
  auto countPrefix(tuple<PrefixKeyListHead, PrefixKeyListTail...> const& t) const {
    auto result = 0;
    try {
      result = table.at(get<0>(t)).template countPrefix<PrefixKeyListTail...>(
          tail(t, make_index_sequence<sizeof...(PrefixKeyListTail)>()));
    } catch(...) {
    }
    return result;
  }

  template <typename PrefixKeyListHead, typename... PrefixKeyListTail,
            typename = enable_if_t<(sizeof...(PrefixKeyListTail) == 0)>*>
  auto countPrefix(tuple<PrefixKeyListHead> const& t) const {
    auto result = 0;
    try {
      result = table.at(get<0>(t)).enumerate().size();
    } catch(...) {
    }
    return result;
  }

  auto find(tuple<KeyListHead, KeyListTail...> const& t) {
    tuple<KeyListHead, KeyListTail...> result;
    try {
      auto nextLevelResult =
          table.at(get<0>(t)).find(tail(t, make_index_sequence<sizeof...(KeyListTail)>()));
      if(t == tuple_cat(tuple{get<0>(t)}, nextLevelResult)) {
        result = tuple_cat(tuple{get<0>(t)}, nextLevelResult);
      }
    } catch(...) {
    }
    return result;
  }

  auto pointLookup(tuple<KeyListHead, KeyListTail...> const& t) {
    return (find(t) != tupleDefaultValue<tuple<KeyListHead, KeyListTail...>>());
  }

  auto enumerate() const {
    vector<tuple<KeyListHead, KeyListTail...>> result;
    for(auto& [key, value] : table) {
      for(auto& resultFromNextLevel : table.at(key).enumerate()) {
        result.push_back(tuple_cat(tuple{key}, resultFromNextLevel));
      }
    }
    return result;
  }

  auto insert(tuple<KeyListHead, KeyListTail...> const& t) {
    table[get<0>(t)].insert(tail(t, make_index_sequence<sizeof...(KeyListTail)>()));
    indexSize++;
  }

  auto scan() const { return enumerate(); }

  HierarchicalMap() { indexSize = 0; }

  HierarchicalMap(vector<tuple<KeyListHead, KeyListTail...>> const& table) {
    indexSize = 0;
    for(auto t : table) {
      insert(t);
    }
  }

  size_t getSize() const { return indexSize; }
};
