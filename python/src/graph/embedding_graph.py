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

    def embed_edge(self, frm: int, to: int, chain=0) -> None:
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
        if not chain:
            chain = 0
        super().set_edge(frm, to, cost=chain)

    def get_embedded_edges(self) -> list[tuple[int, int, int]]:
        """Returns all embedded edges.

        Returns:
            list[tuple[int, int, int]]: A List of entries (frm, to, cost) \
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

    ############################### Chains #####################################

    def add_chain(self, node1: int, node2: int) -> int:
        """Adds a new chain between two nodes.

        Args:
            node1 (int): The first node of the chain.
            node2 (int): The second node of the chain.

        Returns:
            int: The new chain's identifier.
        """
        self._chain_last += 1
        self.embed_edge(node1, node2, chain=self._chain_last)
        return self._chain_last

    def get_nodes_in_same_chains(self, node: int) -> list[int]:
        """Returns the nodes that are in one of the chains the given node \
            is contained in.

        Warning:
            A node might be contained in multiple chains temporarily. Therefore,
            this method might return nodes that are not related to each other
            as they are located in different chains.

            Consider the functions :py:func:`get_node_chains` and
            :py:func:`get_chain_nodes` as alternatives.

        Args:
            node (int): The node from which to take the chain to compare against.

        Returns:
            list[int]: All nodes that are in one of the chains `node` \
                is contained in.
        """
        chains = self.get_node_chains(node)

        nodes = []
        for chain in chains:
            nodes.extend(self.get_chain_nodes(chain))

        return nodes

    def get_node_chains(self, node: int, include_default_chain=True) -> set[int]:
        """Returns all chains in which the current node is contained.

        A node might be contained in multiple chains. This would be an incorrect
        embedding in any case but allows for evolutionary algorithms to have
        temporarily invalid states which may be beneficial for some algorithms.

        Args:
            node (int):
                The node for which the chains are to be determined.
            include_default_chain (bool):
                Whether to include the default chain (0) in the results or not.
                Default to True.

        Returns:
            set[int]: All chains in which `node` is contained.
        """
        edges = super().get_neighbor_nodes_with_costs(node)
        chains = [edge[1] for edge in edges]
        chains = set(chains)
        if not include_default_chain:
            chains.discard(0)
        return chains

    def get_first_node_chain_other_than_default(self, node: int) -> Optional[int]:
        chains = self.get_node_chains(node, include_default_chain=False)
        return get_first_from(chains)

    def get_chain_nodes(self, chain: int) -> list[int]:
        """Returns all nodes that are contained in a given chain.

        Args:
            chain (int): The chain to retrieve all nodes for.

        Returns:
            list[int]: All nodes that are contained in `chain`. (These nodes \
                might be contained in other chains at the same time.) \
        """
        nodes = super().get_nodes()
        nodes = [node for node in nodes if chain in self.get_node_chains(node)]
        return nodes

    ############################### Embedding ##################################

    def get_embedding(self) -> tuple[list[int], list[tuple[int, int, int]]]:
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
