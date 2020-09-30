
#ifndef _QPTREE_H_
#define _QPTREE_H_

#include <array>
#include <cstddef>
#include <vector>

#include "../../header/hypergraph.h"

namespace qptree3 {

const std::array<size_t, 3> total_order = {2, 1, 3};

std::vector<Hyperedge> hyperedges = {Hyperedge(1, std::vector<size_t>{2, 1}),
                                     Hyperedge(2, std::vector<size_t>{2, 3}),
                                     Hyperedge(3, std::vector<size_t>{1, 3})};

} // namespace qptree3

namespace qptree5 {

const std::array<size_t, 6> total_order = {1, 4, 2, 5, 3, 6};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{1, 4, 2, 5}), Hyperedge(2, std::vector<size_t>{1, 4, 3, 6}),
    Hyperedge(3, std::vector<size_t>{1, 2, 3}), Hyperedge(4, std::vector<size_t>{4, 2, 6}),
    Hyperedge(5, std::vector<size_t>{5, 3, 6})};

} // namespace qptree5

#endif
