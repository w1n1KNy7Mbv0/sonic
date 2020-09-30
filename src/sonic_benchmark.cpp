#ifndef _BUILD_BENCHMARK_
#define _BUILD_BENCHMARK_

#include <unordered_map>

#include <benchmark/benchmark.h>

#include "../header/data_generator.h"
#include "../header/indices/abseil_wrapper.h"
#include "../header/indices/btree_wrapper.h"
#include "../header/indices/hierarchical_hashtable.hpp"
#include "../header/indices/htrie_wrapper.h"
#include "../header/indices/robin_wrapper.h"
#include "../header/indices/sonic/sonic_index.h"
#include "../header/indices/surf_wrapper.h"

#include <absl/container/flat_hash_map.h>

#include <iostream>

#define NUMBER_OF_ROWS 8192 << 5
#define SONIC_BUCKET_SIZE 8

#endif

using namespace std;

template <typename K, typename V> using AbseilHashAdapter = absl::flat_hash_map<K, V, CRCHash<K>>;

template <typename... ColumnTypes>
using HierarchicalAbseilHashAdapter = HierarchicalMap<AbseilHashAdapter, ColumnTypes...>;

template <size_t N> constexpr size_t Log2() { return ((N < 2) ? 1 : 1 + Log2<N / 2>()); }

template <size_t R, size_t B> constexpr size_t Capacity() { return (R << (Log2<B>())); }

template <typename Index, typename LookupTable, typename ColumnTuple, size_t... I>
size_t lookup(Index& index, LookupTable& lookupTable, size_t rowSize, ColumnTuple tuple,
              index_sequence<I...>) {
  auto lookupTuple = make_tuple(get<I>(lookupTable[rand() % rowSize])...);
  return index.template prefixLookup<tuple_element_t<I, ColumnTuple>...>(lookupTuple).size();
}

template <typename Index, typename LookupTable, typename ColumnTuple, size_t... I>
size_t countPrefix(Index& index, LookupTable& lookupTable, size_t rowSize, ColumnTuple tuple,
                   index_sequence<I...>) {
  auto lookupTuple = make_tuple(get<I>(lookupTable[rand() % rowSize])...);
  return index.template countPrefix<tuple_element_t<I, ColumnTuple>...>(lookupTuple);
}

template <size_t RowsNumber, typename IndexWrapper, typename... Columns>
static void BuildIndexBenchmark(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateIntegerTuples("join", RowsNumber);

  for(auto _ : state) {
    IndexWrapper index(table);
  }
}

template <size_t RowsNumber, typename IndexWrapper, typename... Columns>
static void BuildIndexBenchmarkString(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateStringTuples("join", RowsNumber);

  for(auto _ : state) {
    IndexWrapper index(table);
  }
}

template <size_t RowsNumber, typename IndexWrapper, typename... Columns>
static void PointLookupBenchmark(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateIntegerTuples("join", RowsNumber);

  IndexWrapper index(table);

  vector<tuple<Columns...>> lookupTable;
  lookupTable.reserve(RowsNumber);
  for(auto i = 0; i < RowsNumber; i++) {
    if(i % 8192 == 0) {
      lookupTable.emplace_back(
          datagenerator::Table<Columns...>::generateIntegerTuples("random", 1)[0]);
      i++;
    }
    lookupTable.emplace_back(table[rand() % RowsNumber]);
  }

  for(auto i = 0; i < RowsNumber - 1; i++) {
    auto j = i + rand() % (RowsNumber - i);
    swap(lookupTable[i], lookupTable[j]);
  }

  auto sum = 0;
  for(auto _ : state) {
    benchmark::DoNotOptimize(sum += index.pointLookup(lookupTable[rand() % RowsNumber]));
  }
}

