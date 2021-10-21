from src.graphs.undirected_graphs import (UndirectedGraphAdjList,
                                          UndirectedGraphAdjMatrix)

#            Chimera graph       Graph gird structure (not needed right now)
#                  4
#                                         x x x x ——4
#                  5                      x x x x ——5
#            0   1   2   3                x x x x ——6
#                  6                      x x x x ——7
#                                         | | | |
#                  7                      0 1 2 3
#


class ChimeraGraphLayout(UndirectedGraphAdjList):
    """
    A Chimera graph representation (right now with fixed shore_size of 4
    and a single unit cell TODO).
    """

    def __init__(self):
        shore_size = 4  # TODO: init layout of ChimeraGraph dynamically
        vertices_count = shore_size * 2
        super().__init__(vertices_count)

        # Define layout of Chimera graphs
        for i in range(0, 4):  # TODO: use shore_size here
            for j in range(4, 8):
                self._set_edge(i, j)

    def get_neighbor_nodes(self, from_node):
        return super()._get_neighbor_nodes(from_node)


class GraphEmbedding(UndirectedGraphAdjList):
    """
    A Graph Embedding
    """

    def __init__(self, nodes_count):
        self._chain_last = 1
        super().__init__(nodes_count)

    def get_neighbor_nodes(self, from_node):
        return super()._get_neighbor_nodes(from_node)

    def embed_edge(self, frm, to, chain=0):
        # Chain=0 means no chain, just a "normal" edge
        super()._set_edge(frm, to, cost=chain)

    def add_node(self):
        super()._add_node()

    def get_embedded_nodes(self):
        nodes = super()._get_nodes()
        # Filter for nodes that have an edge
        nodes = [node for node in nodes if self._adj_list[node].get()]
        return nodes

    def get_embedded_edges(self):
        return super()._get_edges()

    def get_embedding(self):
        nodes = self.get_embedded_nodes()
        edges = self.get_embedded_edges()
        return nodes, edges
