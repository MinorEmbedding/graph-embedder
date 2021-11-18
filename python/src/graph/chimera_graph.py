from src.graph.undirected_graph import UndirectedGraphAdjList


class ChimeraGraphLayout(UndirectedGraphAdjList):
    """A Chimera Graph representation.

    Note:
        As of now, this Graph is limited to one single unit cell with
        shore size 4.
    """

    # Nodes of one unit cell of a Chimera Graph:
    #        4
    #
    #        5
    #  0   1   2   3
    #        6
    #
    #        7

    def __init__(self):
        shore_size = 4  # TODO: init layout of ChimeraGraph dynamically
        nodes_count = shore_size * 2
        super().__init__(nodes_count)

        # --- Define layout of Chimera graphs (right now: one unit cell)
        for i in range(0, 4):  # TODO: use shore_size here
            for j in range(4, 8):
                self.set_edge(i, j)
