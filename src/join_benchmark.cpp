#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>

#include "../header/data_generator.h"
#include "../header/indices/btree_wrapper.h"
#include "../header/indices/hierarchical_hashtable.hpp"
#include "../header/indices/htrie_wrapper.h"
#include "../header/indices/index_total_order_adapter.h"
#include "BinaryJoin.hpp"
#include "join_interface.h"

#include <benchmark/benchmark.h>

#include "ITTNotifySupport.hpp"
#include <absl/container/btree_set.h>
#include <absl/container/flat_hash_map.h>
using absl::btree_multiset;
using absl::btree_set;

using namespace std;

template <typename K, typename V> using AbseilHashAdapter = absl::flat_hash_map<K, V, CRCHash<K>>;

template <size_t /*Capacity*/, size_t /*bucketsize*/, size_t /*ColumnIndex*/,
          typename... ColumnTypes>
using HierarchicalAbseilHashIndex = HierarchicalMap<AbseilHashAdapter, ColumnTypes...>;

template <typename HierarchicalAbseilHashIndex, typename TupleOfTypesInTotalOrder,
          size_t... OffsetsOfStoredTuplesInTotalOrder>
using HierarchicalAbseilHashIndexAdapter =
    IndexToTotalOrderAdapter<HierarchicalAbseilHashIndex, TupleOfTypesInTotalOrder,
                             OffsetsOfStoredTuplesInTotalOrder...>;

template <size_t /*Capacity*/, size_t /*bucketsize*/, size_t /*ColumnIndex*/,
          typename... ColumnTypes>
using BTreeIndex = BTreeAdapter<ColumnTypes...>;

template <typename BTreeIndex, typename TupleOfTypesInTotalOrder,
          size_t... OffsetsOfStoredTuplesInTotalOrder>
using BTreeIndexAdapter = IndexToTotalOrderAdapter<BTreeIndex, TupleOfTypesInTotalOrder,
                                                   OffsetsOfStoredTuplesInTotalOrder...>;

template <size_t /*Capacity*/, size_t /*bucketsize*/, size_t /*ColumnIndex*/,
          typename... ColumnTypes>
using HTrieIndex = HTrieAdapter<ColumnTypes...>;

template <typename HTrieIndex, typename TupleOfTypesInTotalOrder,
          size_t... OffsetsOfStoredTuplesInTotalOrder>
using HTrieIndexAdapter = IndexToTotalOrderAdapter<HTrieIndex, TupleOfTypesInTotalOrder,
                                                   OffsetsOfStoredTuplesInTotalOrder...>;

