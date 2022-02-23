from typing import Optional

from src.graph.undirected_graph import UndirectedGraphAdjList
from src.util.util import get_first_from


class EmbeddingGraph(UndirectedGraphAdjList):
    """A Graph to **form** an embedding (using an adjacency list).

    This Graph is supposed to **converge into a valid minor** of another graph
    by using the methods it provides, e.g. to embed edges, form chains etc.

    Hint:
        Pay attention whether you use methods which are directly defined here
        or in the parent class
        :py:class:`src.graph.undirected_graph.UndirectedGraphAdjList`.
        Methods defined over there may not be convenient as the current class
        tries to hide implementation details, e.g. chains being encoded as
        edge costs. It is recommend to just stick to the methods defined here
        (in the subclass).

    Warning:
        This class does not implement any checks for chains, so it is possible
        to assign a node to multiple chains.
    """

    def __init__(self, nodes_count):
        self._chain_last = 0
        super().__init__(nodes_count)

    ############################### Edges ######################################

    def embed_edge(self, frm: int, to: int) -> None:
        """Embeds an edge (and optionally assigns a chain).

        Args:
            frm (int): One node of the edge.
            to (int): The other node of the edge.
            chain (int, optional): The chain to assign to the new embedded edge. \
                The default value `0` means: no chain, just a "normal" edge.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` or ``to`` \
            does not exist in the Graph.
        """
        super().set_edge(frm, to, cost=0)

    def get_embedded_edges(self) -> set[tuple[int, int, int]]:
        """Returns all embedded edges.

        Returns:
            set[tuple[int, int, int]]: A set of entries (frm, to, cost) \
                describing all embedded edges of this Graph with their \
                respective costs.
        """
        return super().get_edges()

    ################################# Nodes ####################################

    def get_embedded_nodes(self) -> set[int]:
        """Returns all embedded nodes.

        Returns:
            set[int]: The embedded nodes.
        """
        nodes = super().get_nodes()
        nodes = {node for node in nodes if self.get_neighbor_nodes(node)}
        return nodes

    ############################### Embedding ##################################

    def get_embedding(self) -> tuple[set[int], set[tuple[int, int, int]]]:
        """Returns the current embedding.

        Returns:
            tuple[list[int], list[tuple[int, int, int]]]: \
                A tuple of nodes and edges. `nodes` is a list of integers, \
                while `edges` is a list of tuples (frm, to, chain) describing \
                the edges (frm node, to node, chain)
        """
        nodes = self.get_embedded_nodes()
        edges = self.get_embedded_edges()
        return nodes, edges
