#include "super_vertex_placer.hpp"

#include <common/embedding_manager.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>

using namespace majorminer;

SuperVertexPlacer::SuperVertexPlacer(EmbeddingState& state, EmbeddingManager& embeddingManager)
  : m_state(state), m_embeddingManager(embeddingManager)
{}

void SuperVertexPlacer::operator()()
{
  if (!m_nodesToProcess.empty())
  {
    if (!connectedNode()) return;
  }
  else
  {
    trivialNode();
  }
}

void SuperVertexPlacer::trivialNode()
{
  // just pick an arbitrary node and place it somewhere
  fuint32_t node = m_state.getTrivialNode();
  DEBUG(OUT_S << "Trivial node to embedd: " << node << std::endl;)
  embeddTrivialNode(node);

  if (m_state.hasVisualizer()) visualize(node, TRIVIAL);
}

bool SuperVertexPlacer::connectedNode()
{
  PrioNode node{};
  bool found = m_nodesToProcess.try_pop(node);
  if (!found) return false;

  if (!m_state.removeRemainingNode(node.m_id)) return false;


  if (node.m_nbConnections > 1)
  {
    DEBUG(OUT_S << "Complex node to embedd: " << node.m_id << " (" << node.m_nbConnections << ")" << std::endl;)

    embeddNode(node.m_id);
    if (m_state.hasVisualizer()) visualize(node.m_id, COMPLEX, node.m_nbConnections);

    // prepareFrontierShifting(node.m_id, node.m_nbConnections);
  }
  else
  { // nbConnections = 1
    DEBUG(OUT_S << "Simple adjacent node to embedd: " << node.m_id << std::endl;)

    embeddSimpleNode(node.m_id);
    m_state.updateConnections(node.m_id, m_nodesToProcess);

    if (m_state.hasVisualizer()) visualize(node.m_id, SIMPLE);
  }
  return true;
}

void SuperVertexPlacer::embeddNode(fuint32_t node)
{
  embeddNodeNetworkSimplex(node);
}


void SuperVertexPlacer::visualize(fuint32_t node, PlacedNodeType type, fuint32_t nbConnections)
{
  auto& visualizer = *m_state.getVisualizer();
  const auto& mapping = m_state.getMapping();
  switch(type)
  {
    case TRIVIAL:
    {
      visualizer.draw(mapping,
        [&](std::ostream& os)
          { os << "Trivial node " << node; }
      );
      break;
    }
    case SIMPLE:
    {
      visualizer.draw(mapping,
        [&](std::ostream &os)
          { os << "Simple adjacent node " << node << " (1)."; }
      );
      break;
    }
    case COMPLEX:
    {
      visualizer.draw(mapping,
        [&](std::ostream &os)
          { os << "Complex adjacent node " << node
               << " (" << nbConnections << ")"; }
      );
      break;
    }
  }
}


void SuperVertexPlacer::embeddNodeNetworkSimplex(fuint32_t node)
{
  if (m_nsWrapper.get() == nullptr) m_nsWrapper = std::make_unique<NetworkSimplexWrapper>(m_state, m_embeddingManager);

  m_nsWrapper->embeddNode(node);
}

void SuperVertexPlacer::embeddSimpleNode(fuint32_t node)
{
  // find a node that is adjacent to the node "adjacentNode"

  auto adjacentIt = m_state.getSourceAdjGraph().equal_range(node);
  const auto& nodesRemaining = m_state.getRemainingNodes();
  fuint32_t adjacentNode = -1;
  for (auto n = adjacentIt.first; n != adjacentIt.second; ++n)
  {
    if (!nodesRemaining.contains(n->second))
    {
      adjacentNode = n->second;
      break;
    }
  }
  if (adjacentNode == (fuint32_t)-1) throw std::runtime_error("Could not find the adjacent node.");


  fuint32_t bestNodeFound = adjacentNode;
  m_state.iterateSourceMappingAdjacent<true>(adjacentNode, [&bestNodeFound](fuint32_t candidate, fuint32_t){
    bestNodeFound = candidate;
    return true;
  });

  // map "node" to "bestNodeFound"
  m_embeddingManager.mapNode(node, bestNodeFound);
  m_state.updateNeededNeighbors(node);
}

void SuperVertexPlacer::embeddTrivialNode(fuint32_t node)
{
  auto& remainingTargetNodes = m_state.getRemainingTargetNodes();
  if (!remainingTargetNodes.empty())
  {
    auto targetNode = *remainingTargetNodes.begin();
    remainingTargetNodes.unsafe_erase(remainingTargetNodes.begin());
    m_embeddingManager.mapNode(node, targetNode);
    m_state.updateNeededNeighbors(node);
    m_state.updateConnections(node, m_nodesToProcess);
  }
  else
  {
    throw std::runtime_error("Oooops...");
  }
}
