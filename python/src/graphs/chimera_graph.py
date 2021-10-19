from src.graphs.undirected_graph import UndirectedGraph


class ChimeraGraph(UndirectedGraph):
    """
    A Chimera graph representation (right now with fixed shore_size of 4
    and a single unit cell TODO).
    """

    #            Chimera graph       Graph gird structure (not needed right now)
    #                  4
    #                                         x x x x ——4
    #                  5                      x x x x ——5
    #            0   1   2   3                x x x x ——6
    #                  6                      x x x x ——7
    #                                         | | | |
    #                  7                      0 1 2 3
    #

    def __init__(self):
        shore_size = 4  # TODO: init layout of ChimeraGraph dynamically
        vertices_count = shore_size * 2
        super().__init__(vertices_count)

        # Define layout of Chimera graphs
        for i in range(0, 4):  # TODO: use shore_size here
            for j in range(4, 8):
                # cost=0 is just an edge that exists but is not used by the embedding
                self._set_edge(i, j)
