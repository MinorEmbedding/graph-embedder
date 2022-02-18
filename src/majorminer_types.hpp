#ifndef __MAJORMINER_TYPES_HPP_
#define __MAJORMINER_TYPES_HPP_

#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_set.h>
#include <tbb/concurrent_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <stack>
#include <vector>

#include <cinttypes>
#include <atomic>
#include <memory>

#include "config.hpp"


namespace majorminer
{
  typedef uint_fast32_t fuint32_t;

  template<typename K, typename V = K>
  struct PairHashFunc
  {
    size_t operator()(const std::pair<K,V>& pair) const
    {
      size_t first = std::hash<K>()(pair.first);
      size_t second = std::hash<V>()(pair.second);
      return first ^ (second << 32) ^ (second >> 32);
    }
  };

  struct PrioNode
  {
    PrioNode() : m_id(-1), m_nbConnections(0) {}
    PrioNode(fuint32_t id, fuint32_t nbConnections = 0)
      : m_id(id), m_nbConnections(nbConnections) {}

    friend bool operator<(const PrioNode& n1, const PrioNode& n2)
    { return n1.m_nbConnections < n2.m_nbConnections; }

    fuint32_t m_id;
    fuint32_t m_nbConnections;
  };

  template<typename T, typename Allocator = std::allocator<T>>
  using Vector = tbb::concurrent_vector<T, Allocator>;

  template<class T, typename HashFunc = std::hash<T>, typename Allocator = std::allocator<T>>
  using UnorderedSet = tbb::concurrent_unordered_set<T, HashFunc, std::equal_to<T>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = tbb::concurrent_unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMultiMap = tbb::concurrent_unordered_multimap<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename T, typename Comparator = std::less<T>>
  using PriorityQueue = tbb::concurrent_priority_queue<T, Comparator>;

  template<typename T>
  using Queue = tbb::concurrent_queue<T>;

  template<typename T>
  using BoundedQueue = tbb::concurrent_bounded_queue<T>;

  template<typename T>
  using Stack = std::stack<T, std::vector<T>>;

  typedef std::pair<fuint32_t, fuint32_t> fuint32_pair_t;
  typedef fuint32_pair_t edge_t;

  typedef UnorderedSet<edge_t, PairHashFunc<fuint32_t>> graph_t;
  typedef UnorderedMultiMap<fuint32_t, fuint32_t> adjacency_list_t;
  typedef adjacency_list_t embedding_mapping_t;
  typedef UnorderedSet<fuint32_t> nodeset_t;
  typedef PriorityQueue<PrioNode, std::less<PrioNode>> PrioNodeQueue;
  typedef std::pair<adjacency_list_t::const_iterator, adjacency_list_t::const_iterator> adjacency_list_range_iterator_t;

  class EmbeddingVisualizer;
  class EmbeddingSuite;
  class EmbeddingState;
  class EmbeddingManager;
  class SuperVertexPlacer;
  class GenericMutation;
  class MutationExtend;
  class MuationFrontierShifting;
  class MutationManager;

  struct FrontierShiftingData;
}


#endif