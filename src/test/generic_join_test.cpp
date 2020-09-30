#ifndef _SONIC_TEST_
#define _SONIC_TEST_

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../BinaryJoin.hpp"
#include "join_interfaces.h"

#define SONIC_BUCKET_SIZE 2

#endif

using namespace std;

namespace TestConfigurations {
class Sonic {
public:
  template <size_t Capacity, size_t BucketSize, size_t ColumnIndex, typename... ColumnTypes>
  using Index = SonicIndex<Capacity, BucketSize, ColumnIndex, ColumnTypes...>;

  template <typename Index, typename TupleOfTypesInTotalOrder,
            size_t... OffsetsOfStoredTuplesInTotalOrder>
  using Adapter = SonicToTotalOrderLookupAdapter<Index, TupleOfTypesInTotalOrder,
                                                 OffsetsOfStoredTuplesInTotalOrder...>;
};

class BinaryHashJoin {
public:
  template <size_t Capacity, size_t BucketSize, size_t ColumnIndex, typename... ColumnTypes>
  using Index = UnorderedMapIndex<Capacity, BucketSize, ColumnIndex, ColumnTypes...>;

  template <typename Index, typename TupleOfTypesInTotalOrder,
            size_t... OffsetsOfStoredTuplesInTotalOrder>
  using Adapter = UnorderedMapIndexAdapter<Index, TupleOfTypesInTotalOrder,
                                           OffsetsOfStoredTuplesInTotalOrder...>;
};

}; // namespace TestConfigurations

TEMPLATE_TEST_CASE("ThreeTables", "[ThreeTables]", TestConfigurations::Sonic,
                   TestConfigurations::BinaryHashJoin) {
  using Configuration = TestType;
  constexpr size_t Capacity = 8;

  using totalOrderSchema = tuple<string, int, int>;

  using inputSchemaR = tuple<int, string>;
  using sonicIndexR =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, string, int>;
  using sonicAdapterR =
      typename Configuration::template Adapter<sonicIndexR, totalOrderSchema, 0, 1>;
  using attributesInTotalOrderR = AttributeIndex<1, 0>;
  using attributesOrderR = AttributeIndex<1, 0>;

  using inputSchemaS = tuple<string, int>;
  using sonicIndexS =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, string, int>;
  using sonicAdapterS =
      typename Configuration::template Adapter<sonicIndexS, totalOrderSchema, 0, 2>;
  using attributesInTotalOrderS = AttributeIndex<0, 2>;
  using attributesOrderS = AttributeIndex<0, 1>;

  using inputSchemaT = tuple<int, int>;
  using sonicIndexT =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int>;
  using sonicAdapterT =
      typename Configuration::template Adapter<sonicIndexT, totalOrderSchema, 1, 2>;
  using attributesInTotalOrderT = AttributeIndex<1, 2>;
  using attributesOrderT = AttributeIndex<0, 1>;

  auto tableR = vector<inputSchemaR>{{1, "b"}, {2, "bb"}, {3, "bbb"}, {4, "bbbb"}};
  auto tableS = vector<inputSchemaS>{{"b", 1}, {"bb", 2}, {"bbb", 3}, {"bbbb", 4}};
  auto tableT = vector<inputSchemaT>{{1, 1}, {2, 2}, {3, 3}, {4, 4}};

  auto R =
      Relation<sonicAdapterR, inputSchemaR, attributesInTotalOrderR, attributesOrderR>(1, tableR);
  auto S =
      Relation<sonicAdapterS, inputSchemaS, attributesInTotalOrderS, attributesOrderS>(2, tableS);

  auto T =
      Relation<sonicAdapterT, inputSchemaT, attributesInTotalOrderT, attributesOrderT>(3, tableT);

  tuple<Relation<sonicAdapterR, inputSchemaR, attributesInTotalOrderR, attributesOrderR>&,
        Relation<sonicAdapterS, inputSchemaS, attributesInTotalOrderS, attributesOrderS>&,
        Relation<sonicAdapterT, inputSchemaT, attributesInTotalOrderT, attributesOrderT>&>
      relationSet = {R, S, T};

  auto joinResult = join3<tuple<string, int, int>>(relationSet);
  auto expectedResult =
      vector<tuple<string, int, int>>{{"bbb", 3, 3}, {"bbbb", 4, 4}, {"bb", 2, 2}, {"b", 1, 1}};

  for(auto t : expectedResult) {
    REQUIRE(find(joinResult.begin(), joinResult.end(), t) != joinResult.end());
  }
}

