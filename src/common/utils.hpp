#ifndef __MAJORMINER_UTILS_HPP_
#define __MAJORMINER_UTILS_HPP_

#include <majorminer_types.hpp>
#include <common/debug_utils.hpp>


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

  template<typename K, typename V>
  bool containsPair(const UnorderedMultiMap<K, V>& umap, const K& key, const V& val)
  {
    auto range = umap.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == val)
      {
        return true;
      }
    }
    return false;
  }

  void insertMappedTargetNodes(const EmbeddingBase& base, nodeset_t& nodes, fuint32_t sourceNode);

  inline bool isDefined(fuint32_t value) { return value != FUINT32_UNDEF; }
  inline bool isDefined(fuint32_pair_t& p) { return isDefined(p.first) && isDefined(p.second); }
  inline bool isDefined(NodePair& p) { return isDefined(p.source) && isDefined(p.target); }

  template<typename T>
  inline std::shared_ptr<T[]> make_shared_array(std::size_t size)
  {
      return std::shared_ptr<T[]>( new T[size], []( T *p ){ delete [] p; } );
  }

  template<typename T, typename Hash>
  inline bool overlappingSets(const UnorderedSet<T, Hash>& map1, const UnorderedSet<T, Hash>& map2)
  {
    for (const auto& key : map1)
    {
      if (map2.contains(key)) return true;
    }
    return false;
  }

  template<typename T>
  inline void clearStack(Stack<T>& s)
  {
    while(!s.empty()) s.pop();
  }

  nodeset_t getVertices(const graph_t& graph);
}



#endif