template <size_t RowsNumber, size_t PrefixLength, typename IndexWrapper, typename... Columns>
static void PrefixLookupBenchmark(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateIntegerTuples("join", RowsNumber);

  IndexWrapper index(table);

  vector<tuple<Columns...>> lookupTable;
  lookupTable.reserve(RowsNumber);
  for(auto i = 0; i < RowsNumber; i++) {
    if(i % 8192 == 0) {
      lookupTable.emplace_back(
          datagenerator::Table<Columns...>::generateIntegerTuples("random", 1)[0]);
      i++;
    }
    lookupTable.emplace_back(table[rand() % RowsNumber]);
  }

  for(auto i = 0; i < RowsNumber - 1; i++) {
    auto j = i + rand() % (RowsNumber - i);
    swap(lookupTable[i], lookupTable[j]);
  }

  auto sum = 0;
  for(auto _ : state) {
    benchmark::DoNotOptimize(sum += lookup(index, lookupTable, RowsNumber, tuple<Columns...>(),
                                           make_index_sequence<PrefixLength>()));
  }
}

template <size_t RowsNumber, size_t PrefixLength, typename IndexWrapper, typename... Columns>
static void PrefixLookupBenchmarkString(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateStringTuples("join", RowsNumber);

  IndexWrapper index(table);

  vector<tuple<Columns...>> lookupTable;
  lookupTable.reserve(RowsNumber);
  for(auto i = 0; i < RowsNumber; i++) {
    if(i % 8192 == 0) {
      lookupTable.emplace_back(
          datagenerator::Table<Columns...>::generateStringTuples("random", 1)[0]);
      i++;
    }
    lookupTable.emplace_back(table[rand() % RowsNumber]);
  }

  for(auto i = 0; i < RowsNumber - 1; i++) {
    auto j = i + rand() % (RowsNumber - i);
    swap(lookupTable[i], lookupTable[j]);
  }

  auto sum = 0;
  for(auto _ : state) {
    benchmark::DoNotOptimize(sum += lookup(index, lookupTable, RowsNumber, tuple<Columns...>(),
                                           make_index_sequence<PrefixLength>()));
  }
}

template <size_t RowsNumber, size_t PrefixLength, typename IndexWrapper, typename... Columns>
static void CountPrefixBenchmark(benchmark::State& state) {
  auto table = datagenerator::Table<Columns...>::generateIntegerTuples("join", RowsNumber);

  IndexWrapper index(table);

  vector<tuple<Columns...>> lookupTable;
  lookupTable.reserve(RowsNumber);
  for(auto i = 0; i < RowsNumber; i++) {
    if(i % 8192 == 0) {
      lookupTable.emplace_back(
          datagenerator::Table<Columns...>::generateIntegerTuples("random", 1)[0]);
      i++;
    }
    lookupTable.emplace_back(table[rand() % RowsNumber]);
  }

  for(auto i = 0; i < RowsNumber - 1; i++) {
    auto j = i + rand() % (RowsNumber - i);
    swap(lookupTable[i], lookupTable[j]);
  }

  auto sum = 0;
  for(auto _ : state) {
    benchmark::DoNotOptimize(sum += countPrefix(index, lookupTable, RowsNumber, tuple<Columns...>(),
                                                make_index_sequence<PrefixLength>()));
  }
}

static const char ABSEIL[] = "abseil";
static const char ARTOLC[] = "art";
static const char BTREE[] = "btree";
static const char HIERARCHICAL_MAP[] = "hierarchicalmap";
static const char HTRIE[] = "htrie";
static const char ROBIN_FASTMAP[] = "robin";
static const char SURF[] = "surf";

// ================================= BUILD =====================================

