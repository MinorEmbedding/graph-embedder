import dwave_networkx as dnx
import numpy as np
from chimera_graph import ChimeraGraph


class ChimeraGraphView:

    def __init__(self, n):
        """Inits a Chimera graph.

        Parameters
        ----------
        n : int
            Number of nodes
        """
        # --- Basic checks
        shore_size = 4
        if n > (2*shore_size):
            raise ValueError(
                f"""
                The number of nodes ({n}) is surpassing the number of nodes
                available in the Chimera graph ({2*shore_size})
                """
            )
        self.n = n

        # --- Init view graph
        # sum_{i=1}^{n-1}(i) = (n^2 - n) / 2
        length = int((n**2 - n) / 2)
        # adjacency array representing upper diagonal matrix
        self.adjacency_array = np.zeros(length, dtype=np.int)

        # --- Init Chimera graph
        self.graph = dnx.chimera_graph(1, 1, shore_size)
        self.chimera = ChimeraGraph(shore_size)

    def init_basic_graph(self):
        """Inits the graph as path graph starting from node 1 in the Chimera graph.
        The length is determined by `self.nodes_count.`

        Example
        -------
        For n=5 the adjacency matrix of the view graph looks like this
        after having called this method\\

        0 1 0 0 0 \\
        0 0 1 0 0 \\
        0 0 0 1 0 \\
        0 0 0 0 1 \\
        0 0 0 0 0
        """

        # --- Modify Chimera graph
        # Construct a simple path through the Chimera graph
        qubit_id = 0
        for i in range(self.n-1):
            next_qubit_id = self.chimera.get_random_neighbor_not_used(
                qubit_id)
            self.chimera.add_edge((qubit_id, next_qubit_id))
            qubit_id = next_qubit_id

        # --- Modify view graph
        # array positions for 1s are calculated as follows
        # i*n - sum_{k=1}^i(k) = i/2 * (2n-1-i)
        for i in range(self.n-1):
            index = int(i/2 * (2*self.n-1-i))
            self.adjacency_array[index] = 1

    def get_adjacency_matrix(self):
        matrix = np.zeros(self.n**2, dtype=np.int)

        i = 0
        j = 0

        for row in range(self.n):
            # Fill 0s in the beginning of the row
            for _ in range(row+1):
                matrix[i] = 0
                i += 1

            # Take rest from adjacency array
            for _ in range(self.n-(row+1)):
                matrix[i] = self.adjacency_array[j]
                i += 1
                j += 1

        # Reshape array to a matrix-like form
        return np.array(matrix).reshape((self.n, self.n))
