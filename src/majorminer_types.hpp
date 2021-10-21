#ifndef __MAJORMINER_TYPES_HPP_
#define __MAJORMINER_TYPES_HPP_

#include <vector>
#include <map>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_unordered_map.h>

#include <cinttypes>


namespace majorminer
{
  typedef uint_fast32_t fuint32_t;

  template<typename K, typename V>
  struct PairHashFunc
  {
    size_t operator()(const std::pair<K,V>& pair) const
    {
      size_t first = std::hash<K>()(pair.first);
      size_t second = std::hash<V>()(pair.second);
      return first ^ (second << 32) ^ (second >> 32);
    }
  };

  template<typename T, typename Allocator = std::allocator<T>>
  using Vector = tbb::concurrent_vector<T, Allocator>;

  template<class T, typename HashFunc = std::hash<T>, typename Allocator = std::allocator<T>>
  using UnorderedSet = tbb::concurrent_unordered_set<T, HashFunc, std::equal_to<T>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = tbb::concurrent_unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  typedef std::pair<fuint32_t, fuint32_t> fuint32_pair_t;
  typedef fuint32_pair_t edge_t;

  typedef UnorderedSet<edge_t, PairHashFunc<fuint32_t, fuint32_t>> graph_t;
}


#endif