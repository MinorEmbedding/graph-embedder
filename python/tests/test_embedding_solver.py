import unittest
from unittest.case import TestCase

from src.embedding_solver import EmbeddingSolver
from src.graphs.undirected_graphs import UndirectedGraphAdjList


class Test_EmbeddingSolver(TestCase):

    def test_init_basic_path(self):
        for i in range(0, 8):
            for j in range(10):
                H = UndirectedGraphAdjList(i)  # init minor
                if i < 2:
                    self.assertRaises(NameError, EmbeddingSolver, H)
                    continue
                solver = EmbeddingSolver(H)
                solver.init_basic_path()
                nodes, edges = solver.get_embedding()
                self.assertEqual(len(nodes), i)

    def test_cost_stay_the_same_for_same_inits(self):
        cost_fixed = -1

        for i in range(20):
            H = UndirectedGraphAdjList(5)
            H._set_edge(0, 1)
            H._set_edge(1, 2)
            H._set_edge(1, 3)
            H._set_edge(2, 3)
            H._set_edge(3, 4)
            H._set_edge(4, 0)

            solver = EmbeddingSolver(H)
            solver.init_basic_path()
            cost = solver.calculate_cost()

            if i == 0:
                cost_fixed = cost
            else:
                self.assertEqual(cost, cost_fixed)


if __name__ == '__main__':
    unittest.main()
