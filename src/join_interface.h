#ifndef _JOIN_INTERFACES_H_
#define _JOIN_INTERFACES_H_

#include <cstddef>

#include "../header/generic_join.h"
#include "../header/relation.h"
#include "benchmark_qptree.h"

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join3(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree3::total_order.begin(), qptree3::total_order.end()),
                      qptree3::hyperedges);

  vector<double> fractional_cover(qptree3::hyperedges.size(),
                                  1 / (double)qptree3::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join4(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree4::total_order.begin(), qptree4::total_order.end()),
                      qptree4::hyperedges);

  vector<double> fractional_cover(qptree4::hyperedges.size(),
                                  1 / (double)qptree4::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join5(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree5::total_order.begin(), qptree5::total_order.end()),
                      qptree5::hyperedges);

  vector<double> fractional_cover(qptree5::hyperedges.size(),
                                  1 / (double)qptree5::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> joinRec(RelationSet const& tables) {
  auto H =
      Hypergraph(std::vector<size_t>(qptreeRec::total_order.begin(), qptreeRec::total_order.end()),
                 qptreeRec::hyperedges);

  vector<double> fractional_cover(qptreeRec::hyperedges.size(),
                                  1 / (double)qptreeRec::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> joinPen(RelationSet const& tables) {
  auto H =
      Hypergraph(std::vector<size_t>(qptreePen::total_order.begin(), qptreePen::total_order.end()),
                 qptreePen::hyperedges);

  vector<double> fractional_cover(qptreePen::hyperedges.size(),
                                  1 / (double)qptreePen::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join6(RelationSet const& tables) {
  auto H = Hypergraph(std::vector<size_t>(qptree6::total_order.begin(), qptree6::total_order.end()),
                      qptree6::hyperedges);

  vector<double> fractional_cover(qptree6::hyperedges.size(),
                                  1 / (double)qptree6::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

template <typename Tuple, typename RelationSet>
std::vector<Tuple> join6_2(RelationSet const& tables) {
  auto H =
      Hypergraph(std::vector<size_t>(qptree6_2::total_order.begin(), qptree6_2::total_order.end()),
                 qptree6_2::hyperedges);

  vector<double> fractional_cover(qptree6_2::hyperedges.size(),
                                  1 / (double)qptree6_2::hyperedges.size());

  Tuple t = tupleDefaultValue<Tuple>();
  return genericJoin<Tuple, RelationSet>(H, tables, fractional_cover, t);
}

#endif
