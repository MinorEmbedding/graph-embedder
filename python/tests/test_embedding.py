import unittest
from unittest.case import TestCase

from src.embedding_solver import EmbeddingSolver
from src.graphs.undirected_graphs import UndirectedGraphAdjList


class Test_Embedding(TestCase):

    def test_init_basic_path(self):
        for i in range(0, 8):
            for j in range(10):
                H = UndirectedGraphAdjList(i)  # init minor
                if i < 2:
                    self.assertRaises(NameError, EmbeddingSolver, H)
                    continue
                solver = EmbeddingSolver(H)
                solver.init_basic_path()
                nodes, edges = solver.get_current_embedding()
                self.assertEquals(len(nodes), i)


if __name__ == '__main__':
    unittest.main()
