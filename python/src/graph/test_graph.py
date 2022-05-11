import networkx as nx
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
    def k(n: int) -> UndirectedGraphAdjList:
        G = nx.complete_graph(n)
        H = UndirectedGraphAdjList(n)
        for e in G.edges:
            H.set_edge(e[0], e[1])
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

    @staticmethod
    def crossed_house() -> UndirectedGraphAdjList:
        # In Germany known as "Haus vom Nikolaus"
        H = TestGraph.k(4)
        new_node = H.add_node()
        H.set_edge(1, new_node)
        H.set_edge(2, new_node)
        return H
