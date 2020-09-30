#ifndef _SONIC_TEST_
#define _SONIC_TEST_

#include <catch2/catch.hpp>

#include "../../header/indices/sonic/sonic_index.h"

#endif

using namespace std;

TEST_CASE("TwoColumnTable", "[TwoColumn]") {
  vector<tuple<int, string>> data = {tuple<int, string>{1, "s"}, tuple<int, string>{1, "p"},
                                     tuple<int, string>{2, "s"}, tuple<int, string>{1, "d"},
                                     tuple<int, string>{2, "f"}};
  SonicIndex<10, 2, 0, int, string> sonic(data);

  auto pointLookup1 = sonic.pointLookup(tuple<int, string>{1, "s"});
  REQUIRE(pointLookup1 == 1);

  auto pointLookup2 = sonic.pointLookup(tuple<int, string>{2, "f"});
  REQUIRE(pointLookup2 == 1);

  auto pointLookup3 = sonic.pointLookup(tuple<int, string>{1, "q"});
  REQUIRE(pointLookup3 == 0);

  auto pointLookup4 = sonic.pointLookup(tuple<int, string>{1, "d"});
  REQUIRE(pointLookup4 == 1);

  auto countPrefix1 = sonic.template countPrefix<int, string>(tuple<int, string>{1, "s"});
  REQUIRE(countPrefix1 == 1);

  auto countPrefix2 = sonic.template countPrefix<int>(tuple<int>{1});
  REQUIRE(countPrefix2 == 3);

  auto countPrefix3 = sonic.template countPrefix<int>(tuple<int>{3});
  REQUIRE(countPrefix3 == 0);

  auto countPrefix4 = sonic.template countPrefix<int>(tuple<int>{2});
  REQUIRE(countPrefix4 == 2);

  auto prefixLookup1 = sonic.template prefixLookup<int, string>(tuple<int, string>{1, "s"}).size();
  REQUIRE(prefixLookup1 == 1);

  auto prefixLookup2 = sonic.template prefixLookup<int>(tuple<int>{1}).size();
  REQUIRE(prefixLookup2 == 3);

  auto prefixLookup3 = sonic.template prefixLookup<int>(tuple<int>{3}).size();
  REQUIRE(prefixLookup3 == 0);

  auto prefixLookup4 = sonic.template prefixLookup<int>(tuple<int>{2}).size();
  REQUIRE(prefixLookup4 == 2);
}

TEST_CASE("ThreeColumnTable", "[ThreeColumn]") {
  vector<tuple<int, string, int>> data = {
      tuple<int, string, int>{1, "s", 2}, tuple<int, string, int>{1, "p", 3},
      tuple<int, string, int>{2, "s", 6}, tuple<int, string, int>{1, "d", 5},
      tuple<int, string, int>{2, "f", 8}};
  SonicIndex<10, 2, 0, int, string, int> sonic(data);

  auto pointLookup1 = sonic.pointLookup(tuple<int, string, int>{1, "s", 2});
  REQUIRE(pointLookup1 == 1);

  auto pointLookup2 = sonic.pointLookup(tuple<int, string, int>{2, "f", 8});
  REQUIRE(pointLookup2 == 1);

  auto pointLookup3 = sonic.pointLookup(tuple<int, string, int>{1, "q", 2});
  REQUIRE(pointLookup3 == 0);

  auto pointLookup4 = sonic.pointLookup(tuple<int, string, int>{1, "p", 2});
  REQUIRE(pointLookup4 == 0);

  auto pointLookup5 = sonic.pointLookup(tuple<int, string, int>{5, "d", 1});
  REQUIRE(pointLookup5 == 0);

  auto countPrefix1 = sonic.template countPrefix<int, string>(tuple<int, string>{1, "s"});
  REQUIRE(countPrefix1 == 1);

  auto countPrefix2 = sonic.template countPrefix<int>(tuple<int>{1});
  REQUIRE(countPrefix2 == 3);

  auto countPrefix3 = sonic.template countPrefix<int, string>(tuple<int, string>{2, "p"});
  REQUIRE(countPrefix3 == 0);

  auto countPrefix4 = sonic.template countPrefix<int, string>(tuple<int, string>{2, "s"});
  REQUIRE(countPrefix4 == 1);

  auto countPrefix5 = sonic.template countPrefix<int>(tuple<int>{3});
  REQUIRE(countPrefix5 == 0);

  auto countPrefix6 = sonic.template countPrefix<int>(tuple<int>{2});
  REQUIRE(countPrefix6 == 2);

  auto prefixLookup1 = sonic.template prefixLookup<int, string>(tuple<int, string>{1, "s"}).size();
  REQUIRE(prefixLookup1 == 1);

  auto prefixLookup2 = sonic.template prefixLookup<int>(tuple<int>{1}).size();
  REQUIRE(prefixLookup2 == 3);

  auto prefixLookup3 = sonic.template prefixLookup<int, string>(tuple<int, string>{2, "p"}).size();
  REQUIRE(prefixLookup3 == 0);

  auto prefixLookup4 = sonic.template prefixLookup<int, string>(tuple<int, string>{2, "s"}).size();
  REQUIRE(prefixLookup4 == 1);

  auto prefixLookup5 = sonic.template prefixLookup<int>(tuple<int>{3}).size();
  REQUIRE(prefixLookup5 == 0);

  auto prefixLookup6 = sonic.template prefixLookup<int>(tuple<int>{2}).size();
  REQUIRE(prefixLookup6 == 2);
}

