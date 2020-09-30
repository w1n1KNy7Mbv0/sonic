#ifndef _SURF_WRAPPER_H_
#define _SURF_WRAPPER_H_

#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <valarray>
#include <vector>

#include <surf/include/surf.hpp>

template <typename... ColumnTypes> class SuRFAdapter {
public:
  SuRFAdapter() { size = 0; }

  SuRFAdapter(std::vector<std::tuple<ColumnTypes...>> const& table) {
    size = 0;
    std::vector<std::string> data;
    data.reserve(table.size());
    for(auto i = 0; i < table.size(); i++) {
      std::string key;
      for_each(table[i], [&](auto const& e) { key += e; });
      data.emplace_back(key);
      size++;
    }
    std::sort(data.begin(), data.end());
    surf_index = new surf::SuRF(data);
  }
  ~SuRFAdapter() { delete(surf_index); }

  size_t pointLookup(std::tuple<ColumnTypes...> const& input_tuple) {
    std::string key;
    for_each(input_tuple, [&](auto const& e) { key += e; });

    return surf_index->lookupKey(key);
  }

  size_t getSize() { return size; }

private:
  size_t size;
  surf::SuRF* surf_index;
};

#endif