static const char BINARY_JOIN[] = "BinaryJoin";
static const char ART_GENERIC_JOIN[] = "ARTJoin";
static const char BTREE_GENERIC_JOIN[] = "BTreeJoin";
static const char HTRIE_GENERIC_JOIN[] = "HTrieJoin";
static const char SONIC_GENERIC_JOIN[] = "SonicGenericJoin";
static const char HIERARCHICAL_MAP_GENERIC_JOIN[] = "HierarchicalMapGenericJoin";

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void ThreeTableJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema = tuple<int, int, int>;

  using inputSchemaR = tuple<int, int>;
  using IndexR = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterR = IndexAdapter<IndexR, totalOrderSchema, 0, 1>;
  using attributesInTotalOrderR = AttributeIndex<1, 0>;
  using attributesOrderR = AttributeIndex<1, 0>;

  using inputSchemaS = tuple<int, int>;
  using IndexS = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterS = IndexAdapter<IndexS, totalOrderSchema, 0, 2>;
  using attributesInTotalOrderS = AttributeIndex<0, 2>;
  using attributesOrderS = AttributeIndex<0, 1>;

  using inputSchemaT = tuple<int, int>;
  using IndexT = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterT = IndexAdapter<IndexT, totalOrderSchema, 1, 2>;
  using attributesInTotalOrderT = AttributeIndex<1, 2>;
  using attributesOrderT = AttributeIndex<0, 1>;

  auto tableR = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableS = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableT = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("ThreeTableJoin<", RowSize, ">");
    auto R = make_unique<
        Relation<IndexAdapterR, inputSchemaR, attributesInTotalOrderR, attributesOrderR>>(1,
                                                                                          tableR);
    auto S = make_unique<
        Relation<IndexAdapterS, inputSchemaS, attributesInTotalOrderS, attributesOrderS>>(2,
                                                                                          tableS);

    auto T = make_unique<
        Relation<IndexAdapterT, inputSchemaT, attributesInTotalOrderT, attributesOrderT>>(3,
                                                                                          tableT);

    tuple<Relation<IndexAdapterR, inputSchemaR, attributesInTotalOrderR, attributesOrderR>&,
          Relation<IndexAdapterS, inputSchemaS, attributesInTotalOrderS, attributesOrderS>&,
          Relation<IndexAdapterT, inputSchemaT, attributesInTotalOrderT, attributesOrderT>&>
        relationSet = {*R, *S, *T};
    join3<totalOrderSchema>(relationSet);
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void FourTableJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema = tuple<int, int, int, int, int, int, int, int>;

  using inputSchemaA = tuple<int, int, int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 0, 1, 5, 6>;
  using attributesInTotalOrderA = AttributeIndex<0, 1, 5, 6>;
  using attributesOrderA = AttributeIndex<0, 1, 3, 2>;

  using inputSchemaB = tuple<int, int, int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 0, 2, 3, 6>;
  using attributesInTotalOrderB = AttributeIndex<0, 2, 6, 3>;
  using attributesOrderB = AttributeIndex<0, 1, 3, 2>;

  using inputSchemaC = tuple<int, int, int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 1, 2, 3, 7>;
  using attributesInTotalOrderC = AttributeIndex<1, 2, 3, 7>;
  using attributesOrderC = AttributeIndex<0, 1, 2, 3>;

  using inputSchemaD = tuple<int, int, int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 4, 5, 6, 7>;
  using attributesInTotalOrderD = AttributeIndex<7, 6, 5, 4>;
  using attributesOrderD = AttributeIndex<2, 1, 0, 3>;

  auto tableA = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("FourTableJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&>
        relationSet = {*A, *B, *C, *D};
    join4<totalOrderSchema>(relationSet);
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void FiveTableJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema = tuple<int, int, int, int, int, int>;

  using inputSchemaA = tuple<int, int, int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 0, 1, 2, 3>;
  using attributesInTotalOrderA = AttributeIndex<0, 2, 1, 3>;
  using attributesOrderA = AttributeIndex<0, 2, 1, 3>;

  using inputSchemaB = tuple<int, int, int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 0, 1, 4, 5>;
  using attributesInTotalOrderB = AttributeIndex<0, 4, 1, 5>;
  using attributesOrderB = AttributeIndex<0, 2, 1, 3>;

  using inputSchemaC = tuple<int, int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 0, 2, 4>;
  using attributesInTotalOrderC = AttributeIndex<0, 2, 4>;
  using attributesOrderC = AttributeIndex<0, 1, 2>;

  using inputSchemaD = tuple<int, int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 1, 2, 5>;
  using attributesInTotalOrderD = AttributeIndex<2, 1, 5>;
  using attributesOrderD = AttributeIndex<1, 0, 2>;

  using inputSchemaE = tuple<int, int, int>;
  using IndexE = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterE = IndexAdapter<IndexE, totalOrderSchema, 3, 4, 5>;
  using attributesInTotalOrderE = AttributeIndex<4, 3, 5>;
  using attributesOrderE = AttributeIndex<1, 0, 2>;

  auto tableA = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableE = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("FiveTableJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);
    auto E = make_unique<
        Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>>(5,
                                                                                          tableE);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&,
          Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>&>
        relationSet = {*A, *B, *C, *D, *E};
    join5<totalOrderSchema>(relationSet);
    ;
    vtune.stopSampling();
  }
}

