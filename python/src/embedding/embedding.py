import itertools
import logging
from copy import deepcopy

from src.embedding.graph_mapping import GraphMapping
from src.graph.chimera_graph import ChimeraGraphLayout
from src.graph.embedding_graph import EmbeddingGraph
from src.graph.undirected_graph import UndirectedGraphAdjList

logger = logging.getLogger('evolution')


class Embedding():
    def __init__(self, H: UndirectedGraphAdjList):
        """Initializes an Embedding.

        Args:
            H (UndirectedGraphAdjList): The minor Graph to embed
        """
        # --- Minor H
        self.H = H

        # --- Layout Graph
        # Graph to embed H onto
        self.G_layout = ChimeraGraphLayout(3, 3, 4)

        # --- Embedding Graph
        # Full graph: nodes are labeled according to G
        self.G_embedding = EmbeddingGraph(self.G_layout.get_size())
        # View graph: nodes are labeled according to H
        self.G_embedding_view = EmbeddingGraph(H.nodes_count)

        self.mapping = GraphMapping(H.nodes_count)

    def get_embedded_nodes(self) -> set[int]:
        """Returns all embedded nodes.

        Returns:
            set[int]: The embedded nodes.
        """
        return self.G_embedding.get_embedded_nodes()

    def get_reachable_neighbors(self, source):
        return self.G_layout.get_neighbor_nodes(source)

    def get_free_neighbors(self, source) -> set[int]:
        neighbors = self.G_layout.get_neighbor_nodes(source)
        neighbors_used = self.G_embedding.get_embedded_nodes()
        neighbors_free = {neighbor for neighbor in neighbors
                          if neighbor not in neighbors_used}
        if not neighbors_free:
            raise NoFreeNeighborNodes(source)
        return neighbors_free

    def get_embedded_neighbors(self, source) -> set[int]:
        return self.G_embedding.get_neighbor_nodes(source)

    def embed_edge(self, node1, node2) -> None:
        # Embed edge
        if not self.G_layout.exists_edge(node1, node2):
            raise NoViableEdge(node1, node2)
        self.G_embedding.embed_edge(node1, node2)

        # Adjust view graph
        supernode1 = self.mapping.get_supernode_create_if_not_available(node1)
        supernode2 = self.mapping.get_supernode_create_if_not_available(node2)
        if supernode1 != supernode2:  # avoid unnecessary selfloops
            self.G_embedding_view.embed_edge(supernode1, supernode2)

    # def embed_edge_without_mapping(self, node1, node2) -> None:
    #     # TODO: add warning on how to use this as this will leave inconsistent states
    #     # if not called correctly
    #     if not self.G_layout.exists_edge(node1, node2):
    #         raise NoViableEdge(node1, node2)
    #     self.G_embedding.embed_edge(node1, node2)

    def embed_edge_with_mapping(self, source_H, source_G, target_H, target_G) -> None:
        # TODO: add warning that this will overwrite the mapping (!)
        # not extend it
        self.G_embedding.embed_edge(source_G, target_G)
        self.mapping.set_mapping(source_H, source_G)
        self.mapping.set_mapping(target_H, target_G)
        self.G_embedding_view.embed_edge(source_H, target_H)

    def remove_edge_inconsistently(self, source, target):
        self.G_embedding.remove_edge(source, target)

    def exists_edge(self, source, target):
        return self.G_layout.exists_edge(source, target)

    def get_supernode(self, node_G) -> int:
        return self.mapping.get_node_H(node_G)

    def get_nodes_in_supernode(self, node_H) -> set[int]:
        return self.mapping.get_nodes_G(node_H)

    def get_nodes_in_supernode_of(self, node_G) -> set[int]:
        """Returns the nodes in the supernode that ``node_G`` is in.

        Note that ``node_G`` will be included in the resulting set.
        """
        supernode = self.get_supernode(node_G)
        return self.get_nodes_in_supernode(supernode)

    def construct_supernode(self, source: int, target: int) -> None:
        # TODO: add javadoc describing that it takes supernode of source as default,
        # but can be modified with optional param

        # Embed edge
        self.G_embedding.embed_edge(source, target)

        # Adjust mapping
        supernode = self.mapping.get_supernode_create_if_not_available(source)
        self.mapping.extend_mapping(supernode, target)

    def is_valid_embedding(self) -> bool:
        for source in self.H.get_nodes():
            expected_targets = self.H.get_neighbor_nodes(source)
            actual_targets = self.G_embedding_view.get_neighbor_nodes(source)
            if actual_targets != expected_targets:
                return False
        return True

    def get_playground(self):
        return deepcopy(self)

    def get_embedding(self, G_to_H_mapping=True):
        nodes, edges = self.G_embedding.get_embedding()
        if G_to_H_mapping:
            mapping = self.get_mapping_G_to_H()
        else:
            mapping = self.get_mapping_H_to_G()
        return nodes, edges, mapping

    def get_mapping_H_to_G(self):
        return self.mapping.get_mapping_H_to_G()

    def get_mapping_G_to_H(self):
        return self.mapping.get_mapping_G_to_H()

    def get_nodes_G(self, node_H: int) -> set[int]:
        return self.mapping.get_nodes_G(node_H)

    def try_embed_missing_edges(self) -> int:
        """Tries to embed missing edges if possible.

        Returns:
            int: How many missing edges were successfully added.
        """
        missing_edges_added = 0

        for source_H in self.H.get_nodes():
            expected_targets = self.H.get_neighbor_nodes(source_H)
            actual_targets = self.G_embedding_view.get_neighbor_nodes(source_H)

            for target_H in expected_targets:
                if target_H not in actual_targets:
                    # All possible combination (accounting for supernodes)
                    source_supernode_nodes = self.get_nodes_in_supernode(source_H)
                    target_supernode_nodes = self.get_nodes_in_supernode(target_H)
                    possible_edges = itertools.product(
                        source_supernode_nodes, target_supernode_nodes)

                    # Can we add missing edge with the current embedding?
                    for possible_edge in possible_edges:
                        try:
                            self.embed_edge(possible_edge[0], possible_edge[1])
                            logger.info(
                                f'➕ Added missing edge in H: {source_H}-{target_H}')
                            missing_edges_added += 1
                            break  # successfully added missing edge
                        except:
                            pass  # it's ok if we found no possible edge

        return missing_edges_added


############################### Exceptions #####################################
class NoFreeNeighborNodes(Exception):
    def __init__(self, source):
        self.source = source

    def __str__(self):
        return f'Node {self.source} has no free neighbors'


class NoViableEdge(Exception):
    def __init__(self, node1, node2):
        self.edge = (node1, node2)

    def __str__(self):
        return f'Not a valid edge: {self.edge[0]}-{self.edge[1]}'
