#include "state_gen.hpp"

using namespace majorminer;

void StateGen::addMapping(std::initializer_list<edge_t> mapping)
{
  m_mapping.insert(mapping.begin(), mapping.end());
}

void StateGen::addMapping(vertex_t source, std::initializer_list<vertex_t> targetVertices)
{
  for (auto target : targetVertices)
  {
    m_mapping.insert(std::make_pair(source, target));
  }
}

void StateGen::addMapping(const embedding_mapping_t& mapping)
{
  for (const auto& mapped : mapping)
  {
    m_mapping.insert(mapped);
  }
}

void StateGen::addMapping(vertex_t source, const nodeset_t& mapping)
{
  for (auto target : mapping) m_mapping.insert(std::make_pair(source, target));
}

std::unique_ptr<EmbeddingState> StateGen::get()
{
  auto state = std::make_unique<EmbeddingState>(m_source, m_target, nullptr);
  auto& remaining = state->getRemainingTargetNodes();
  auto& occupied = state->getNodesOccupied();
  for (const auto& edge : m_mapping)
  {
    state->mapNode(edge.first, edge.second);
    remaining.unsafe_erase(edge.second);
    occupied.insert(edge.first);
  }
  return state;
}

void StateGen::removeSuperVertex(vertex_t sourceVertex)
{
  graph_t remove{};
  for (const auto& mapped : m_mapping)
  {
    if (mapped.first == sourceVertex) remove.insert(mapped);
  }
  for (const auto& mapped : remove) m_mapping.unsafe_erase(mapped);
}
