#ifndef _SONIC_INDEX_H_
#define _SONIC_INDEX_H_

#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <tuple>
#include <vector>

#include "../../helper_functions.h"
#include "sonic_leaf_layer.h"
#include "sonic_node_layer.h"

using namespace std;

namespace {
template <size_t Capacity, size_t BucketSize, size_t ColumnIndex, typename... ColumnTypes>
class SonicIndex {
  static constexpr bool lastLevel = ((ColumnIndex + 2) == sizeof...(ColumnTypes));
  static constexpr size_t numberOfBuckets = (size_t)(Capacity / BucketSize);
  size_t indexSize;

  class NoFurtherLevels {};

  typename conditional<!lastLevel,
                       SonicIndex<Capacity, BucketSize, ColumnIndex + 1, ColumnTypes...>,
                       NoFurtherLevels>::type next_level;

  typename conditional<!lastLevel,
                       SonicLayer<Capacity, BucketSize, ColumnIndex,
                                  CRCHash<tuple_element_t<ColumnIndex, tuple<ColumnTypes...>>>,
                                  tuple<ColumnTypes...>>,
                       NoFurtherLevels>::type node_level;

  typename conditional<lastLevel,
                       SonicTuple<Capacity, BucketSize, ColumnIndex,
                                  CRCHash<tuple_element_t<ColumnIndex, tuple<ColumnTypes...>>>,
                                  tuple<ColumnTypes...>>,
                       NoFurtherLevels>::type leaf_level;

public:
  /////////////////////////////////////////////// LAST LEVEL /////////////////
  template <typename Tuple = tuple<ColumnTypes...>>
  inline pair<SonicLeaf<ColumnIndex, Tuple>&, size_t>
  insert(typename enable_if<lastLevel, Tuple const&>::type input_tuple,
         size_t bucket_number_level_up = numberOfBuckets, size_t hash_key_level_up = 0) {
    indexSize++;
    return leaf_level.insert(input_tuple, bucket_number_level_up);
  }

  template <typename... PrefixColumns>
  inline pair<size_t, size_t> computeMatchedPrefix(
      typename enable_if<lastLevel, tuple<PrefixColumns...> const&>::type input_tuple,
      size_t bucket_number_level_up = numberOfBuckets) const {
    if constexpr(ColumnIndex + 1 <= sizeof...(PrefixColumns)) {
      return leaf_level.template countPrefix<PrefixColumns...>(input_tuple, bucket_number_level_up);
    }
  }

  template <typename Tuple = tuple<ColumnTypes...>>
  inline pair<SonicLeaf<ColumnIndex, Tuple>&, size_t>
  find(typename enable_if<lastLevel, Tuple const&>::type input_tuple,
       size_t bucket_number_level_up = numberOfBuckets) {
    return leaf_level.pointLookup(input_tuple, bucket_number_level_up);
  }

  template <typename... PrefixColumns>
  inline vector<reference_wrapper<const tuple<ColumnTypes...>>>
  prefixLookup(enable_if_t<lastLevel, tuple<PrefixColumns...> const&> input_tuple,
               size_t bucket_number_level_up = numberOfBuckets) const {
    auto resultTupleIndices =
        leaf_level.template prefixLookup<PrefixColumns...>(input_tuple, bucket_number_level_up);

    vector<reference_wrapper<const tuple<ColumnTypes...>>> resultTuples;
    resultTuples.reserve(resultTupleIndices.size());
    for(auto idx : resultTupleIndices) {
      resultTuples.emplace_back(leaf_level.getTupleByIndex(idx));
    }
    return resultTuples;
  }
  ////////////////////////////////////////////////////////////////////////