template <size_t RowsNumber, template <typename...> typename Index>
static void BuildIndex(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, BuildIndexBenchmark<RowsNumber, Index<int, int>, int, int>},
       {4, BuildIndexBenchmark<RowsNumber, Index<int, int, int, int>, int, int, int, int>},
       {8, BuildIndexBenchmark<RowsNumber, Index<int, int, int, int, int, int, int, int>, int, int,
                               int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName> static void Build(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"abseil", BuildIndex<RowsNumber, AbseilAdapter>},
       {"btree", BuildIndex<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", BuildIndex<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", BuildIndex<RowsNumber, HTrieAdapter>},
       {"robin", BuildIndex<RowsNumber, RobinAdapter>},
       {"surf", BuildIndex<RowsNumber, SuRFAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize> static void Build_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2,
        BuildIndexBenchmark<
            RowsNumber, SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int>,
            int, int>},
       {4, BuildIndexBenchmark<
               RowsNumber,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int, int, int>,
               int, int, int, int>},
       {8, BuildIndexBenchmark<RowsNumber,
                               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int,
                                          int, int, int, int, int, int, int>,
                               int, int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(Build, 8388608, ABSEIL)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build, 8388608, BTREE)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build, 8388608, HIERARCHICAL_MAP)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build, 8388608, HTRIE)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build, 8388608, ROBIN_FASTMAP)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build_SONIC, 8388608, 4)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(Build, 8388608, SURF)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

// ================================= POINT LOOKUP ==============================

template <size_t RowsNumber, template <typename...> typename Index>
static void PointLookup(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PointLookupBenchmark<RowsNumber, Index<int, int>, int, int>},
       {4, PointLookupBenchmark<RowsNumber, Index<int, int, int, int>, int, int, int, int>},
       {8, PointLookupBenchmark<RowsNumber, Index<int, int, int, int, int, int, int, int>, int, int,
                                int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName>
static void Point_Lookup(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"abseil", PointLookup<RowsNumber, AbseilAdapter>},
       {"btree", PointLookup<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", PointLookup<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", PointLookup<RowsNumber, HTrieAdapter>},
       {"robin", PointLookup<RowsNumber, RobinAdapter>},
       {"surf", PointLookup<RowsNumber, SuRFAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize>
static void Point_Lookup_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2,
        PointLookupBenchmark<
            RowsNumber, SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int>,
            int, int>},
       {4, PointLookupBenchmark<
               RowsNumber,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int, int, int>,
               int, int, int, int>},
       {8, PointLookupBenchmark<RowsNumber,
                                SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int,
                                           int, int, int, int, int, int, int>,
                                int, int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, ABSEIL)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, BTREE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, HIERARCHICAL_MAP)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, HTRIE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, ROBIN_FASTMAP)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup_SONIC, 8388608, 4)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Point_Lookup, 8388608, SURF)->RangeMultiplier(2)->Ranges({{2, 8}});

// ================================ PREFIX LOOKUP ==============================