/// Worse performance
template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void SixTableJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema = tuple<int, int, int, int, int, int, int, int, int, int, int, int, int>;

  using inputSchemaA = tuple<int, int, int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 0, 1, 5, 6>;
  using attributesInTotalOrderA = AttributeIndex<0, 1, 5, 6>;
  using attributesOrderA = AttributeIndex<1, 2, 3, 0>;

  using inputSchemaB = tuple<int, int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 1, 5, 12>;
  using attributesInTotalOrderB = AttributeIndex<1, 5, 12>;
  using attributesOrderB = AttributeIndex<0, 1, 2>;

  using inputSchemaC = tuple<int, int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 3, 4, 10>;
  using attributesInTotalOrderC = AttributeIndex<3, 4, 10>;
  using attributesOrderC = AttributeIndex<1, 0, 2>;

  using inputSchemaD = tuple<int, int, int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 2, 3, 5, 12>;
  using attributesInTotalOrderD = AttributeIndex<2, 3, 5, 12>;
  using attributesOrderD = AttributeIndex<2, 0, 1, 3>;

  using inputSchemaE = tuple<int, int, int, int>;
  using IndexE = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterE = IndexAdapter<IndexE, totalOrderSchema, 4, 5, 11, 12>;
  using attributesInTotalOrderE = AttributeIndex<4, 5, 11, 12>;
  using attributesOrderE = AttributeIndex<1, 0, 2, 3>;

  using inputSchemaF = tuple<int, int, int, int, int, int, int>;
  using IndexF = Index<Capacity, BucketSize, 0, int, int, int, int, int, int, int>;
  using IndexAdapterF = IndexAdapter<IndexF, totalOrderSchema, 6, 7, 8, 9, 10, 11, 12>;
  using attributesInTotalOrderF = AttributeIndex<6, 7, 8, 9, 10, 11, 12>;
  using attributesOrderF = AttributeIndex<0, 4, 5, 6, 2, 1, 3>;

  auto tableA = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableE = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableF = datagenerator::Table<int, int, int, int, int, int, int>::generateIntegerTuples(
      "join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("SixTableJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);
    auto E = make_unique<
        Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>>(5,
                                                                                          tableE);
    auto F = make_unique<
        Relation<IndexAdapterF, inputSchemaF, attributesInTotalOrderF, attributesOrderF>>(6,
                                                                                          tableF);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&,
          Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>&,
          Relation<IndexAdapterF, inputSchemaF, attributesInTotalOrderF, attributesOrderF>&>
        relationSet = {*A, *B, *C, *D, *E, *F};
    join6<totalOrderSchema>(relationSet);
    ;
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void SixTableJoin2(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema =
      tuple<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int>;

  using inputSchemaA = tuple<int, int, int, int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int, int, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 1, 2, 3, 4, 10>;
  using attributesInTotalOrderA = AttributeIndex<1, 2, 3, 4, 10>;
  using attributesOrderA = AttributeIndex<1, 3, 4, 2, 0>;

  using inputSchemaB = tuple<int, int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 2, 4, 5>;
  using attributesInTotalOrderB = AttributeIndex<2, 4, 5>;
  using attributesOrderB = AttributeIndex<1, 0, 2>;

  using inputSchemaC = tuple<int, int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 3, 4, 5>;
  using attributesInTotalOrderC = AttributeIndex<3, 4, 5>;
  using attributesOrderC = AttributeIndex<1, 0, 2>;

  using inputSchemaD = tuple<int, int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 5, 6, 7>;
  using attributesInTotalOrderD = AttributeIndex<5, 6, 7>;
  using attributesOrderD = AttributeIndex<0, 1, 2>;

  using inputSchemaE = tuple<int, int, int, int>;
  using IndexE = Index<Capacity, BucketSize, 0, int, int, int, int>;
  using IndexAdapterE = IndexAdapter<IndexE, totalOrderSchema, 6, 7, 8, 15>;
  using attributesInTotalOrderE = AttributeIndex<6, 7, 8, 15>;
  using attributesOrderE = AttributeIndex<1, 2, 0, 3>;

  using inputSchemaF = tuple<int, int, int, int, int, int, int>;
  using IndexF = Index<Capacity, BucketSize, 0, int, int, int, int, int, int, int>;
  using IndexAdapterF = IndexAdapter<IndexF, totalOrderSchema, 9, 10, 11, 12, 13, 14, 15>;
  using attributesInTotalOrderF = AttributeIndex<9, 10, 11, 12, 13, 14, 15>;
  using attributesOrderF = AttributeIndex<6, 1, 2, 3, 4, 5, 0>;

  auto tableA =
      datagenerator::Table<int, int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableE = datagenerator::Table<int, int, int, int>::generateIntegerTuples("join", RowSize);
  auto tableF = datagenerator::Table<int, int, int, int, int, int, int>::generateIntegerTuples(
      "join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("SixTableJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);
    auto E = make_unique<
        Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>>(5,
                                                                                          tableE);
    auto F = make_unique<
        Relation<IndexAdapterF, inputSchemaF, attributesInTotalOrderF, attributesOrderF>>(6,
                                                                                          tableF);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&,
          Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>&,
          Relation<IndexAdapterF, inputSchemaF, attributesInTotalOrderF, attributesOrderF>&>
        relationSet = {*A, *B, *C, *D, *E, *F};
    join6_2<totalOrderSchema>(relationSet);
    ;
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void RectangleJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  using totalOrderSchema = tuple<int, int, int, int>;

  using inputSchemaA = tuple<int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 0, 2>;
  using attributesInTotalOrderA = AttributeIndex<0, 2>;
  using attributesOrderA = AttributeIndex<1, 0>;

  using inputSchemaB = tuple<int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 0, 1>;
  using attributesInTotalOrderB = AttributeIndex<0, 1>;
  using attributesOrderB = AttributeIndex<0, 1>;

  using inputSchemaC = tuple<int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 1, 3>;
  using attributesInTotalOrderC = AttributeIndex<1, 3>;
  using attributesOrderC = AttributeIndex<0, 1>;

  using inputSchemaD = tuple<int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 2, 3>;
  using attributesInTotalOrderD = AttributeIndex<2, 3>;
  using attributesOrderD = AttributeIndex<1, 0>;

  auto tableA = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("RectangleJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&>
        relationSet = {*A, *B, *C, *D};
    joinRec<totalOrderSchema>(relationSet);
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void PentagonJoin(benchmark::State& state) {
  size_t constexpr Capacity = RowSize << 1;

  /// Total order = <2, 3, 4, 1, 5>
  using totalOrderSchema = tuple<int, int, int, int, int>;

  /// A <1,2>
  using inputSchemaA = tuple<int, int>;
  using IndexA = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterA = IndexAdapter<IndexA, totalOrderSchema, 0, 3>;
  using attributesInTotalOrderA = AttributeIndex<0, 3>;
  using attributesOrderA = AttributeIndex<1, 0>;

  /// B <2,3>
  using inputSchemaB = tuple<int, int>;
  using IndexB = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterB = IndexAdapter<IndexB, totalOrderSchema, 0, 1>;
  using attributesInTotalOrderB = AttributeIndex<0, 1>;
  using attributesOrderB = AttributeIndex<0, 1>;

  /// C <3,4>
  using inputSchemaC = tuple<int, int>;
  using IndexC = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterC = IndexAdapter<IndexC, totalOrderSchema, 1, 2>;
  using attributesInTotalOrderC = AttributeIndex<1, 2>;
  using attributesOrderC = AttributeIndex<0, 1>;

  /// D <4,5>
  using inputSchemaD = tuple<int, int>;
  using IndexD = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterD = IndexAdapter<IndexD, totalOrderSchema, 2, 4>;
  using attributesInTotalOrderD = AttributeIndex<2, 4>;
  using attributesOrderD = AttributeIndex<0, 1>;

  /// E <5,1>
  using inputSchemaE = tuple<int, int>;
  using IndexE = Index<Capacity, BucketSize, 0, int, int>;
  using IndexAdapterE = IndexAdapter<IndexE, totalOrderSchema, 3, 4>;
  using attributesInTotalOrderE = AttributeIndex<3, 4>;
  using attributesOrderE = AttributeIndex<1, 0>;

  auto tableA = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableB = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableC = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableD = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);
  auto tableE = datagenerator::Table<int, int>::generateIntegerTuples("join", RowSize);

  static auto vtune = VTuneAPIInterface{"WCOJ"};

  for(auto _ : state) {
    vtune.startSampling("PentagonJoin<", RowSize, ">");
    auto A = make_unique<
        Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>>(1,
                                                                                          tableA);
    auto B = make_unique<
        Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>>(2,
                                                                                          tableB);
    auto C = make_unique<
        Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>>(3,
                                                                                          tableC);
    auto D = make_unique<
        Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>>(4,
                                                                                          tableD);
    auto E = make_unique<
        Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>>(5,
                                                                                          tableE);

    tuple<Relation<IndexAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
          Relation<IndexAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
          Relation<IndexAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
          Relation<IndexAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&,
          Relation<IndexAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>&>
        relationSet = {*A, *B, *C, *D, *E};
    joinPen<totalOrderSchema>(relationSet);
    ;
    vtune.stopSampling();
  }
}

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void JoinBenchmark(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {
          {4, FourTableJoin<RowSize, BucketSize, Index, IndexAdapter>},
          {5, FiveTableJoin<RowSize, BucketSize, Index, IndexAdapter>},
          {61, SixTableJoin<RowSize, BucketSize, Index, IndexAdapter>},
          {62, SixTableJoin2<RowSize, BucketSize, Index, IndexAdapter>},
      })
      .at(state.range(0))(state);
}

