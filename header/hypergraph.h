#ifndef _HYPERGRAPH_H_
#define _HYPERGRAPH_H_

#include <cstddef>
#include <vector>

class Hyperedge {
public:
  Hyperedge(size_t hyperedge_label, std::vector<size_t> const& attribute_ids_)
      : label(hyperedge_label), attribute_ids(attribute_ids_) {}

  size_t label;
  std::vector<size_t> attribute_ids;
};

class Hypergraph {
public:
  Hypergraph(std::vector<size_t> const& universe, std::vector<Hyperedge> const& hyperedges)
      : V(universe), E(hyperedges) {}

  std::vector<size_t> V;
  std::vector<Hyperedge> E;
};

#endif