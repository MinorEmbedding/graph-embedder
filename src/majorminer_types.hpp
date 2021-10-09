#ifndef __MAJORMINER_TYPES_HPP_
#define __MAJORMINER_TYPES_HPP_

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <cinttypes>


namespace majorminer
{
  typedef uint_fast32_t fuint32_t;

  template<typename K, typename V>
  struct PairHashFunc
  {
    size_t operator()(const std::pair<K,V>& p) const
    {
      return (std::hash<K>()(p.first) << 32) ^ (std::hash<V>()(p.second) >> 32);
    }
  };

  template<typename T, typename Allocator>
  using Vector = std::vector<T, Allocator>;

  template<class T, typename HashFunc = std::hash<T>>
  using UnorderedSet = std::unordered_set<T, HashFunc, std::equal_to<T>, std::allocator<T>>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = std::unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  typedef std::pair<fuint32_t, fuint32_t> fuint32_pair_t;
  typedef fuint32_pair_t edge_t;

  typedef UnorderedSet<std::pair<fuint32_t, fuint32_t>, PairHashFunc<fuint32_t, fuint32_t>> graph_t;
}


#endif