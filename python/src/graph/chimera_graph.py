import dwave_networkx as dnx
from src.graph.undirected_graph import UndirectedGraphAdjList


class ChimeraGraphLayout(UndirectedGraphAdjList):

    def __init__(self, m: int, n: int, t=4):
        """Creates a Chimera lattice of size (m, n, t).

        Args:
            m (int): Number of rows in the Chimera lattice.
            n (int, optional, default m): Number of columns in the Chimera lattice.
            t (int, optional, default 4): Size of the shore within each Chimera tile.

        Returns:
            G : NetworkX Graph
                An (m, n, t) Chimera lattice. Nodes are labeled by integers.

        A Chimera lattice is an m-by-n grid of Chimera tiles. Each Chimera
        tile is itself a bipartite graph with shores of size t. The
        connection in a Chimera lattice can be expressed using a node-indexing
        notation (i,j,u,k) for each node.

        * (i,j) indexes the (row, column) of the Chimera tile. i must be
        between 0 and m-1, inclusive, and j must be between 0 and
        n-1, inclusive.
        * u=0 indicates the left-hand nodes in the tile, and u=1 indicates
        the right-hand nodes.
        * k=0,1,...,t-1 indexes nodes within either the left- or
        right-hand shores of a tile.

        In this notation, two nodes (i, j, u, k) and (i', j', u', k') are
        neighbors if and only if:

            (i = i' AND j = j' AND u != u') OR
            (i = i' +/- 1 AND j = j' AND u = 0 AND u' = 0 AND k = k') OR
            (i = i' AND j = j' +/- 1 AND u = 1 AND u' = 1 AND k = k')

        The first of the three terms of the disjunction gives the
        bipartite connections  within the tile. The second and third terms
        give the vertical and horizontal connections between blocks
        respectively.

        (Partially copied from the official dwave_networkx documentation.)
        """

        # --- Nodes
        nodes_count = m * n * 2 * t
        self.nodes_count = nodes_count
        super().__init__(nodes_count)

        # --- Edges
        G = dnx.chimera_graph(m, n, t)
        for e in G.edges:
            self.set_edge(e[0], e[1])

    def get_size(self) -> int:
        return self.nodes_count

    # def __init__(self, grid_width=1):
    #     # --- Nodes
    #     # One chimera cell has 8 nodes
    #     nodes_count = 8 * grid_width**2
    #     super().__init__(nodes_count)

    #     # --- Edges
    #     # Start from top-left corner to top-right
    #     # then go bottom
    #     cell_start_offset = 0

    #     for i in range(grid_width):
    #         for j in range(grid_width):

    #             # --- One Chimera cell
    #             for chimera_i in range(0, 4):
    #                 for chimera_j in range(4, 8):
    #                     self.set_edge(chimera_i + cell_start_offset,
    #                                   chimera_j + cell_start_offset)

    #             # --- Edges to other cells

    #             # 0,1,2,3 to bottom cell
    #             # Check that we are not one of most bottom cells
    #             if i != grid_width - 1:
    #                 for chimera_i in range(0, 4):
    #                     self.set_edge(chimera_i + grid_width,
    #                                   chimera_i + grid_width + (grid_width*8))

    #             # 4,5,6,7 to right-hand cell
    #             # Check that we are not one of most right-hand cells
    #             if j != grid_width - 1:
    #                 for chimera_j in range(4, 8):
    #                     self.set_edge(chimera_j + cell_start_offset,
    #                                   chimera_j + cell_start_offset + 8)

    #             cell_start_offset += 8