template <size_t RowSize, size_t BucketSize, const char* JoinType>
static void Join(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"BinaryJoin",
        JoinBenchmark<RowSize, BucketSize, UnorderedMapIndex, UnorderedMapIndexAdapter>},
       {"BTreeJoin", JoinBenchmark<RowSize, BucketSize, BTreeIndex, BTreeIndexAdapter>},
       {"HTrieJoin", JoinBenchmark<RowSize, BucketSize, BTreeIndex, BTreeIndexAdapter>},
       {"SonicGenericJoin",
        JoinBenchmark<RowSize, BucketSize, SonicIndex, SonicToTotalOrderLookupAdapter>},
       {"HierarchicalMapGenericJoin",
        JoinBenchmark<RowSize, BucketSize, HierarchicalAbseilHashIndex,
                      HierarchicalAbseilHashIndexAdapter>}})
      .at(JoinType)(state);
}

BENCHMARK_TEMPLATE(Join, 1048576, 4, BINARY_JOIN)
    ->Arg(4)
    ->Arg(5)
    ->Arg(61)
    ->Arg(62)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Join, 1048576, 4, BTREE_GENERIC_JOIN)
    ->Arg(4)
    ->Arg(5)
    ->Arg(61)
    ->Arg(62)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Join, 1048576, 4, HTRIE_GENERIC_JOIN)
    ->Arg(4)
    ->Arg(5)
    ->Arg(61)
    ->Arg(62)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Join, 1048576, 4, SONIC_GENERIC_JOIN)
    ->Arg(4)
    ->Arg(5)
    ->Arg(61)
    ->Arg(62)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Join, 1048576, 4, HIERARCHICAL_MAP_GENERIC_JOIN)
    ->Arg(4)
    ->Arg(5)
    ->Arg(61)
    ->Arg(62)
    ->Unit(benchmark::kMillisecond);

