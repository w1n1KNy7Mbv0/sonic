#ifndef _QPTREE_H_
#define _QPTREE_H_

#include <array>
#include <cstddef>
#include <vector>

#include "../header/hypergraph.h"

namespace qptree3 {

const std::array<size_t, 3> total_order = {2, 1, 3};

std::vector<Hyperedge> hyperedges = {Hyperedge(1, std::vector<size_t>{2, 1}),
                                     Hyperedge(2, std::vector<size_t>{2, 3}),
                                     Hyperedge(3, std::vector<size_t>{1, 3})};

} // namespace qptree3

namespace qptree4 {

const std::array<size_t, 8> total_order = {1, 2, 3, 6, 8, 5, 4, 7};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{1, 2, 5, 4}), Hyperedge(2, std::vector<size_t>{1, 3, 6, 4}),
    Hyperedge(3, std::vector<size_t>{2, 3, 6, 7}), Hyperedge(4, std::vector<size_t>{8, 5, 4, 7})};

} // namespace qptree4

namespace qptree5 {

const std::array<size_t, 6> total_order = {1, 4, 2, 5, 3, 6};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{1, 4, 2, 5}), Hyperedge(2, std::vector<size_t>{1, 4, 3, 6}),
    Hyperedge(3, std::vector<size_t>{1, 2, 3}), Hyperedge(4, std::vector<size_t>{4, 2, 6}),
    Hyperedge(5, std::vector<size_t>{5, 3, 6})};

} // namespace qptree5

namespace qptreeRec {

const std::array<size_t, 4> total_order = {2, 3, 1, 4};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{2, 1}), Hyperedge(2, std::vector<size_t>{2, 3}),
    Hyperedge(3, std::vector<size_t>{3, 4}), Hyperedge(4, std::vector<size_t>{1, 4})};

} // namespace qptreeRec

namespace qptreePen {

const std::array<size_t, 5> total_order = {2, 3, 4, 1, 5};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{2, 1}), Hyperedge(2, std::vector<size_t>{2, 3}),
    Hyperedge(3, std::vector<size_t>{3, 4}), Hyperedge(4, std::vector<size_t>{4, 5}),
    Hyperedge(5, std::vector<size_t>{1, 5})};

} // namespace qptreePen

namespace qptree3_4column {

const std::array<size_t, 6> total_order = {3, 4, 1, 2, 5, 6};

std::vector<Hyperedge> hyperedges = {Hyperedge(1, std::vector<size_t>{3, 4, 1, 2}),
                                     Hyperedge(2, std::vector<size_t>{3, 4, 5, 6}),
                                     Hyperedge(3, std::vector<size_t>{1, 2, 5, 6})};

} // namespace qptree3_4column

namespace qptree4_4column {

const std::array<size_t, 8> total_order = {3, 4, 5, 6, 1, 2, 8, 7};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{3, 4, 1, 2}), Hyperedge(2, std::vector<size_t>{3, 4, 5, 6}),
    Hyperedge(3, std::vector<size_t>{5, 6, 8, 7}), Hyperedge(4, std::vector<size_t>{1, 2, 8, 7})};

} // namespace qptree4_4column

namespace qptree5_4column {

const std::array<size_t, 10> total_order = {3, 4, 5, 6, 8, 7, 1, 2, 9, 10};

std::vector<Hyperedge> hyperedges = {
    Hyperedge(1, std::vector<size_t>{3, 4, 1, 2}), Hyperedge(2, std::vector<size_t>{3, 4, 5, 6}),
    Hyperedge(3, std::vector<size_t>{5, 6, 8, 7}), Hyperedge(4, std::vector<size_t>{8, 7, 9, 10}),
    Hyperedge(5, std::vector<size_t>{1, 2, 9, 10})};

} // namespace qptree5_4column

namespace qptree6 {

const std::array<size_t, 13> total_order = {2, 3, 9, 7, 5, 4, 1, 11, 12, 13, 8, 6, 10};

std::vector<Hyperedge> hyperedges = {Hyperedge(1, std::vector<size_t>{2, 3, 4, 1}),
                                     Hyperedge(2, std::vector<size_t>{3, 4, 6}),
                                     Hyperedge(3, std::vector<size_t>{7, 5, 8}),
                                     Hyperedge(4, std::vector<size_t>{9, 7, 4, 10}),
                                     Hyperedge(5, std::vector<size_t>{5, 4, 6, 10}),
                                     Hyperedge(6, std::vector<size_t>{1, 11, 12, 13, 8, 6, 10})};

} // namespace qptree6

namespace qptree6_2 {

const std::array<size_t, 16> total_order = {9, 2, 4, 5, 3, 6, 8, 10, 7, 16, 1, 12, 13, 14, 15, 11};

std::vector<Hyperedge> hyperedges = {Hyperedge(1, std::vector<size_t>{2, 4, 5, 3, 1}),
                                     Hyperedge(2, std::vector<size_t>{4, 3, 6}),
                                     Hyperedge(3, std::vector<size_t>{5, 3, 6}),
                                     Hyperedge(4, std::vector<size_t>{6, 8, 10}),
                                     Hyperedge(5, std::vector<size_t>{8, 10, 7, 11}),
                                     Hyperedge(6, std::vector<size_t>{16, 1, 12, 13, 14, 15, 11})};

} // namespace qptree6_2

#endif
