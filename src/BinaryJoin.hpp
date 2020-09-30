#pragma once
#include "../header/helper_functions.h"
#include "../header/relation.h"
#include <tao/seq/concatenate.hpp>
#include <tao/seq/difference.hpp>
#include <tao/seq/make_integer_range.hpp>
#include <tao/seq/make_integer_sequence.hpp>
#include <tao/seq/map.hpp>
#include <tao/seq/select.hpp>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
namespace wcoj::utilities {
template <typename, typename> struct project;
template <std::size_t... Ns, typename M> struct project<tao::seq::index_sequence<Ns...>, M> {
  using type = std::tuple<typename std::tuple_element<Ns, std::remove_reference_t<M>>::type...>;
  auto operator()(M const& v) { return std::tuple{std::get<Ns>(v)...}; }
};

template <typename S, typename M> using project_t = typename project<S, M>::type;

template <typename, typename, typename> struct scatter;
template <std::size_t... Ns, std::size_t... InNs, typename M>
struct scatter<tao::seq::index_sequence<Ns...>, tao::seq::index_sequence<InNs...>, M> {
  template <typename... Inputs> void apply(M& output, std::tuple<Inputs...> const& input) {
    ((std::get<Ns>(output) = std::get<InNs>(input)), ...);
  }
};

template <template <typename> typename hash = std::hash> class hash_tuple {
public:
  template <typename... Elements, size_t... Indices>
  size_t foldHash(const std::tuple<Elements...>& tuple, std::index_sequence<Indices...>) const {
    return hash<int>()(
        ((hash<std::tuple_element_t<Indices, std::tuple<Elements...>>>()(std::get<Indices>(tuple))
          << 1) ^
         ...));
  }
  template <typename... Elements> size_t operator()(const std::tuple<Elements...>& tuple) const {
    return foldHash(tuple, std::make_index_sequence<sizeof...(Elements)>());
  }
};

} // namespace wcoj::utilities

template <size_t Capacity, size_t /*bucketsize*/, size_t /*ColumnIndex*/, typename... ColumnTypes>
class UnorderedMapIndex {
public:
  using IndexedTupleType = std::tuple<ColumnTypes...>;
};

template <typename Index, typename TupleOfTypesInTotalOrder,
          size_t... OffsetsOfStoredTuplesInTotalOrder>
class UnorderedMapIndexAdapter {
public:
  using IndexedTupleType = typename Index::IndexedTupleType;
  using TupleOfTypesInTotalOrderType = TupleOfTypesInTotalOrder;

private:
  std::vector<IndexedTupleType> tuples = {};

public:
  auto const& getTuples() const { return tuples; }
  std::bool_constant<true> PerformChainOfBinaryJoins{};
  void insertIntoIndex(typename Index::IndexedTupleType const& tuple) { tuples.push_back(tuple); }
};

template <typename T, typename = void> struct RequestsChainOfBinaryJoins : std::false_type {};
template <typename T>
struct RequestsChainOfBinaryJoins<
    T, std::void_t<decltype(std::declval<T>().wrappedIndex.PerformChainOfBinaryJoins)>>
    : std::true_type {};
using namespace tao;
class NoOpOperator {
public:
  using ExportedAttributesByTotalOrderID = std::index_sequence<0, 1, 2>;
};

template <typename Adapter, typename InputSchema, typename AttributesInTotalOrder,
          typename AttributesOrder, typename InputOperator = NoOpOperator,
          typename ExportedAttributes = AttributesInTotalOrder>
class JoinOperator {
  using RelationType = Relation<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder>;
  using OutputType = typename Adapter::TupleOfTypesInTotalOrderType;
  RelationType const& r;
  InputOperator input;
  using PayloadAttributes =
      tao::seq::difference_t<ExportedAttributes,
                             typename InputOperator::ExportedAttributesByTotalOrderID>;
  using IndexedAttributes = seq::difference_t<AttributesInTotalOrder, PayloadAttributes>;

  std::unordered_multimap<
      wcoj::utilities::project_t<IndexedAttributes, typename Adapter::TupleOfTypesInTotalOrderType>,
      wcoj::utilities::project_t<PayloadAttributes, typename Adapter::TupleOfTypesInTotalOrderType>,
      wcoj::utilities::hash_tuple<CRCHash>>
      hashTable;

public:
  using ExportedAttributesByTotalOrderID = ExportedAttributes;
  JoinOperator(Relation<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder> const& r)
      : r(r) {}

  JoinOperator(Relation<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder> const& r,
               InputOperator const& input)
      : r(r), input(input) {
    for(auto& tuple : r.wrappedIndex.getTuples()) {
      hashTable.insert(
          {wcoj::utilities::project<tao::seq::make_index_sequence<IndexedAttributes::size()>,
                                    decltype(tuple)>()(tuple),
           wcoj::utilities::project<
               tao::seq::make_index_range<IndexedAttributes::size(),
                                          IndexedAttributes::size() + PayloadAttributes::size()>,
               decltype(tuple)>()(tuple)});
    }
  }

  template <typename FirstAttributeIDs, typename SecondAttributeIDs>
  using mergeAttributeIDs =
      tao::seq::concatenate_t<FirstAttributeIDs,
                              tao::seq::difference_t<SecondAttributeIDs, FirstAttributeIDs>>;