template <size_t RowsNumber, template <typename...> typename Index>
static void PrefixLookup(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PrefixLookupBenchmark<RowsNumber, 1, Index<int, int>, int, int>},
       {4, PrefixLookupBenchmark<RowsNumber, 2, Index<int, int, int, int>, int, int, int, int>},
       {8, PrefixLookupBenchmark<RowsNumber, 4, Index<int, int, int, int, int, int, int, int>, int,
                                 int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName>
static void Prefix_Lookup(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"btree", PrefixLookup<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", PrefixLookup<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", PrefixLookup<RowsNumber, HTrieAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize>
static void Prefix_Lookup_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2,
        PrefixLookupBenchmark<
            RowsNumber, 1,
            SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int>, int, int>},
       {4, PrefixLookupBenchmark<
               RowsNumber, 2,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int, int, int>,
               int, int, int, int>},
       {8, PrefixLookupBenchmark<RowsNumber, 4,
                                 SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0,
                                            int, int, int, int, int, int, int, int>,
                                 int, int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(Prefix_Lookup, 8388608, BTREE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Prefix_Lookup, 8388608, HIERARCHICAL_MAP)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Prefix_Lookup, 8388608, HTRIE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Prefix_Lookup_SONIC, 8388608, 4)->RangeMultiplier(2)->Ranges({{2, 8}});

// ================================ COUNT PREFIX ==============================

template <size_t RowsNumber, template <typename...> typename Index>
static void CountPrefix(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, CountPrefixBenchmark<RowsNumber, 1, Index<int, int>, int, int>},
       {4, CountPrefixBenchmark<RowsNumber, 2, Index<int, int, int, int>, int, int, int, int>},
       {8, CountPrefixBenchmark<RowsNumber, 4, Index<int, int, int, int, int, int, int, int>, int,
                                int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName>
static void Count_Prefix(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"btree", CountPrefix<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", CountPrefix<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", CountPrefix<RowsNumber, HTrieAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize>
static void Count_Prefix_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2,
        CountPrefixBenchmark<
            RowsNumber, 1,
            SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int>, int, int>},
       {4, CountPrefixBenchmark<
               RowsNumber, 2,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int, int, int, int>,
               int, int, int, int>},
       {8, CountPrefixBenchmark<RowsNumber, 4,
                                SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, int,
                                           int, int, int, int, int, int, int>,
                                int, int, int, int, int, int, int, int>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(Count_Prefix, 8388608, BTREE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Count_Prefix, 8388608, HIERARCHICAL_MAP)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Count_Prefix, 8388608, HTRIE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Count_Prefix_SONIC, 8388608, 4)->RangeMultiplier(2)->Ranges({{2, 8}});

// ======================================= BUCKET SIZE =========================

template <size_t RowsNumber> static void Bucket_Size_B_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, BuildIndexBenchmark<RowsNumber,
                               SonicIndex<(Capacity<RowsNumber, 2>()), 2, 0, int, int, int, int>,
                               int, int, int, int>},
       {4, BuildIndexBenchmark<RowsNumber,
                               SonicIndex<(Capacity<RowsNumber, 4>()), 4, 0, int, int, int, int>,
                               int, int, int, int>},
       {8, BuildIndexBenchmark<RowsNumber,
                               SonicIndex<(Capacity<RowsNumber, 8>()), 8, 0, int, int, int, int>,
                               int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber> static void Bucket_Size_Po_Lookup_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PointLookupBenchmark<RowsNumber,
                                SonicIndex<(Capacity<RowsNumber, 2>()), 2, 0, int, int, int, int>,
                                int, int, int, int>},
       {4, PointLookupBenchmark<RowsNumber,
                                SonicIndex<(Capacity<RowsNumber, 4>()), 4, 0, int, int, int, int>,
                                int, int, int, int>},
       {8, PointLookupBenchmark<RowsNumber,
                                SonicIndex<(Capacity<RowsNumber, 4>()), 4, 0, int, int, int, int>,
                                int, int, int, int>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber> static void Bucket_Size_Pr_Lookup_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PrefixLookupBenchmark<RowsNumber, 2,
                                 SonicIndex<(Capacity<RowsNumber, 2>()), 2, 0, int, int, int, int>,
                                 int, int, int, int>},
       {4, PrefixLookupBenchmark<RowsNumber, 2,
                                 SonicIndex<(Capacity<RowsNumber, 4>()), 4, 0, int, int, int, int>,
                                 int, int, int, int>},
       {8, PrefixLookupBenchmark<RowsNumber, 2,
                                 SonicIndex<(Capacity<RowsNumber, 8>()), 8, 0, int, int, int, int>,
                                 int, int, int, int>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(Bucket_Size_B_SONIC, 8388608)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Bucket_Size_Po_Lookup_SONIC, 8388608)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(Bucket_Size_Pr_Lookup_SONIC, 8388608)->RangeMultiplier(2)->Ranges({{2, 8}});

// ============================== BUILD STRING =================================

template <size_t RowsNumber, template <typename...> typename Index>
static void BuildIndexString(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, BuildIndexBenchmarkString<RowsNumber, Index<string, string>, string, string>},
       {4, BuildIndexBenchmarkString<RowsNumber, Index<string, string, string, string>, string,
                                     string, string, string>},
       {8, BuildIndexBenchmarkString<
               RowsNumber, Index<string, string, string, string, string, string, string, string>,
               string, string, string, string, string, string, string, string>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName>
static void String_Key_B(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"abseil", BuildIndexString<RowsNumber, AbseilAdapter>},
       {"btree", BuildIndexString<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", BuildIndexString<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", BuildIndexString<RowsNumber, HTrieAdapter>},
       {"robin", BuildIndexString<RowsNumber, RobinAdapter>},
       {"surf", BuildIndexString<RowsNumber, SuRFAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize>
static void String_Key_B_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, BuildIndexBenchmarkString<
               RowsNumber,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, string, string>,
               string, string>},
       {4, BuildIndexBenchmarkString<RowsNumber,
                                     SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0,
                                                string, string, string, string>,
                                     string, string, string, string>},
       {8, BuildIndexBenchmarkString<
               RowsNumber,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, string, string,
                          string, string, string, string, string, string>,
               string, string, string, string, string, string, string, string>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(String_Key_B, 262144, ABSEIL)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B, 262144, BTREE)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B, 262144, HIERARCHICAL_MAP)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B, 262144, HTRIE)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B, 262144, ROBIN_FASTMAP)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B_SONIC, 262144, 4)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(String_Key_B, 262144, SURF)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}})
    ->Unit(benchmark::kMillisecond);

