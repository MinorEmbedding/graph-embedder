#ifndef __MAJORMINER_UTILS_HPP_
#define __MAJORMINER_UTILS_HPP_

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

  template<typename T>
  void setMin(T& val, const T& v) { if (v < val) val = v; }
  template<typename T>
  void setMax(T& val, const T& v) { if (v > val) val = v; }

  void printAdjacencyList(const adjacency_list_t& adj);

  template<typename K, typename V>
  void eraseSinglePair(UnorderedMultiMap<K, V>& umap, const K& key, const V& val)
  {
    auto range = umap.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == val)
      {
        umap.unsafe_erase(it);
        return;
      }
    }
  }
}



#endif