TEMPLATE_TEST_CASE("FiveTables", "[FiveTables]", TestConfigurations::Sonic,
                   TestConfigurations::BinaryHashJoin) {
  using Configuration = TestType;
  constexpr size_t Capacity = 8;

  /// Total order = <1,4,2,5,3,6>
  using totalOrderSchema = tuple<int, int, int, int, int, int>;

  /// A <1,2,4,5>
  using inputSchemaA = tuple<int, int, int, int>;
  using sonicIndexA =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int, int, int>;
  using sonicAdapterA =
      typename Configuration::template Adapter<sonicIndexA, totalOrderSchema, 0, 1, 2, 3>;
  using attributesInTotalOrderA = AttributeIndex<0, 2, 1, 3>;
  using attributesOrderA = AttributeIndex<0, 2, 1, 3>;

  /// B <1,3,4,6>
  using inputSchemaB = tuple<int, int, int, int>;
  using sonicIndexB =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int, int, int>;
  using sonicAdapterB =
      typename Configuration::template Adapter<sonicIndexB, totalOrderSchema, 0, 1, 4, 5>;
  using attributesInTotalOrderB = AttributeIndex<0, 4, 1, 5>;
  using attributesOrderB = AttributeIndex<0, 2, 1, 3>;

  /// C <1,2,3>
  using inputSchemaC = tuple<int, int, int>;
  using sonicIndexC =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int, int>;
  using sonicAdapterC =
      typename Configuration::template Adapter<sonicIndexC, totalOrderSchema, 0, 2, 4>;
  using attributesInTotalOrderC = AttributeIndex<0, 2, 4>;
  using attributesOrderC = AttributeIndex<0, 1, 2>;

  /// D <2,4,6>
  using inputSchemaD = tuple<int, int, int>;
  using sonicIndexD =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int, int>;
  using sonicAdapterD =
      typename Configuration::template Adapter<sonicIndexD, totalOrderSchema, 1, 2, 5>;
  using attributesInTotalOrderD = AttributeIndex<2, 1, 5>;
  using attributesOrderD = AttributeIndex<1, 0, 2>;

  /// E <3,5,6>
  using inputSchemaE = tuple<int, int, int>;
  using sonicIndexE =
      typename Configuration::template Index<Capacity, SONIC_BUCKET_SIZE, 0, int, int, int>;
  using sonicAdapterE =
      typename Configuration::template Adapter<sonicIndexE, totalOrderSchema, 3, 4, 5>;
  using attributesInTotalOrderE = AttributeIndex<4, 3, 5>;
  using attributesOrderE = AttributeIndex<1, 0, 2>;

  auto tableA = vector<inputSchemaA>{{1, 1, 1, 1}, {2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}};
  auto tableB = vector<inputSchemaB>{{1, 1, 1, 1}, {2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}};
  auto tableC = vector<inputSchemaC>{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};
  auto tableD = vector<inputSchemaD>{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};
  auto tableE = vector<inputSchemaE>{{1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};

  auto A =
      Relation<sonicAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>(1, tableA);
  auto B =
      Relation<sonicAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>(2, tableB);
  auto C =
      Relation<sonicAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>(3, tableC);
  auto D =
      Relation<sonicAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>(4, tableD);
  auto E =
      Relation<sonicAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>(5, tableE);

  tuple<Relation<sonicAdapterA, inputSchemaA, attributesInTotalOrderA, attributesOrderA>&,
        Relation<sonicAdapterB, inputSchemaB, attributesInTotalOrderB, attributesOrderB>&,
        Relation<sonicAdapterC, inputSchemaC, attributesInTotalOrderC, attributesOrderC>&,
        Relation<sonicAdapterD, inputSchemaD, attributesInTotalOrderD, attributesOrderD>&,
        Relation<sonicAdapterE, inputSchemaE, attributesInTotalOrderE, attributesOrderE>&>
      relationSet = {A, B, C, D, E};

  auto joinResult = join5<tuple<int, int, int, int, int, int>>(relationSet);
  auto expectedResult = vector<tuple<int, int, int, int, int, int>>{
      {1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 2, 2}, {3, 3, 3, 3, 3, 3}, {4, 4, 4, 4, 4, 4}};

  for(auto t : expectedResult) {
    REQUIRE(find(joinResult.begin(), joinResult.end(), t) != joinResult.end());
  }
  REQUIRE(joinResult.size() == 4);
}
