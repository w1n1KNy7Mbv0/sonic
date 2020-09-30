import os
from pathlib import Path


class QPNode:
    def __init__(self, label, universe):
        self.label = label
        self.universe = universe
        self.left_child = None
        self.right_child = None

    def IsLeaf(self):
        return (self.left_child == None and
                self.right_child == None)


class Relation:
    def __init__(self, label, attributes):
        self.label = label
        self.attributes = attributes


def BuildQPTree(universe, relations, relation_index):
    has_intersect = False
    for i in range(relation_index):
        if(set(universe) & set(relations[i].attributes)):
            has_intersect = True

    if(not has_intersect):
        return None

    u = QPNode(relation_index, universe)

    is_leaf = True
    for i in range(relation_index):
        if(not set(universe).issubset(relations[i].attributes)):
            is_leaf = False

    if(relation_index > 1 and not is_leaf):
        u.left_child = BuildQPTree(
            set(universe).difference(
                relations[relation_index-1].attributes
            ),
            relations,
            relation_index - 1)
        u.right_child = BuildQPTree(
            (set(universe) & set(relations[relation_index-1].attributes)),
            relations,
            relation_index - 1)

    return u


def GetTotalOrder(root, total_order):
    if root.IsLeaf():
        total_order += root.universe
    elif root.left_child == None:
        GetTotalOrder(root.right_child, total_order)
    elif root.right_child == None:
        GetTotalOrder(root.left_child, total_order)
        for a in root.universe:
            if a not in root.left_child.universe:
                total_order.append(a)
    else:
        GetTotalOrder(root.left_child, total_order)
        GetTotalOrder(root.right_child, total_order)


def GenrateTotalOrderFile(total_order, relations):
    qptree_file = open(str(Path(os.getcwd()).parent) +
                       "/header/qptree-s.h", "w")
    content = \
        """
    # ifndef _QPTREE_H_
    # define _QPTREE_H_

    # include <vector>
    # include <cstddef>
    # include <array>

    # include "hypergraph.h"

    namespace qptree6{

    """

    total_order_str = "const std::array<size_t, " + \
        str(len(total_order))+"> total_order = {"
    for a in total_order:
        total_order_str = total_order_str + str(a) + ","
    total_order_str = total_order_str[:len(total_order_str)-1] + "};\n\n"

    content += total_order_str

    relations_attribute_sequence = "template<typename ... Sequence> auto relations_attributes_orders = std::tuple<Sequence...>({"
    hyperedges_init_str = "\tstd::vector<Hyperedge> hyperedges = {"
    for r in relations:
        ordered_attributes = {}
        hyperedge_str = "\n\t\tHyperedge("+str(r.label)+",std::vector<size_t>{"
#        relation_order_sequence = "std::integer_sequence<size_t, "

        for a in r.attributes:
            ordered_attributes[str(a)] = total_order.index(a)

        sorted_ordered_attributes = {k: v for k, v in sorted(
            ordered_attributes.items(), key=lambda item: item[1])}

        for k, v in sorted_ordered_attributes.items():
            #            relation_order_sequence = relation_order_sequence + str(v) + ","
            hyperedge_str = hyperedge_str + str(k) + ","

        hyperedge_str = hyperedge_str[:len(hyperedge_str)-1] + "}),"
        hyperedges_init_str = hyperedges_init_str + hyperedge_str

        # relation_order_sequence = relation_order_sequence[:len(relation_order_sequence)-1] + ">{}"
        # relations_attribute_sequence = relations_attribute_sequence + relation_order_sequence + ","

    relations_attribute_sequence = relations_attribute_sequence[:len(
        relations_attribute_sequence)-1] + "});\n"

    hyperedges_init_str = hyperedges_init_str[:len(
        hyperedges_init_str)-1] + "};\n"

    content += hyperedges_init_str

#    content += relations_attribute_sequence

    content += \
        """
        }

        # endif
        """
    qptree_file.write(content)
    qptree_file.close()


total_order = []

# relations = [Relation(1, [1,2,4,5]), Relation(2, [1,3,4,6]),\
#             Relation(3, [1,2,3]), Relation(4, [2,4,6]),\
#             Relation(5, [3,5,6])]
# GetTotalOrder(BuildQPTree([1,2,3,4,5,6], relations,5),\
#             total_order)
# relations = [Relation(1, [1,2]), Relation(2, [2,3]),\
#             Relation(3, [1,3])]
# GetTotalOrder(BuildQPTree([1,2,3], relations,3),\
#             total_order)

# relations = [Relation(1, [1,2,4,5]), Relation(2, [1,3,4,6]),\
#             Relation(3, [2,3,6,7]), Relation(4, [7,4,5,8])]
# GetTotalOrder(BuildQPTree([1,2,3,4,5,6,7,8], relations,4),\
#             total_order)

# relations = [Relation(1, [1, 2]), Relation(2, [2, 3]),
#              Relation(3, [3, 4]), Relation(4, [4, 1])]
# GetTotalOrder(BuildQPTree([1, 2, 3, 4], relations, 4),
#               total_order)
# relations = [Relation(1, [1, 2]), Relation(2, [2, 3]),
#              Relation(3, [3, 4]), Relation(4, [4, 5]), Relation(5, [5, 1])]
# relations = [Relation(1, [1, 2, 3, 4]), Relation(2, [3, 4, 5, 6]),
#              Relation(3, [5, 6, 7, 8]), Relation(4, [7, 8, 9, 10]), Relation(5, [9, 10, 1, 2])]

relations = [Relation(1, [1, 2, 3, 4, 5]), Relation(2, [3, 4, 6]),
             Relation(3, [3, 5, 6]), Relation(4, [6, 8, 10]), Relation(5, [7, 8, 10, 11]), Relation(6, [11, 1, 12, 13, 14, 15, 16])]


GetTotalOrder(BuildQPTree([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16], relations, 6),
              total_order)
GenrateTotalOrderFile(total_order, relations)