TEST_CASE("FourColumnTable", "[FourColumn]") {
  vector<tuple<int, string, int, string>> data = {tuple<int, string, int, string>{1, "s", 2, "ss"},
                                                  tuple<int, string, int, string>{1, "p", 3, "ss"},
                                                  tuple<int, string, int, string>{1, "s", 6, "ss"},
                                                  tuple<int, string, int, string>{1, "d", 5, "x"},
                                                  tuple<int, string, int, string>{2, "f", 8, "q"},
                                                  tuple<int, string, int, string>{2, "p", 8, "q"},
                                                  tuple<int, string, int, string>{1, "s", 4, "q"}};
  SonicIndex<14, 2, 0, int, string, int, string> sonic(data);

  auto pointLookup1 = sonic.pointLookup(tuple<int, string, int, string>{1, "s", 2, "ss"});
  REQUIRE(pointLookup1 == 1);

  auto pointLookup2 = sonic.pointLookup(tuple<int, string, int, string>{2, "f", 8, "q"});
  REQUIRE(pointLookup2 == 1);

  auto pointLookup3 = sonic.pointLookup(tuple<int, string, int, string>{1, "q", 1, "x"});
  REQUIRE(pointLookup3 == 0);

  auto pointLookup4 = sonic.pointLookup(tuple<int, string, int, string>{1, "p", 2, "ss"});
  REQUIRE(pointLookup4 == 0);

  auto pointLookup5 = sonic.pointLookup(tuple<int, string, int, string>{5, "d", 1, "ss"});
  REQUIRE(pointLookup5 == 0);

  auto pointLookup6 = sonic.pointLookup(tuple<int, string, int, string>{1, "s", 4, "q"});
  REQUIRE(pointLookup6 == 1);

  auto countPrefix1 = sonic.template countPrefix<int, string>(tuple<int, string>{1, "s"});
  REQUIRE(countPrefix1 == 3);

  auto countPrefix2 = sonic.template countPrefix<int>(tuple<int>{1});
  REQUIRE(countPrefix2 == 5);

  auto countPrefix3 = sonic.template countPrefix<int, string>(tuple<int, string>{2, "p"});
  REQUIRE(countPrefix3 == 1);

  auto countPrefix4 = sonic.template countPrefix<int, string>(tuple<int, string>{2, "q"});
  REQUIRE(countPrefix4 == 0);

  auto countPrefix5 = sonic.template countPrefix<int>(tuple<int>{3});
  REQUIRE(countPrefix5 == 0);

  auto countPrefix6 = sonic.template countPrefix<int>(tuple<int>{2});
  REQUIRE(countPrefix6 == 2);

  auto prefixLookup1 = sonic.template prefixLookup<int, string>(tuple<int, string>{1, "s"}).size();
  REQUIRE(prefixLookup1 == 3);

  auto prefixLookup2 = sonic.template prefixLookup<int>(tuple<int>{1}).size();
  REQUIRE(prefixLookup2 == 5);

  auto prefixLookup3 = sonic.template prefixLookup<int, string>(tuple<int, string>{2, "p"}).size();
  REQUIRE(prefixLookup3 == 1);

  auto prefixLookup4 = sonic.template prefixLookup<int, string>(tuple<int, string>{2, "q"}).size();
  REQUIRE(prefixLookup4 == 0);

  auto prefixLookup5 = sonic.template prefixLookup<int>(tuple<int>{3}).size();
  REQUIRE(prefixLookup5 == 0);

  auto prefixLookup6 = sonic.template prefixLookup<int>(tuple<int>{2}).size();
  REQUIRE(prefixLookup6 == 2);
}

TEST_CASE("VariableLengthKey", "[KeyLength]") {
  vector<tuple<string, string, string, string, string, string>> data = {
      tuple<string, string, string, string, string, string>{"first", "second", "third", "fourth",
                                                            "fifth", "sixth"},
      tuple<string, string, string, string, string, string>{"first", "second", "third", "fourth",
                                                            "fifth two", "sixth two"},
      tuple<string, string, string, string, string, string>{
          "first column", "second column", "This is a long key for third column", "fourth",
          "This is test for sonic index", "last column"},
      tuple<string, string, string, string, string, string>{
          "first column", "second column", "third column", "fourth", "This is test for sonic index",
          "last column"},
  };
  SonicIndex<8, 2, 0, string, string, string, string, string, string> sonic(data);

  auto pointLookup1 = sonic.pointLookup(tuple<string, string, string, string, string, string>{
      "first", "second", "third", "fourth", "fifth", "sixth"});
  REQUIRE(pointLookup1 == 1);

  auto countPrefix1 = sonic.template countPrefix<string>(tuple<string>{"first column"});
  REQUIRE(countPrefix1 == 2);

  auto prefixLookup1 = sonic.template prefixLookup<string>(tuple<string>{"first"}).size();
  REQUIRE(prefixLookup1 == 2);
}