  ///////////////////////////////// INNER LEVEL //////////////////////////
  template <typename Tuple = tuple<ColumnTypes...>>
  inline pair<SonicNode<tuple_element_t<ColumnIndex, Tuple>>&, size_t>
  insert(enable_if_t<!lastLevel, Tuple const&> input_tuple,
         size_t bucket_number_level_up = numberOfBuckets) {
    auto entry = node_level.insert(input_tuple, bucket_number_level_up);
    entry.first.bucket_number_level_down =
        (next_level.insert(input_tuple, entry.first.bucket_number_level_down)).second;
    return entry;
  }

  template <typename... PrefixColumns>
  inline pair<size_t, size_t>
  computeMatchedPrefix(enable_if_t<!lastLevel, tuple<PrefixColumns...> const&> input_tuple,
                       size_t bucket_number_level_up = numberOfBuckets) const {
    auto entry =
        node_level.template countPrefix<PrefixColumns...>(input_tuple, bucket_number_level_up);

    auto resultCount = entry.second;

    if(entry.second > 0) {
      if constexpr(ColumnIndex + 1 < sizeof...(PrefixColumns)) {
        auto node = node_level.getNodeByIndex(entry.first);
        resultCount = next_level
                          .template computeMatchedPrefix<PrefixColumns...>(
                              input_tuple, node.bucket_number_level_down)
                          .second;
      }
    }
    return {entry.first, resultCount};
  }

  template <typename Tuple = tuple<ColumnTypes...>>
  inline pair<SonicNode<tuple_element_t<ColumnIndex, Tuple>>&, size_t>
  find(enable_if_t<!lastLevel, Tuple const&> input_tuple,
       size_t bucket_number_level_up = numberOfBuckets) {
    auto entry = node_level.pointLookup(input_tuple, bucket_number_level_up);
    entry.second =
        entry.second && next_level.find(input_tuple, entry.first.bucket_number_level_down).second;

    return entry;
  }

  template <typename Tuple = tuple<ColumnTypes...>> size_t pointLookup(Tuple input_tuple) {
    return find(input_tuple).second;
  }

  template <typename... PrefixColumns>
  size_t countPrefix(tuple<PrefixColumns...> const& input_tuple) const {
    return computeMatchedPrefix<PrefixColumns...>(input_tuple).second;
  }

  template <typename... PrefixColumns>
  inline vector<reference_wrapper<const tuple<ColumnTypes...>>>
  prefixLookup(enable_if_t<!lastLevel, tuple<PrefixColumns...> const&> input_tuple,
               size_t bucket_number_level_up = numberOfBuckets) const {
    vector<reference_wrapper<const tuple<ColumnTypes...>>> resultTuples;

    auto resultNodesIndices =
        node_level.template prefixLookup<PrefixColumns...>(input_tuple, bucket_number_level_up);

    for(auto node_index : resultNodesIndices) {
      auto node = node_level.getNodeByIndex(node_index);

      vector<reference_wrapper<const tuple<ColumnTypes...>>> nextLevelResult;

      if constexpr(sizeof...(PrefixColumns) <= ColumnIndex) {
        auto newPrefix = tuple_cat(input_tuple, make_tuple(node.key));
        nextLevelResult = next_level.template prefixLookup<PrefixColumns..., decltype(node.key)>(
            newPrefix, node.bucket_number_level_down);
      } else {
        nextLevelResult = next_level.template prefixLookup<PrefixColumns...>(
            input_tuple, node.bucket_number_level_down);
      }

      resultTuples.insert(resultTuples.end(), nextLevelResult.begin(), nextLevelResult.end());
    }
    return resultTuples;
  }

  template <typename InputSchema = tuple<ColumnTypes...>>
  SonicIndex(vector<InputSchema> const& input_data) {
    for(auto const& input_tuple : input_data) {
      insert<InputSchema>(input_tuple);
    }
  };

  SonicIndex() { indexSize = 0; };

  size_t getSize() const { return indexSize; }

  vector<reference_wrapper<const tuple<ColumnTypes...>>> scan() const {
    if constexpr(lastLevel) {
      return leaf_level.scan();
    } else {
      return next_level.scan();
    }
  }
};
} // namespace
#endif
