#ifndef _SONIC_WRAPPER_H_
#define _SONIC_WRAPPER_H_

#include <utility>

#include "sonic/sonic_index.h"

#include <iostream>

using namespace std;

namespace {

template <typename Index, typename TupleOfTypesInTotalOrder,
          size_t... OffsetsOfStoredTuplesInTotalOrder>
class SonicToTotalOrderLookupAdapter {
public:
  Index index;
  SonicToTotalOrderLookupAdapter() {}

  SonicToTotalOrderLookupAdapter(Index&& index,
                                 index_sequence<OffsetsOfStoredTuplesInTotalOrder...> const& = {})
      : index(index) {}

  template <size_t... PrefixIndices>
  vector<TupleOfTypesInTotalOrder>
  lookupByPrefixWithIndices(TupleOfTypesInTotalOrder const& t,
                            index_sequence<PrefixIndices...> const) const {
    auto resultFromIndex = index.template prefixLookup<decltype(
        get<get<PrefixIndices>(tuple{OffsetsOfStoredTuplesInTotalOrder...})>(t))...>(
        tuple{get<get<PrefixIndices>(tuple{OffsetsOfStoredTuplesInTotalOrder...})>(t)...});
    vector<TupleOfTypesInTotalOrder> resultTuples;
    resultTuples.reserve(resultFromIndex.size());

    for(auto tupleFromIndexRef : resultFromIndex) {
      TupleOfTypesInTotalOrder result = t;
      tie(get<OffsetsOfStoredTuplesInTotalOrder>(result)...) = tupleFromIndexRef.get();
      resultTuples.emplace_back(result);
    }
    return resultTuples;
  }

  template <size_t PrefixLength, typename Indices = make_index_sequence<PrefixLength>>
  vector<TupleOfTypesInTotalOrder>
  lookupByPrefixWithLength(TupleOfTypesInTotalOrder const& t) const {
    return lookupByPrefixWithIndices(t, Indices{});
  }

  template <size_t... ViablePrefixLengthValues>
  vector<TupleOfTypesInTotalOrder> lookupByPrefixGivenViablePrefixLengthValues(
      TupleOfTypesInTotalOrder const& t, size_t const prefixLength,
      index_sequence<ViablePrefixLengthValues...> const&) const {

    vector<TupleOfTypesInTotalOrder> r;

    ((r = ((ViablePrefixLengthValues + 1 == prefixLength)
               ? lookupByPrefixWithLength<ViablePrefixLengthValues + 1>(t)
               : r)),
     ...);
    return r;
  }

  template <typename ViablePrefixLengthValuesSequence =
                make_index_sequence<sizeof...(OffsetsOfStoredTuplesInTotalOrder)>>
  vector<TupleOfTypesInTotalOrder>
  lookupByPrefixAndScatterIntoTuple(TupleOfTypesInTotalOrder const& t,
                                    size_t const prefixLength) const {
    return lookupByPrefixGivenViablePrefixLengthValues(t, prefixLength,
                                                       ViablePrefixLengthValuesSequence{});
  }

  size_t getIndexSize() const { return index.getSize(); }

  template <typename TupleSchema> void insertIntoIndex(TupleSchema const& input_tuple) {
    index.insert(input_tuple);
  }

  template <size_t... PrefixIndices>
  size_t countByPrefixWithIndices(TupleOfTypesInTotalOrder const& t,
                                  index_sequence<PrefixIndices...> const) const {
    return index.template countPrefix<
        std::tuple_element_t<get<PrefixIndices>(tuple{OffsetsOfStoredTuplesInTotalOrder...}),
                             TupleOfTypesInTotalOrder>...>(
        tuple{(get<get<PrefixIndices>(tuple{OffsetsOfStoredTuplesInTotalOrder...})>(t))...});
  }

  template <size_t PrefixLength, typename Indices = make_index_sequence<PrefixLength>>
  size_t countByPrefixWithLength(TupleOfTypesInTotalOrder const& t) const {
    return countByPrefixWithIndices(t, Indices{});
  }

  template <size_t... ViablePrefixLengthValues>
  size_t countByPrefixGivenViablePrefixLengthValues(
      TupleOfTypesInTotalOrder const& t, size_t const prefixLength,
      index_sequence<ViablePrefixLengthValues...> const&) const {

    size_t r;

    ((r = ((ViablePrefixLengthValues + 1 == prefixLength)
               ? countByPrefixWithLength<ViablePrefixLengthValues + 1>(t)
               : r)),
     ...);
    return r;
  }

  template <typename ViablePrefixLengthValuesSequence =
                make_index_sequence<sizeof...(OffsetsOfStoredTuplesInTotalOrder)>>
  size_t countPrefix(TupleOfTypesInTotalOrder const& t, size_t const prefixLength) const {
    return countByPrefixGivenViablePrefixLengthValues(t, prefixLength,
                                                      ViablePrefixLengthValuesSequence{});
  }

  vector<TupleOfTypesInTotalOrder> scanIndex(TupleOfTypesInTotalOrder const& t) const {
    auto resultFromIndex = index.scan();
    vector<TupleOfTypesInTotalOrder> resultTuples;
    resultTuples.reserve(resultFromIndex.size());

    for(auto tupleFromIndexRef : resultFromIndex) {
      TupleOfTypesInTotalOrder result = t;
      tie(get<OffsetsOfStoredTuplesInTotalOrder>(result)...) = tupleFromIndexRef.get();
      resultTuples.emplace_back(result);
    }

    return resultTuples;
  }
};
} // namespace

#endif