// ========================= PREFIX LOOKUP STRING ==============================

template <size_t RowsNumber, template <typename...> typename Index>
static void PrefixLookupString(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PrefixLookupBenchmarkString<RowsNumber, 1, Index<string, string>, string, string>},
       {4, PrefixLookupBenchmarkString<RowsNumber, 2, Index<string, string, string, string>, string,
                                       string, string, string>},
       {8, PrefixLookupBenchmarkString<
               RowsNumber, 4, Index<string, string, string, string, string, string, string, string>,
               string, string, string, string, string, string, string, string>}})
      .at(state.range(0))(state);
}

template <size_t RowsNumber, const char* IndexName>
static void String_Key_Pr_Lookup(benchmark::State& state) {
  map<string, function<void(benchmark::State&)>>(
      {{"btree", PrefixLookupString<RowsNumber, BTreeAdapter>},
       {"hierarchicalmap", PrefixLookupString<RowsNumber, HierarchicalAbseilHashAdapter>},
       {"htrie", PrefixLookupString<RowsNumber, HTrieAdapter>}})
      .at(IndexName)(state);
}

template <size_t RowsNumber, size_t BucketSize>
static void String_Key_Pr_Lookup_SONIC(benchmark::State& state) {
  map<int, function<void(benchmark::State&)>>(
      {{2, PrefixLookupBenchmarkString<
               RowsNumber, 1,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, string, string>,
               string, string>},
       {4, PrefixLookupBenchmarkString<RowsNumber, 2,
                                       SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize,
                                                  0, string, string, string, string>,
                                       string, string, string, string>},
       {8, PrefixLookupBenchmarkString<
               RowsNumber, 4,
               SonicIndex<(Capacity<RowsNumber, BucketSize>()), BucketSize, 0, string, string,
                          string, string, string, string, string, string>,
               string, string, string, string, string, string, string, string>}})
      .at(state.range(0))(state);
}

BENCHMARK_TEMPLATE(String_Key_Pr_Lookup, 262144, BTREE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(String_Key_Pr_Lookup, 262144, HIERARCHICAL_MAP)
    ->RangeMultiplier(2)
    ->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(String_Key_Pr_Lookup, 262144, HTRIE)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_TEMPLATE(String_Key_Pr_Lookup_SONIC, 262144, 4)->RangeMultiplier(2)->Ranges({{2, 8}});

BENCHMARK_MAIN();
