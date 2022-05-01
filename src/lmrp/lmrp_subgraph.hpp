#ifndef __MAJORMINER_LMRP_SUBGRAPH_HPP_
#define __MAJORMINER_LMRP_SUBGRAPH_HPP_

#include <majorminer_types.hpp>


namespace majorminer
{
  class LMRPSubgraph
  {
    public:
      // Get a subgraph that contains the vertex "contained". Subgraph might not
      // be connected. If successful, method returns true.
      virtual bool getSubgraph(vertex_t contained, nodeset_t& subgraph) = 0;

      virtual bool isBeingDestroyed(vertex_t contained) = 0;

      // If a LMRP instance was solved, free the mapped vertices
      virtual void commit(vertex_t contained) = 0;
  };
}


#endif