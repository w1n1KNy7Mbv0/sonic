#ifndef _HELPER_FUNCTIONS_H_
#define _HELPER_FUNCTIONS_H_

#include <algorithm>
#include <climits>
#include <cstdint>
#include <math.h>
#include <nmmintrin.h>
#include <string>
#include <tuple>
#include <utility>

#include "hypergraph.h"

using namespace std;

template <typename KeyType> struct CRCHash { size_t operator()(KeyType const& key) const; };

template <> inline size_t CRCHash<int>::operator()(int const& key) const {
  return _mm_crc32_u64(0, key);
}

template <> inline size_t CRCHash<long>::operator()(long const& key) const {
  return _mm_crc32_u64(0, key);
}

template <> inline size_t CRCHash<string>::operator()(string const& key) const {
  auto result = 0;

  for(auto i = 0; i < key.length(); i++) {
    result = _mm_crc32_u64(result, key[i]);
  }

  return result;
}

struct CRCHashHTrie {
  size_t operator()(const char* key, size_t keySize) const {
    auto result = 0;

    for(auto i = 0; i < keySize; i++) {
      result = _mm_crc32_u64(result, key[i]);
    }

    return result;
  }
};

template <typename T> inline void hashTuple(size_t& seed, const T& val) {
  seed ^= CRCHash<T>{}(val);
}

template <typename Tuple> size_t CRCHash<Tuple>::operator()(Tuple const& key) const {
  size_t result = 0;
  apply([&result](auto&&... args) { (hashTuple(result, args), ...); }, key);

  return result;
}

template <typename KeyType> struct DefaultValue { KeyType operator()() const; };
template <> inline int DefaultValue<int>::operator()() const { return 0; }
template <> inline long DefaultValue<long>::operator()() const { return 0; }
template <> inline string DefaultValue<string>::operator()() const { return ""; }

template <typename KeyType> struct MaxValue { KeyType operator()() const; };
template <> inline int MaxValue<int>::operator()() const { return INT_MAX; }
template <> inline long MaxValue<long>::operator()() const { return LONG_MAX; }
template <> inline string MaxValue<string>::operator()() const {
  return "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
}

inline vector<size_t> getIntersect(vector<size_t> const& u, vector<size_t> const& v) {
  vector<size_t> intersect;
  for(auto const& t : u) {
    if(find(v.begin(), v.end(), t) != v.end()) {
      intersect.emplace_back(t);
    }
  }
  return intersect;
}

template <typename Hyperedge, typename ColumnType>
inline uint16_t findArgMin(pair<vector<ColumnType>, vector<uint16_t>> const& tuple_s_set,
                           uint16_t k, vector<uint16_t> const& U,
                           vector<Hyperedge> const& hyperedges) {
  auto min_size = INT32_MAX;
  uint16_t j_idx;

  if(getIntersect(tuple_s_set.second, U).size() == 0) {
    return k - 1;
  }

  for(auto i = 0; i < k; i++) {
    auto proj_size = hyperedges[i].index.Find(tuple_s_set.first);
    if(proj_size < min_size) {
      j_idx = i;
      min_size = proj_size;
    }
  }

  return j_idx;
}

template <typename ColumnType>
inline vector<ColumnType> getProjection(vector<ColumnType> const& tuple,
                                        vector<uint16_t> const& tuple_attributes,
                                        vector<uint16_t> const& projection_attributes) {
  vector<ColumnType> result;

  for(auto a : projection_attributes) {
    auto iter = find(tuple_attributes.begin(), tuple_attributes.end(), a);
    if(iter != tuple_attributes.end() && tuple.size() > 0) {
      result.emplace_back(tuple[distance(tuple_attributes.begin(), iter)]);
    }
  }
  return result;
}

template <typename Hyperedge, typename ColumnType>
inline uint32_t
calculatePI(uint16_t k, vector<uint16_t> const& W_minus, vector<double> const& fractional_cover,
            vector<ColumnType> const& tuple_s_w, vector<Hyperedge> const& hyperedges) {
  auto pi_ei_w_minus = 1;
  for(auto i = 0; i < (k - 1); i++) {
    auto size_ei_w_minus = hyperedges[i].index.Find(tuple_s_w);
    pi_ei_w_minus *= pow(size_ei_w_minus, fractional_cover[i] / (1 - fractional_cover[k - 1]));
  }
  return pi_ei_w_minus;
}

inline size_t isSuffix(vector<size_t> const& V, vector<size_t> const& f) {
  for(auto i = 0; i < V.size(); i++) {
    if(f == vector<size_t>(V.begin() + i, V.end())) {
      return i;
    }
  }
  return V.size();
}

