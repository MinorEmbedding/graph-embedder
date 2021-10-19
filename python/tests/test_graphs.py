import unittest
from unittest.case import TestCase

from src.graphs.chimera_graph import ChimeraGraph
from src.graphs.undirected_graph import UndirectedGraph


class Test_Graphs(TestCase):

    def test_sample_undirected_graph(self):
        # --- Arrange
        G = UndirectedGraph(6)
        G.add_edge(0, 4, 10)
        G.add_edge(0, 2, 20)
        G.add_edge(2, 1, 30)
        G.add_edge(1, 4, 40)
        G.add_edge(4, 3, 50)
        G.add_edge(5, 4, 60)

        # --- Act
        edges = G.get_edges()
        adj_matrix = G.get_adj_matrix()

        # --- Assert
        edges_assert = [(0, 2, 20), (0, 4, 10), (1, 2, 30), (1, 4, 40), (2, 0, 20),
                        (2, 1, 30), (3, 4, 50), (4, 0, 10), (4, 1, 40), (4, 3, 50),
                        (4, 5, 60), (5, 4, 60)]
        adjacency_matrix_assert = [[-1, -1, 20, -1, 10, -1],
                                   [-1, -1, 30, -1, 40, -1],
                                   [20, 30, -1, -1, -1, -1],
                                   [-1, -1, -1, -1, 50, -1],
                                   [10, 40, -1, 50, -1, 60],
                                   [-1, -1, -1, -1, 60, -1]]
        self.assertEqual(edges, edges_assert)
        self.assertEqual(adj_matrix, adjacency_matrix_assert)

    def test_chimera_graph(self):
        # --- Arrange
        G = ChimeraGraph()

        # --- Act
        edges = G.get_edges()
        adj_matrix = G.get_adj_matrix()

        # --- Assert
        edges_assert = [(0, 4, 0), (0, 5, 0), (0, 6, 0), (0, 7, 0),
                        (1, 4, 0), (1, 5, 0), (1, 6, 0), (1, 7, 0),
                        (2, 4, 0), (2, 5, 0), (2, 6, 0), (2, 7, 0),
                        (3, 4, 0), (3, 5, 0), (3, 6, 0), (3, 7, 0),
                        (4, 0, 0), (4, 1, 0), (4, 2, 0), (4, 3, 0),
                        (5, 0, 0), (5, 1, 0), (5, 2, 0), (5, 3, 0),
                        (6, 0, 0), (6, 1, 0), (6, 2, 0), (6, 3, 0),
                        (7, 0, 0), (7, 1, 0), (7, 2, 0), (7, 3, 0)]
        adj_matrix_assert = [[-1, -1, -1, -1, 0, 0, 0, 0],
                             [-1, -1, -1, -1, 0, 0, 0, 0],
                             [-1, -1, -1, -1, 0, 0, 0, 0],
                             [-1, -1, -1, -1, 0, 0, 0, 0],
                             [0, 0, 0, 0, -1, -1, -1, -1],
                             [0, 0, 0, 0, -1, -1, -1, -1],
                             [0, 0, 0, 0, -1, -1, -1, -1],
                             [0, 0, 0, 0, -1, -1, -1, -1]]
        self.assertEqual(edges, edges_assert)
        self.assertEqual(adj_matrix, adj_matrix_assert)


if __name__ == '__main__':
    unittest.main()
