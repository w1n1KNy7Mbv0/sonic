#ifndef _JOIN_INTERFACES_H_
#define _JOIN_INTERFACES_H_

#include <cstddef>

#include "../../header/generic_join.h"
#include "../../header/relation.h"
#include "qptree-s.h"

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join3(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree3::total_order.begin(), qptree3::total_order.end()),
                      qptree3::hyperedges);

  /// Has to be computed from solving the optimization problem
  vector<double> fractional_cover(qptree3::hyperedges.size(),
                                  1 / (double)qptree3::hyperedges.size());

  //// Call Perform Join Function
  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join5(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree5::total_order.begin(), qptree5::total_order.end()),
                      qptree5::hyperedges);

  /// Has to be computed from solving the optimization problem
  vector<double> fractional_cover(qptree5::hyperedges.size(),
                                  1 / (double)qptree5::hyperedges.size());

  //// Call Perform Join Function
  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

#endif