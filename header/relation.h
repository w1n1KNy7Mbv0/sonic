#ifndef _RELATION_H_
#define _RELATION_H_

#include <tuple>
#include <vector>

template <size_t... I> using AttributeIndex = std::index_sequence<I...>;

template <typename... T> class Relation;

template <typename Index, typename InputTupleSchema, size_t... IndexInTotalOrder,
          size_t... OrderedIndex>
class Relation<Index, InputTupleSchema, AttributeIndex<IndexInTotalOrder...>,
               AttributeIndex<OrderedIndex...>> {
public:
  size_t label;
  Index wrappedIndex;

  decltype(std::tuple{IndexInTotalOrder...}) attributesInTotalOrder =
      std::tuple{IndexInTotalOrder...};

  Relation(size_t const label_, std::vector<InputTupleSchema> const& input_tuples) : label(label_) {
    for(auto i = 0; i < input_tuples.size(); i++) {
      wrappedIndex.insertIntoIndex(std::tuple{std::get<OrderedIndex>(input_tuples[i])...});
    }
  }

  Relation(Relation& r) = delete;
  Relation& operator=(Relation& r) = delete;
};

#endif
