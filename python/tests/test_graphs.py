import unittest
from unittest.case import TestCase

from src.graphs.chimera_embedding import ChimeraEmbedding
from src.graphs.chimera_graph import ChimeraGraph
from src.graphs.undirected_graph import UndirectedGraph


class Test_Graphs(TestCase):

    def test_sample_undirected_graph(self):
        G = UndirectedGraph(6)
        G._set_edge(0, 4, 10)
        G._set_edge(0, 2, 20)
        G._set_edge(2, 1, 30)
        G._set_edge(1, 4, 40)
        G._set_edge(4, 3, 50)
        G._set_edge(5, 4, 60)

        edges = G._get_edges()
        adj_matrix = G._get_adj_matrix()

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
        G = ChimeraGraph()

        edges = G._get_edges()
        adj_matrix = G._get_adj_matrix()

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

    def test_chimera_embedding_edge_embeddings(self):
        G = ChimeraEmbedding()

        # --- Add edges
        try:
            G.add_edge_embedding(4, 0)
            G.add_edge_embedding(4, 1)
            G.add_edge_embedding(4, 2)
            G.add_edge_embedding(4, 3)

            G.add_edge_embedding(0, 4)
            G.add_edge_embedding(0, 5)
            G.add_edge_embedding(0, 6)
            G.add_edge_embedding(0, 7)
        except IndexError:
            self.fail('Did not expect an IndexError when adding')

        for i in range(4):
            self.assertRaises(IndexError, G.add_edge_embedding, 0, i)
            self.assertRaises(IndexError, G.add_edge_embedding, i, 0)
            self.assertRaises(IndexError, G.add_edge_embedding, 4, i+4)
            self.assertRaises(IndexError, G.add_edge_embedding, i+4, 4)

        # --- Remove edge
        try:
            G.remove_edge_embedding(4, 0)
            G.remove_edge_embedding(4, 1)
            G.remove_edge_embedding(4, 2)
            G.remove_edge_embedding(4, 3)

            # edge (0,4) go already already removed one line above
            G.remove_edge_embedding(0, 5)
            G.remove_edge_embedding(0, 6)
            G.remove_edge_embedding(7, 0)  # order should not matter
        except IndexError:
            self.fail('Did not expect an IndexError when removing')

        # Already removed those edge embeddings, so should result in error
        # due to symmetric adjacency matrix (undirected graph)
        self.assertRaises(IndexError, G.remove_edge_embedding, 4, 0)
        self.assertRaises(IndexError, G.remove_edge_embedding, 5, 0)
        self.assertRaises(IndexError, G.remove_edge_embedding, 6, 0)
        self.assertRaises(IndexError, G.remove_edge_embedding, 7, 0)

        # Edges that cannot possibly have an embedding
        for i in range(4):
            self.assertRaises(IndexError, G.remove_edge_embedding, 0, i)
            self.assertRaises(IndexError, G.remove_edge_embedding, i, 0)
            self.assertRaises(IndexError, G.remove_edge_embedding, 4, i+4)
            self.assertRaises(IndexError, G.remove_edge_embedding, i+4, 4)

        # Edges that could possibly have an embedding, but have not (anymore)
        self.assertRaises(IndexError, G.remove_edge_embedding, 1, 4)
        self.assertRaises(IndexError, G.remove_edge_embedding, 1, 5)
        self.assertRaises(IndexError, G.remove_edge_embedding, 1, 6)
        self.assertRaises(IndexError, G.remove_edge_embedding, 1, 7)


if __name__ == '__main__':
    unittest.main()
