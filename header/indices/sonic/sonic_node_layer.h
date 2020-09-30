#ifndef _SONIC_NODE_H_
#define _SONIC_NODE_H_

#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "../../helper_functions.h"

using namespace std;

template <typename KeyType> struct SonicNode {
  SonicNode() {
    key = DefaultValue<KeyType>()();
    bucket_number_level_down = 0;
    prefix_count = 0;
  }

  KeyType key;
  size_t bucket_number_level_down;
  size_t prefix_count;
};

template <size_t Capacity, size_t BucketSize, size_t ColumnIndex, typename Hash, typename Tuple>
class SonicLayer {
  class FirstColumn {};

  static bool constexpr firstLevel = (ColumnIndex == 0);
  static size_t constexpr numberOfBuckets = (size_t)(Capacity / BucketSize);
  static Hash constexpr hasher = Hash();

  vector<SonicNode<tuple_element_t<ColumnIndex, Tuple>>> index;
  size_t bucket_idx;

  typename conditional<(ColumnIndex > 0), unique_ptr<size_t[]>, FirstColumn>::type patch_bits;
  typename conditional<
      (ColumnIndex > 0),
      unique_ptr<tuple_element_t<std::min(ColumnIndex - 1, tuple_size_v<Tuple> - 1), Tuple>[]>,
      FirstColumn>::type patch_keys;

public:
  typedef tuple_element_t<ColumnIndex, Tuple> KeyType;

  SonicLayer() {
    index.reserve(Capacity);
    for(auto i = 0; i < Capacity; i++) {
      index.emplace_back(SonicNode<tuple_element_t<ColumnIndex, Tuple>>());
    }
    bucket_idx = 0;

    if constexpr(!firstLevel) {
      typedef tuple_element_t<ColumnIndex - 1, Tuple> PreviousColumnType;
      auto defaultValueOfPreviousKey = DefaultValue<PreviousColumnType>()();

      size_t patch_bits_size = ceil(numberOfBuckets / (double)sizeof(size_t));
      patch_bits = make_unique<size_t[]>(patch_bits_size);
      for(auto i = 0; i < patch_bits_size; i++) {
        patch_bits[i] = 0;
      }

      patch_keys = make_unique<PreviousColumnType[]>(Capacity);
      for(auto i = 0; i < Capacity; i++) {
        patch_keys[i] = defaultValueOfPreviousKey;
      }
    }
  }

  const SonicNode<KeyType>& getNodeByIndex(size_t nodeIndex) const { return index[nodeIndex]; }

