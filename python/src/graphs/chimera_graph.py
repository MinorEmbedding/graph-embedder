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
                # cost=0 is just an edge that exists but is not used by the embedding
                self._set_edge(i, j)  # default cost: 0

    def get_neighbors(self, from_vertex):
        return super()._get_reachable_vertices(from_vertex)


class ChimeraGraphEmbedding(UndirectedGraphAdjMatrix):
    """
    A Chimera graph representation (right now with fixed shore_size of 4
    and a single unit cell TODO).
    """

    def __init__(self):
        shore_size = 4  # TODO: init layout of ChimeraGraph dynamically
        vertices_count = shore_size * 2
        super().__init__(vertices_count)
        self._embedded_nodes = []

        # Define layout of Chimera graphs
        for i in range(0, 4):  # TODO: use shore_size here
            for j in range(4, 8):
                # cost=0 is just an edge that exists but is not used by the embedding
                self._set_edge(i, j)  # default cost: 0

    def embed_edge(self, frm, to):
        # 1 is the default embedded edge that belongs to no chain
        super()._set_edge(frm, to, 1)
        self._embedded_nodes.append(frm)
        self._embedded_nodes.append(to)

    def get_embedded_edges(self):
        # TODO: add logic to indicate chains (use indexes like 1, 2, 3 to indicate groups)
        return [edge for edge in super()._get_edges() if edge[2] > 0]

    def get_embedded_nodes(self):
        return self._embedded_nodes
