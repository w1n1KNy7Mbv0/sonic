#ifndef _ROBIN_WRAPPER_H_
#define _ROBIN_WRAPPER_H_

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <tsl/robin_set.h>

#include "../helper_functions.h"

template <typename... ColumnTypes> class RobinAdapter {
public:
  RobinAdapter() { size = 0; }

  RobinAdapter(std::vector<std::tuple<ColumnTypes...>> const& table) {
    size = 0;
    for(auto i = 0; i < table.size(); i++) {
      insert(table[i]);
    }
  }

  void insert(std::tuple<ColumnTypes...> const& input_tuple) {
    robin_set.insert(input_tuple);
    size++;
  }

  size_t pointLookup(std::tuple<ColumnTypes...> const& input_tuple) {
    return (robin_set.find(input_tuple) != robin_set.end());
  }

  size_t getSize() { return size; }

private:
  size_t size;
  tsl::robin_set<std::tuple<ColumnTypes...>, CRCHash<std::tuple<ColumnTypes...>>> robin_set;
};

#endif