  pair<SonicNode<KeyType>&, size_t> insert(Tuple const& input_tuple,
                                           size_t bucket_number_level_up = numberOfBuckets) {
    size_t bucket_counter = 0;
    auto hash_key = hasher(get<ColumnIndex>(input_tuple));
    auto hash_idx = hash_key % Capacity;
    auto current_bucket = bucket_number_level_up;

    if constexpr(firstLevel) {
      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          index[hash_idx].prefix_count++;
          return {index[hash_idx], current_bucket};
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
    } else {
      typedef tuple_element_t<ColumnIndex - 1, Tuple> PreviousColumnType;
      hash_idx = bucket_number_level_up != numberOfBuckets ? bucket_number_level_up * BucketSize
                                                           : bucket_idx * BucketSize;
      current_bucket = (size_t)(hash_idx / BucketSize);

      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          auto patch_bucket = (size_t)(hash_idx / BucketSize);
          auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
          auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);

          if((patch_bits[patch_idx] == 0) ||
             ((patch_bits[patch_idx] >> patch_bit_idx == 1) &&
              ((patch_keys[hash_idx] == get<ColumnIndex - 1>(input_tuple)) ||
               (patch_keys[hash_idx] ==
                DefaultValue<tuple_element_t<ColumnIndex - 1, Tuple>>()())))) {
            index[hash_idx].prefix_count++;
            return {index[hash_idx], current_bucket};
          }
        }
        hash_idx = (hash_idx + 1) % Capacity;
        bucket_counter++;
      }
      if(bucket_counter > BucketSize) {
        auto patch_bucket = (size_t)(hash_idx / BucketSize);
        auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
        auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);
        auto old_value = patch_bits[patch_idx];
        patch_bits[patch_idx] =
            old_value | (((patch_bits[patch_idx] >> patch_bit_idx) | 1) << patch_bit_idx);
        patch_keys[hash_idx] = get<ColumnIndex - 1>(input_tuple);
      }

      if(bucket_number_level_up == numberOfBuckets) {
        bucket_idx = (bucket_idx + 1) % numberOfBuckets;
      }
    }

    index[hash_idx].key = get<ColumnIndex>(input_tuple);
    index[hash_idx].bucket_number_level_down = numberOfBuckets;
    index[hash_idx].prefix_count = 1;

    return {index[hash_idx], current_bucket};
  }

  pair<SonicNode<KeyType>&, size_t> pointLookup(Tuple input_tuple, size_t bucket_number_level_up) {
    auto hash_key = hasher(get<ColumnIndex>(input_tuple));
    auto hash_idx = hash_key % Capacity;

    if constexpr(firstLevel) {
      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          return {index[hash_idx], 1};
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
    } else {
      typedef tuple_element_t<ColumnIndex - 1, Tuple> PreviousColumnType;
      hash_idx = bucket_number_level_up * BucketSize;

      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          auto patch_bucket = (size_t)(hash_idx / BucketSize);
          auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
          auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);

          if((patch_bits[patch_idx] == 0) ||
             ((patch_bits[patch_idx] >> patch_bit_idx == 1) &&
              ((patch_keys[hash_idx] == get<ColumnIndex - 1>(input_tuple)) ||
               (patch_keys[hash_idx] ==
                DefaultValue<tuple_element_t<ColumnIndex - 1, Tuple>>()())))) {
            return {index[hash_idx], 1};
          }
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
    }

    return {index[hash_idx], 0};
  }

  template <typename... PrefixColumns>
  pair<size_t, size_t> countPrefix(tuple<PrefixColumns...> input_tuple,
                                   size_t bucket_number_level_up) const {
    auto hash_key = hasher(get<ColumnIndex>(input_tuple));
    auto hash_idx = hash_key % Capacity;

    if constexpr(firstLevel) {
      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          return {hash_idx, index[hash_idx].prefix_count};
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
    } else {
      typedef tuple_element_t<ColumnIndex - 1, Tuple> PreviousColumnType;
      hash_idx = bucket_number_level_up * BucketSize;

      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
          auto patch_bucket = (size_t)(hash_idx / BucketSize);
          auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
          auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);

          if((patch_bits[patch_idx] == 0) ||
             ((patch_bits[patch_idx] >> patch_bit_idx == 1) &&
              ((patch_keys[hash_idx] == get<ColumnIndex - 1>(input_tuple)) ||
               (patch_keys[hash_idx] ==
                DefaultValue<tuple_element_t<ColumnIndex - 1, Tuple>>()())))) {
            return {hash_idx, index[hash_idx].prefix_count};
          }
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
    }

    return {hash_idx, 0};
  }

  template <typename... PrefixColumns>
  std::vector<size_t> prefixLookup(std::tuple<PrefixColumns...> input_tuple,
                                   size_t bucket_number_level_up) const {
    std::vector<size_t> results;
    size_t hash_key;
    size_t hash_idx;

    if constexpr(sizeof...(PrefixColumns) > ColumnIndex) {
      hash_key = hasher(get<ColumnIndex>(input_tuple));
      hash_idx = hash_key % Capacity;
    } else {
      hash_idx = bucket_number_level_up * BucketSize;
    }

    if constexpr(firstLevel) {
      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if constexpr(sizeof...(PrefixColumns) > ColumnIndex) {
          if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
            results.emplace_back(hash_idx);
          }
        } else {
          results.emplace_back(hash_idx);
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
      return results;
    } else {
      typedef tuple_element_t<ColumnIndex - 1, Tuple> PreviousColumnType;
      hash_idx = bucket_number_level_up * BucketSize;

      while(index[hash_idx].key != DefaultValue<KeyType>()()) {
        if constexpr(sizeof...(PrefixColumns) > ColumnIndex) {
          if(index[hash_idx].key == get<ColumnIndex>(input_tuple)) {
            auto patch_bucket = (size_t)(hash_idx / BucketSize);
            auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
            auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);

            if((patch_bits[patch_idx] == 0) ||
               ((patch_bits[patch_idx] >> patch_bit_idx == 1) &&
                ((patch_keys[hash_idx] == get<ColumnIndex - 1>(input_tuple)) ||
                 (patch_keys[hash_idx] ==
                  DefaultValue<tuple_element_t<ColumnIndex - 1, Tuple>>()())))) {
              results.emplace_back(hash_idx);
            }
          }
        } else {
          auto patch_bucket = (size_t)(hash_idx / BucketSize);
          auto patch_idx = (size_t)(patch_bucket / (double)sizeof(size_t));
          auto patch_bit_idx = sizeof(size_t) - patch_bucket % sizeof(size_t);

          if((patch_bits[patch_idx] == 0) ||
             ((patch_bits[patch_idx] >> patch_bit_idx == 1) &&
              ((patch_keys[hash_idx] == get<ColumnIndex - 1>(input_tuple)) ||
               (patch_keys[hash_idx] ==
                DefaultValue<tuple_element_t<ColumnIndex - 1, Tuple>>()())))) {
            results.emplace_back(hash_idx);
          }
        }
        hash_idx = (hash_idx + 1) % Capacity;
      }
      return results;
    }
    return results;
  }
};

#endif