inline bool relationIsInHypergraph(size_t label, vector<Hyperedge> const& E) {
  for(auto e : E) {
    if(e.label == label) {
      return true;
    }
  }
  return false;
}

template <typename Tuple, size_t I = 0>
inline const bool isAttributeFilled(Tuple const& inputTuple, size_t indexOfAttribute) {
  if(indexOfAttribute == I) {
    return (get<I>(inputTuple) != DefaultValue<tuple_element_t<I, Tuple>>()());
  } else if constexpr(I + 1 < tuple_size<Tuple>::value) {
    return isAttributeFilled<Tuple, I + 1>(inputTuple, indexOfAttribute);
  }
  return false;
}

template <typename AttributesIndexTuple, typename OutputTuple, size_t I = 0>
inline const size_t findPrefixLength(AttributesIndexTuple const& prefixTuple,
                                     OutputTuple const& totalOrderTuple) {
  size_t prefixLength = 0;
  if(isAttributeFilled(totalOrderTuple, get<I>(prefixTuple))) {
    prefixLength++;
  } else {
    return 0;
  }
  if constexpr(I + 1 < tuple_size<AttributesIndexTuple>::value) {
    prefixLength +=
        findPrefixLength<AttributesIndexTuple, OutputTuple, I + 1>(prefixTuple, totalOrderTuple);
  }
  return prefixLength;
}

template <class Tuple, class F, size_t... I>
constexpr F for_each_impl(Tuple&& t, F&& f, index_sequence<I...> const&) {
  return (void)initializer_list<int>{(forward<F>(f)(get<I>(forward<Tuple>(t))), 0)...}, f;
}

template <class Tuple, class F> constexpr F for_each(Tuple&& t, F&& f) {
  return for_each_impl(forward<Tuple>(t), forward<F>(f),
                       make_index_sequence<tuple_size<remove_reference_t<Tuple>>::value>{});
}

template <typename ColumnType> struct DataTypeParser {
  ColumnType operator()(string const& element) const;
  ColumnType operator()(int const& element) const;
};

template <> inline int DataTypeParser<int>::operator()(string const& element) const {
  auto result = 0;
  try {
    result = stoi(element);
  } catch(...) {
  }

  return result;
}

template <> inline long DataTypeParser<long>::operator()(string const& element) const {
  auto result = 0;

  try {
    result = stol(element);
  } catch(...) {
  }

  return result;
}

template <> inline string DataTypeParser<string>::operator()(string const& element) const {
  return element;
}

template <> inline int DataTypeParser<int>::operator()(int const& element) const { return element; }

template <typename T, typename ColumnTypesTuple, size_t... Indices>
auto vectorToTupleHelper(const vector<T>& v, index_sequence<Indices...> const&) {
  return make_tuple(DataTypeParser<tuple_element_t<Indices, ColumnTypesTuple>>()(v[Indices])...);
}

template <size_t N, typename T, typename ColumnTypesTuple> auto vectorToTuple(const vector<T>& v) {
  return vectorToTupleHelper<T, ColumnTypesTuple>(v, make_index_sequence<N>());
}

template <typename Tuple, size_t... Indices> Tuple tupleMaxInitializer(index_sequence<Indices...>) {
  return tuple{MaxValue<tuple_element_t<Indices, Tuple>>()()...};
}

template <typename Tuple> Tuple tupleMaxValue() {
  return tupleMaxInitializer<Tuple>(make_index_sequence<tuple_size_v<Tuple>>());
}

template <typename Tuple, size_t... Indices> Tuple tupleInitializer(index_sequence<Indices...>) {
  return tuple{DefaultValue<tuple_element_t<Indices, Tuple>>()()...};
}

template <typename Tuple> Tuple tupleDefaultValue() {
  return tupleInitializer<Tuple>(make_index_sequence<tuple_size_v<Tuple>>());
}

template <size_t> struct int_ {};

template <class Tuple, size_t Pos> ostream& print_tuple(ostream& out, const Tuple& t, int_<Pos>) {
  out << get<tuple_size<Tuple>::value - Pos>(t) << ',';
  return print_tuple(out, t, int_<Pos - 1>());
}

template <class Tuple> ostream& print_tuple(ostream& out, const Tuple& t, int_<1>) {
  return out << get<tuple_size<Tuple>::value - 1>(t);
}

template <class... Args> ostream& operator<<(ostream& out, const tuple<Args...>& t) {
  out << '(';
  print_tuple(out, t, int_<sizeof...(Args)>());
  return out << ')';
}

