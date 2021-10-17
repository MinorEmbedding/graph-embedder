import random
from typing import List, Tuple

import numpy as np


class ChimeraGraph:
    #            Chimera graph            Graph grid structure
    #                  4
    #                                         x x x x ——4
    #                  5                      x x x x ——5
    #            0   1   2   3                x x x x ——6
    #                  6                      x x x x ——7
    #                                         | | | |
    #                  7                      0 1 2 3
    #
    # as array: index vertical row first, then horizontal columns (!)
    # # [[x x x x], [x x x x], ...]

    def __init__(self, shore_size):
        self.shore_size = shore_size

        self.qubits_horizontal = [i for i in range(shore_size)]
        self.qubits_vertical = [i+shore_size for i in range(shore_size)]

        self._grid = np.zeros((shore_size, shore_size), dtype=np.int)
        self.edges = set()
        self.nodes = set()

    def get_qubit(self, qubit_id) -> List:
        if qubit_id in self.qubits_horizontal:
            return self._grid[:, qubit_id]  # column
        return self._grid[qubit_id-self.shore_size]  # row

    def get_qubit_neighbors(self, qubit_id) -> List:
        if qubit_id in self.qubits_horizontal:
            return self.qubits_vertical
        return self.qubits_horizontal

    def get_qubit_with_neighbors(self, qubit_id) -> Tuple:
        qubit = self.get_qubit(qubit_id)
        neighbors = self.get_qubit_neighbors(qubit_id)
        return tuple(zip(neighbors, qubit))

    def get_random_neighbor_not_used(self, qubit_id) -> int:
        # --- Get possible qubits
        qubit = self.get_qubit_with_neighbors(qubit_id)
        # Filter out those that already have an edge
        qubit = list(filter(lambda e: e[1] == 0, qubit))
        if not qubit:
            raise RuntimeError("Edges from this qubit are all used")

        # --- Choose next qubit id randomly
        next = random.choice(qubit)[0]
        # print((qubit_id, next))
        return next

    def add_edge(self, edge: Tuple) -> None:
        if edge[0] in self.qubits_horizontal:
            qubit_id_row = edge[1]-self.shore_size
            qubit_id_col = edge[0]
        else:
            qubit_id_row = edge[0]-self.shore_size
            qubit_id_col = edge[1]

        self._grid[qubit_id_row, qubit_id_col] = 1
        self.edges.add((qubit_id_col, qubit_id_row+self.shore_size))
        self.nodes.add(qubit_id_col)
        self.nodes.add(qubit_id_row+self.shore_size)

    def get_nodes(self) -> List:
        return list(self.nodes)

    def get_edges(self) -> List:
        return list(self.edges)

    # def get_edges(self) -> List:
    #     edges = []
    #     for i in range(self.shore_size):
    #         for j in range(self.shore_size):
    #             edge = self._grid[i, j]
    #             if edge != 0:
    #                 edges.append((i, j+self.shore_size))
    #     print(self._grid)
    #     return edges

    # def get_nodes(self) -> List:
    #     nodes = set()
    #     for j in range(self.shore_size):
    #         for i in range(self.shore_size):
    #             edge = self._grid[i, j]
    #             if edge != 0:
    #                 nodes.add(i)
    #                 nodes.add(j+self.shore_size)
    #     return list(nodes)