  template <typename OtherOperator> auto operator+(OtherOperator const& input) {
    return JoinOperator<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder,
                        OtherOperator,
                        mergeAttributeIDs<typename OtherOperator::ExportedAttributesByTotalOrderID,
                                          AttributesInTotalOrder>>(r, input);
  };

private:
  std::optional<OutputType> nextOutput = {};
  std::pair<typename decltype(hashTable)::iterator, typename decltype(hashTable)::iterator> matches;

public:
  bool hasNext() {
    OutputType result;
    while((matches.first == matches.second) && input.hasNext()) {
      result = input.next();
      auto key = wcoj::utilities::project<IndexedAttributes, decltype(result)>()(result);
      matches = hashTable.equal_range(key);
    }

    if(matches.first != matches.second) {
      wcoj::utilities::project_t<PayloadAttributes, typename Adapter::TupleOfTypesInTotalOrderType>
          match = (matches.first++)->second;
      wcoj::utilities::scatter<PayloadAttributes,
                               tao::seq::make_index_sequence<PayloadAttributes::size()>,
                               decltype(result)>()
          .apply(result, match);
      nextOutput = result;
      return true;
    }
    return false;
  }

  auto const next() { return *nextOutput; }
};

template <typename Adapter, typename InputSchema, typename AttributesInTotalOrder,
          typename AttributesOrder, typename InputOperator = size_t>
class ScanOperator {
  Relation<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder> const& r;
  typename std::vector<typename Adapter::IndexedTupleType>::const_iterator it;

public:
  using ExportedAttributesByTotalOrderID = AttributesInTotalOrder;
  ScanOperator(Relation<Adapter, InputSchema, AttributesInTotalOrder, AttributesOrder> const& r,
               InputOperator const& input = {})
      : r(r), it(r.wrappedIndex.getTuples().begin()) {}

  auto hasNext() const { return it != r.wrappedIndex.getTuples().end(); }
  typename Adapter::TupleOfTypesInTotalOrderType const next() {
    auto result = typename Adapter::TupleOfTypesInTotalOrderType();
    auto tuple = *(it++);
    wcoj::utilities::scatter<tao::seq::map_t<AttributesOrder, AttributesInTotalOrder>,
                             tao::seq::make_index_sequence<AttributesInTotalOrder::size()>,
                             decltype(result)>()
        .apply(result, tuple);
    return result;
  }
};

template <typename OutputTuple, typename ProbeRelation, typename... BuildRelations>
std::vector<OutputTuple>
join3(std::tuple<ProbeRelation, BuildRelations...> const& tables,
      std::enable_if_t<RequestsChainOfBinaryJoins<decltype(std::get<0>(tables))>::value>* = {}) {
  auto plan = (JoinOperator(get<BuildRelations>(tables)) + ... + ScanOperator(std::get<0>(tables)));
  std::vector<OutputTuple> result;
  while(plan.hasNext()) {
    result.push_back(plan.next());
  }
  return result;
}

template <typename OutputTuple, typename ProbeRelation, typename... BuildRelations>
std::vector<OutputTuple>
join4(std::tuple<ProbeRelation, BuildRelations...> const& tables,
      std::enable_if_t<RequestsChainOfBinaryJoins<decltype(std::get<0>(tables))>::value>* = {}) {
  auto plan = (JoinOperator(get<BuildRelations>(tables)) + ... + ScanOperator(std::get<0>(tables)));
  std::vector<OutputTuple> result;
  while(plan.hasNext()) {
    result.push_back(plan.next());
  }
  return result;
}

template <typename OutputTuple, typename ProbeRelation, typename... BuildRelations>
std::vector<OutputTuple>
join5(std::tuple<ProbeRelation, BuildRelations...> const& tables,
      std::enable_if_t<RequestsChainOfBinaryJoins<decltype(std::get<0>(tables))>::value>* = {}) {
  auto plan = (JoinOperator(get<BuildRelations>(tables)) + ... + ScanOperator(std::get<0>(tables)));
  std::vector<OutputTuple> result;
  while(plan.hasNext()) {
    result.push_back(plan.next());
  }
  return result;
}

template <typename OutputTuple, typename ProbeRelation, typename... BuildRelations>
std::vector<OutputTuple>
join6(std::tuple<ProbeRelation, BuildRelations...> const& tables,
      std::enable_if_t<RequestsChainOfBinaryJoins<decltype(std::get<0>(tables))>::value>* = {}) {
  auto plan = (JoinOperator(get<BuildRelations>(tables)) + ... + ScanOperator(std::get<0>(tables)));
  std::vector<OutputTuple> result;
  while(plan.hasNext()) {
    result.push_back(plan.next());
  }
  return result;
}

template <typename OutputTuple, typename ProbeRelation, typename... BuildRelations>
std::vector<OutputTuple>
join6_2(std::tuple<ProbeRelation, BuildRelations...> const& tables,
        std::enable_if_t<RequestsChainOfBinaryJoins<decltype(std::get<0>(tables))>::value>* = {}) {
  auto plan = (JoinOperator(get<BuildRelations>(tables)) + ... + ScanOperator(std::get<0>(tables)));
  std::vector<OutputTuple> result;
  while(plan.hasNext()) {
    result.push_back(plan.next());
  }
  return result;
}