template <typename T> inline void removeDuplicates(vector<T>& v) {
  for(auto it = v.begin(); it != v.end();) {
    if(find(v.begin(), v.end(), (*it)) != v.end()) {
      v.erase(it);
    } else {
      ++it;
    }
  }
}

template <class... Formats, size_t N, size_t... Is>
std::tuple<Formats...> arrayToTuple(std::array<int, N> const& arr, std::index_sequence<Is...>) {
  return std::make_tuple(Formats{arr[Is]}...);
}

template <class... Formats, size_t N, class = std::enable_if_t<(N == sizeof...(Formats))>>
std::tuple<Formats...> arrayToTuple(std::array<int, N> const& arr) {
  return arrayToTuple<Formats...>(arr, std::make_index_sequence<N>{});
}

inline string toString(string const& s) { return s; }

inline string toString(int i) { return to_string(i); }

inline string toString(long l) { return to_string(l); }

inline string toString(size_t n) { return to_string(n); }

template <typename Tuple> string tupleToString(Tuple t) {
  string s;
  for_each(t, [&](auto const& e) { s = s + toString(e) + ","; });
  s = s.substr(0, s.length() - 1);
  return s;
}

template <typename... Columns> tuple<Columns...> stringToTuple(string const& s) {
  auto tuple_str = s;
  vector<string> elements;
  elements.reserve(sizeof...(Columns));

  while(tuple_str.find(",") != string::npos) {
    elements.emplace_back(tuple_str.substr(0, tuple_str.find(",")));
    tuple_str = tuple_str.substr(tuple_str.find(",") + 1);
  }
  elements.emplace_back(tuple_str);
  return vectorToTuple<sizeof...(Columns), string, tuple<Columns...>>(elements);
}

inline auto intToBytes(int n) {
  auto bytes = vector<char>();
  bytes.reserve(4);

  bytes.emplace_back(n & 0x000000ff);
  bytes.emplace_back((n & 0x0000ff00) >> 8);
  bytes.emplace_back((n & 0x00ff0000) >> 16);
  bytes.emplace_back((n & 0xff000000) >> 24);

  return bytes;
}

inline auto bytesToInt(vector<char> bytes) {
  int n = 0;
  n = n + (bytes[0] & 0x000000ff);
  n = n + ((bytes[1] & 0x000000ff) << 8);
  n = n + ((bytes[2] & 0x000000ff) << 16);
  n = n + ((bytes[3] & 0x000000ff) << 24);

  return n;
}

inline string elementToString(int n) {
  auto bytes = intToBytes(n);
  string s(bytes.begin(), bytes.end());
  return s;
}

inline string elementToString(string s) { return ("#" + s + "#"); }

template <typename Tuple> inline auto tupleToString2(Tuple t) {
  string result = "";
  for_each(t, [&](auto const& e) { result += elementToString(e); });
  return result;
}

template <typename Tuple> inline Tuple stringToTuple2(string s) {
  vector<string> elements;
  for(auto i = 0; i < s.length(); i++) {
    if(s.at(i) == '#') {
      auto j = i + 1;
      while(s.at(j) == '#') {
        j++;
      }
      elements.emplace_back(s.substr(i + 1, j - i));
      i = j + 1;
    } else {
      vector<char> bytes;
      for(auto j = 0; j < 4; j++) {
        bytes.emplace_back(s[j]);
      }
      elements.emplace_back(to_string(bytesToInt(bytes)));
      i += 3;
    }
  }
  return vectorToTuple<tuple_size_v<Tuple>, string, Tuple>(elements);
}

template <typename Tuple> inline auto tupleToBytes(Tuple t, char* const data) {
  auto elementCounter = 0;
  for_each(t, [&](auto const& e) {
    auto bytes = intToBytes(e);
    for(auto i = 0; i < sizeof(int); i++) {
      data[elementCounter * sizeof(int) + i] = bytes[i];
    }
    elementCounter++;
  });
}

template <typename Tuple> inline auto bytesToTuple(char* const data, size_t numberOfElements) {
  auto elements = vector<int>();
  elements.reserve(numberOfElements);
  for(auto i = 0; i < numberOfElements; i++) {
    auto bytes = vector<char>();
    bytes.reserve(sizeof(int));
    for(auto j = i * sizeof(int); j < (i + 1) * sizeof(int); j++) {
      bytes.emplace_back(data[j]);
    }
    elements.emplace_back(bytesToInt(bytes));
  }
  return vectorToTuple<tuple_size_v<Tuple>, int, Tuple>(elements);
}

#endif
