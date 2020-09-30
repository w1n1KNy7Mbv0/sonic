#ifndef _GENERIC_JOIN_H_
#define _GENERIC_JOIN_H_

#include "helper_functions.h"
#include "hypergraph.h"
#include "indices/sonic_wrapper.h"
#include "relation.h"

using namespace std;

namespace {

template <typename Tuple, typename RelationSet>
vector<Tuple> genericJoin(Hypergraph const& H, RelationSet const& R,
                          vector<double> const& fractional_cover, Tuple const& prefixTuple) {
  vector<Tuple> Q;

  if(H.V.size() == 1) {
    vector<Tuple> resultFromRelation;
    bool scannedRelation = false;

    for(auto i = 0; i < H.E.size(); i++) {
      if(find(H.E[i].attribute_ids.begin(), H.E[i].attribute_ids.end(), H.V[0]) !=
         H.E[i].attribute_ids.end()) {
        for_each(R, [&](auto const& r) {
          if((r.label == H.E[i].label) && !scannedRelation) {
            auto prefixLength = findPrefixLength(r.attributesInTotalOrder, prefixTuple);
            if(prefixLength) {
              resultFromRelation =
                  r.wrappedIndex.lookupByPrefixAndScatterIntoTuple(prefixTuple, prefixLength);
            } else {
              resultFromRelation = r.wrappedIndex.scanIndex(prefixTuple);
            }
            scannedRelation = true;

            for(auto tupleFromRelation : resultFromRelation) {
              auto found = true;
              for(auto j = i + 1; j < H.E.size(); j++) {
                if(find(H.E[j].attribute_ids.begin(), H.E[j].attribute_ids.end(), H.V[0]) !=
                   H.E[j].attribute_ids.end()) {
                  for_each(R, [&](auto const& nextRel) {
                    if(H.E[j].label == nextRel.label) {
                      if(nextRel.wrappedIndex.countPrefix(
                             tupleFromRelation, findPrefixLength(nextRel.attributesInTotalOrder,
                                                                 tupleFromRelation)) == 0) {
                        found = false;
                      }
                    }
                  });
                }
              }
              if(found) {
                Q.emplace_back(tupleFromRelation);
              }
            }
          }
        });
        if(scannedRelation) {
          break;
        }
      }
    }
    return Q;
  }

  size_t suffixIndex;
  size_t indexOfHyperedgeWithSuffix;
  vector<size_t> suffixAttributeIDs;
  for(auto i = H.E.size() - 1; i >= 0; i--) {
    suffixIndex = isSuffix(H.V, H.E[i].attribute_ids);
    if(suffixIndex < H.V.size()) {
      indexOfHyperedgeWithSuffix = i;
      suffixAttributeIDs = H.E[i].attribute_ids;
      break;
    }
  }

  vector<size_t> notSuffixAttributeIDs(H.V.begin(), H.V.begin() + suffixIndex);
  vector<Hyperedge> E1, E2;

  vector<double> Y1, Y2;

  for(auto i = 0; i < H.E.size(); i++) {
    auto notSuffixIntersection = getIntersect(H.E[i].attribute_ids, notSuffixAttributeIDs);
    auto suffixIntersection = getIntersect(H.E[i].attribute_ids, suffixAttributeIDs);

    if(notSuffixIntersection.size() > 0) {
      Hyperedge h(H.E[i].label, notSuffixIntersection);
      E1.emplace_back(h);
      Y1.emplace_back(fractional_cover[i]);
    }
    if(suffixIntersection.size() > 0 && i != indexOfHyperedgeWithSuffix) {
      Hyperedge h(H.E[i].label, suffixIntersection);
      E2.emplace_back(h);
      Y2.emplace_back(fractional_cover[i] / (1 - fractional_cover[indexOfHyperedgeWithSuffix]));
    }
  }

  auto H1 = Hypergraph(notSuffixAttributeIDs, E1);
  auto L = genericJoin(H1, R, Y1, prefixTuple);

  for(auto t : L) {
    for_each(R, [&](auto const& r_f) {
      if(r_f.label == H.E[indexOfHyperedgeWithSuffix].label) {
        auto pi = 1;
        for(auto i = 0; i < E2.size(); i++) {
          for_each(R, [&](auto const& r_e) {
            if(r_e.label == E2[i].label) {
              pi *= pow(
                  r_e.wrappedIndex.countPrefix(t, findPrefixLength(r_e.attributesInTotalOrder, t)),
                  Y2[i]);
            }
          });
        }

        if((fractional_cover[indexOfHyperedgeWithSuffix] < 1) &&
           (r_f.wrappedIndex.getIndexSize() >= pi) && (E2.size() > 0)) {
          auto H2 = Hypergraph(suffixAttributeIDs, E2);
          auto Z = genericJoin(H2, R, Y2, t);
          for(auto t_prime : Z) {
            if(r_f.wrappedIndex.countPrefix(
                   t_prime, findPrefixLength(r_f.attributesInTotalOrder, t_prime)) != 0) {
              Q.emplace_back(t_prime);
            }
          }
        } else {
          auto f_prefixLength = findPrefixLength(r_f.attributesInTotalOrder, t);
          vector<Tuple> R_f_tuples;

          if(f_prefixLength > 0) {
            R_f_tuples = r_f.wrappedIndex.lookupByPrefixAndScatterIntoTuple(t, f_prefixLength);
          } else {
            R_f_tuples = r_f.wrappedIndex.scanIndex(t);
          }
          for(auto t_prime : R_f_tuples) {
            auto found = true;
            for(auto e : E2) {
              for_each(R, [&](auto const& r_e) {
                if(r_e.label == e.label) {
                  if(r_e.wrappedIndex.countPrefix(
                         t_prime, findPrefixLength(r_e.attributesInTotalOrder, t_prime)) == 0) {
                    found = false;
                  }
                }
              });
            }
            if(found) {
              Q.emplace_back(t_prime);
            }
          }
        }
      }
    });
  }
  return Q;
}
} // namespace

#endif
