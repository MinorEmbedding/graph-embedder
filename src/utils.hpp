#ifndef __MAJORMINER_UTILS_HPP_
#define __MAJORMINER_UTILS_HPP_

#include <tbb/parallel_for_each.h>

#include "majorminer_types.hpp"


namespace majorminer
{

  void convertToAdjacencyList(adjacency_list_t& adj, const graph_t& graph);

  template<typename T, typename Comparator = std::less<T>>
  std::pair<T,T> orderedPair(const T& e1, const T& e2)
  {
    if (Comparator()(e1, e2)) return std::make_pair(e1, e2);
    else return std::make_pair(e1, e2);
  }

  template<typename T, typename Comparator = std::less<T>>
  std::pair<T,T> orderedPair(const std::pair<T, T>& p)
  {
    return orderedPair(p.first, p.second);
  }

}



#endif