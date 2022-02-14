from src.graph.undirected_graph import UndirectedGraphAdjList


class TestGraph():

    @staticmethod
    def house() -> UndirectedGraphAdjList:
        H = UndirectedGraphAdjList(5)
        H.set_edge(0, 1)
        H.set_edge(0, 4)
        H.set_edge(1, 2)
        H.set_edge(1, 3)
        H.set_edge(2, 3)
        H.set_edge(3, 4)
        return H

    @staticmethod
    def i_letter() -> UndirectedGraphAdjList:
        H = UndirectedGraphAdjList(6)
        H.set_edge(0, 3)
        H.set_edge(3, 2)
        H.set_edge(3, 5)
        H.set_edge(1, 2)
        H.set_edge(2, 4)
        return H

    @staticmethod
    def k4() -> UndirectedGraphAdjList:
        H = UndirectedGraphAdjList(4)
        H.set_edge(0, 1)
        H.set_edge(0, 2)
        H.set_edge(0, 3)
        H.set_edge(1, 2)
        H.set_edge(1, 3)
        H.set_edge(2, 3)
        return H

    @staticmethod
    def k5() -> UndirectedGraphAdjList:
        # can only be embedded using multiple chimera cells
        H = UndirectedGraphAdjList(5)
        H.set_edge(0, 1)
        H.set_edge(0, 2)
        H.set_edge(0, 3)
        H.set_edge(0, 4)
        H.set_edge(1, 2)
        H.set_edge(1, 3)
        H.set_edge(1, 4)
        H.set_edge(2, 3)
        H.set_edge(2, 4)
        H.set_edge(3, 4)
        return H

    @staticmethod
    def pyramid() -> UndirectedGraphAdjList:
        H = UndirectedGraphAdjList(5)
        H.set_edge(0, 1)
        H.set_edge(0, 2)
        H.set_edge(0, 3)
        H.set_edge(0, 4)
        H.set_edge(1, 2)
        H.set_edge(2, 3)
        H.set_edge(3, 4)
        return H

    @staticmethod
    def tree_like() -> UndirectedGraphAdjList:
        H = UndirectedGraphAdjList(6)
        H.set_edge(0, 1)
        H.set_edge(0, 2)
        H.set_edge(0, 3)
        H.set_edge(1, 4)
        H.set_edge(1, 5)
        H.set_edge(2, 5)  # comment this line out to get a tree
        return H