////////////////////////////////////// TWO COLUMN //////////////////////////////

template <size_t RowSize, size_t BucketSize,
          template <size_t, size_t, size_t, typename...> typename Index,
          template <typename, typename, size_t...> typename IndexAdapter>
static void TwoColumnCountingBenchmark(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {
          {3, ThreeTableJoin<RowSize, BucketSize, Index, IndexAdapter>},
          {4, RectangleJoin<RowSize, BucketSize, Index, IndexAdapter>},
          {5, PentagonJoin<RowSize, BucketSize, Index, IndexAdapter>},
      })
      .at(state.range(0))(state);
}

template <size_t RowSize, size_t BucketSize, const char* JoinType>
static void TwoColumn_Counting(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"BTreeJoin",
        TwoColumnCountingBenchmark<RowSize, BucketSize, BTreeIndex, BTreeIndexAdapter>},
       {"HTrieJoin",
        TwoColumnCountingBenchmark<RowSize, BucketSize, BTreeIndex, BTreeIndexAdapter>},
       {"SonicGenericJoin", TwoColumnCountingBenchmark<RowSize, BucketSize, SonicIndex,
                                                       SonicToTotalOrderLookupAdapter>},
       {"HierarchicalMapGenericJoin",
        TwoColumnCountingBenchmark<RowSize, BucketSize, HierarchicalAbseilHashIndex,
                                   HierarchicalAbseilHashIndexAdapter>}})
      .at(JoinType)(state);
}

BENCHMARK_TEMPLATE(TwoColumn_Counting, 16777216, 4, BTREE_GENERIC_JOIN)
    ->RangeMultiplier(2)
    ->Ranges({{3, 5}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(TwoColumn_Counting, 16777216, 4, HTRIE_GENERIC_JOIN)
    ->RangeMultiplier(2)
    ->Ranges({{3, 5}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(TwoColumn_Counting, 16777216, 4, SONIC_GENERIC_JOIN)
    ->RangeMultiplier(2)
    ->Ranges({{3, 5}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(TwoColumn_Counting, 16777216, 4, HIERARCHICAL_MAP_GENERIC_JOIN)
    ->RangeMultiplier(2)
    ->Ranges({{3, 5}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
