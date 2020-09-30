#ifndef _SONIC_LEAF_
#define _SONIC_LEAF_

#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <tuple>
#include <vector>

#include "../../helper_functions.h"

using namespace std;

template <size_t KeyIndex, typename Tuple> struct SonicLeaf {
  typedef std::tuple_element_t<KeyIndex, Tuple> KeyType;

  SonicLeaf() {
    key = DefaultValue<KeyType>()();
    data_tuple = tupleDefaultValue<Tuple>();
  }

  KeyType key;
  Tuple data_tuple;
};

template <size_t Capacity, size_t BucketSize, size_t ColumnIndex, typename Hash, typename Tuple>
class SonicTuple {
  vector<SonicLeaf<ColumnIndex, Tuple>> index;
  static Hash constexpr hasher = Hash();
  size_t bucket_idx;
  static size_t constexpr numberOfBuckets = (size_t)(Capacity / BucketSize);

  template <size_t... I> auto getSubTuple(Tuple const& input_tuple, index_sequence<I...>) const {
    return make_tuple(get<I>(input_tuple)...);
  }

public:
  typedef tuple_element_t<ColumnIndex, Tuple> KeyType;

  SonicTuple() {
    index.reserve(Capacity);
    for(auto i = 0; i < Capacity; i++) {
      index.emplace_back(SonicLeaf<ColumnIndex, Tuple>());
    }
    bucket_idx = 0;
  }

  const Tuple& getTupleByIndex(size_t tupleIndex) const { return index[tupleIndex].data_tuple; }

  vector<reference_wrapper<const Tuple>> scan() const {
    vector<reference_wrapper<const Tuple>> results;

    for(auto i = 0; i < Capacity; i++) {
      if(index[i].key != DefaultValue<KeyType>()()) {
        results.emplace_back(index[i].data_tuple);
      }
    }
    return results;
  }

  pair<SonicLeaf<ColumnIndex, Tuple>&, size_t>
  insert(Tuple const& input_tuple, size_t bucket_number_level_up = numberOfBuckets) {
    size_t bucket_counter = 0;
    auto hash_key = hasher(get<ColumnIndex>(input_tuple));
    auto hash_idx = hash_key % Capacity;
    auto current_bucket = bucket_number_level_up; // Later Update

    if constexpr(ColumnIndex > 0) {
      hash_idx = bucket_number_level_up != numberOfBuckets ? bucket_number_level_up * BucketSize
                                                           : bucket_idx * BucketSize;
      current_bucket = (size_t)(hash_idx / BucketSize);
    }

    while(index[hash_idx].key != DefaultValue<KeyType>()()) {
      hash_idx = (hash_idx + 1) % Capacity;
      bucket_counter++;
    }

    index[hash_idx].key = get<ColumnIndex>(input_tuple);
    index[hash_idx].data_tuple = input_tuple;

    if(bucket_number_level_up == numberOfBuckets) {
      bucket_idx = (bucket_idx + 1) % numberOfBuckets;
      return {index[hash_idx], (size_t)(hash_idx / BucketSize)};
    } else {
      return {index[hash_idx], bucket_number_level_up};
    }
  }

  pair<SonicLeaf<ColumnIndex, Tuple>&, size_t> pointLookup(Tuple const& input_tuple,
                                                           size_t bucket_number_level_up) {
    auto bucket_counter = 1;
    size_t hash_idx;
    if constexpr(ColumnIndex == 0) {
      hash_idx = hasher(get<ColumnIndex>(input_tuple)) % Capacity;
    } else {
      hash_idx = bucket_number_level_up * BucketSize;
    }

    while(index[hash_idx].key != DefaultValue<KeyType>()()) {
      if(index[hash_idx].data_tuple == input_tuple) {
        return {index[hash_idx], 1};
      }
      hash_idx = (hash_idx + 1) % Capacity;

      if constexpr(ColumnIndex == 0) {
        if((index[hash_idx].key == DefaultValue<KeyType>()()) &&
           (bucket_counter < numberOfBuckets)) {
          hash_idx = (bucket_counter * BucketSize) % Capacity;
          bucket_counter++;
        }
      }
    }
    return {index[hash_idx], 0};
  }

  template <typename... PrefixColumns>
  pair<size_t, size_t> countPrefix(tuple<PrefixColumns...> const& input_tuple,
                                   size_t bucket_number_level_up) const {
    auto hash_key = hasher(get<ColumnIndex>(input_tuple));
    auto result = 0;
    auto bucket_counter = 1;
    size_t hash_idx;
    if constexpr(ColumnIndex == 0) {
      hash_idx = hash_key % Capacity;
    } else {
      hash_idx = bucket_number_level_up * BucketSize;
    }

    while(index[hash_idx].key != DefaultValue<KeyType>()()) {
      if(getSubTuple(index[hash_idx].data_tuple, make_index_sequence<sizeof...(PrefixColumns)>()) ==
         input_tuple) {
        result++;
      }
      hash_idx = (hash_idx + 1) % Capacity;

      if constexpr(ColumnIndex == 0) {
        if((index[hash_idx].key == DefaultValue<KeyType>()()) &&
           (bucket_counter < numberOfBuckets)) {
          hash_idx = (bucket_counter * BucketSize) % Capacity;
          bucket_counter++;
        }
      }
    }
    return {hash_idx, result};
  }

  template <typename... PrefixColumns>
  vector<size_t> prefixLookup(tuple<PrefixColumns...> const& input_tuple,
                              size_t bucket_number_level_up) const {
    vector<size_t> results;
    size_t hash_idx;
    auto bucket_counter = 1;
    if constexpr(ColumnIndex == 0) {
      hash_idx = hasher(get<ColumnIndex>(input_tuple)) % Capacity;
    } else {
      hash_idx = bucket_number_level_up * BucketSize;
    }

    while(index[hash_idx].key != DefaultValue<KeyType>()()) {
      if(getSubTuple(index[hash_idx].data_tuple, make_index_sequence<sizeof...(PrefixColumns)>()) ==
         input_tuple) {
        results.emplace_back(hash_idx);
      }
      hash_idx = (hash_idx + 1) % Capacity;
    }
    return results;
  }
};

#endif