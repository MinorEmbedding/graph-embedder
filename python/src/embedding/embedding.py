import itertools
import logging
from copy import deepcopy

from src.embedding.graph_mapping import GraphMapping
from src.graph.chimera_graph import ChimeraGraphLayout
from src.graph.embedding_graph import EmbeddingGraph
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.embedding.articulation_point import ArticulationPointCalculator

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
        self.G_layout = ChimeraGraphLayout(5, 5, 4)

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
        embedded_nodes = self.G_embedding.get_embedded_nodes()
        if not embedded_nodes:
            raise Exception('No nodes embedded yet')
        return embedded_nodes

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
        # Embed edge
        self.G_embedding.embed_edge(source, target)

        # Remove target from previous chain
        self.try_remove_from_supernode(target)

        # Add to new chain
        source_supernode = self.mapping.get_supernode_create_if_not_available(source)
        self.mapping.extend_mapping(source_supernode, target)

    def try_remove_from_supernode(self, node: int):
        """Removes node from its supernode, leaving us with an inconsistent state.

        If ``node`` was in no supernode, we silently do nothing.
        """
        try:
            supernode = self.get_supernode(node)
            self.mapping.remove_mapping(supernode, node)
        except KeyError:
            pass

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
        missing_edges_added = set()

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
                            # logger.info(
                            #     f'➕ Added missing edge in H: {source_H}-{target_H}')
                            missing_edges_added.add((source_H, target_H))
                            break  # successfully added missing edge
                        except:
                            pass  # it's ok if we found no possible edge

        # Log
        logger.info('Try to add missing edges')
        for edge in missing_edges_added:
            logger.info(f'➕ Added missing edge in H: {edge[0]}-{edge[1]}')

        return len(missing_edges_added)

    def remove_unnecessary_edges_between_supernodes(self) -> None:
        """Tries to remove unnecessary edges, e.g. multiple edges between
        two supernodes.
        """
        # For every supernode (chain)
        for supernode in self.H.get_nodes():
            considered_supernodes = []

            # For every node in supernode (chain)
            for node in self.get_nodes_in_supernode(supernode):

                # Go through every edge to a neighbor
                # that is in ANOTHER supernode
                for neighbor in self.get_embedded_neighbors(node):
                    neighbor_supernode = self.get_supernode(neighbor)

                    # Do not consider edges inside of supernodes
                    if supernode == neighbor_supernode:
                        continue

                    # Only keep the edge if we didn't have any connection
                    # to neighbor's supernode yet
                    if neighbor_supernode not in considered_supernodes:
                        considered_supernodes.append(neighbor_supernode)
                    else:
                        self.G_embedding.remove_edge(node, neighbor)
                        # No need adjust G_embedding_view as
                        # this must be preserved by this method

    def remove_redundant_supernode_nodes(self):
        for supernode in self.H.get_nodes():
            self.remove_redundant_nodes_in_supernode(supernode)

    def remove_redundant_nodes_in_supernode(self, supernode) -> None:
        supernode_nodes = self.get_nodes_in_supernode(supernode)

        if len(supernode_nodes) == 1:
            return

        removed_nodes = set()

        while True:
            removed_in_this_iteration = False

            # Calculate articulation points
            # We need to recalculate them every time a node was removed
            # since the articulation points might change in this case
            articulation_points = ArticulationPointCalculator(self.G_embedding)\
                .calc_articulation_points(supernode_nodes - removed_nodes)
            logger.info(f'supernode {supernode} (nodes: {supernode_nodes}) '
                        + f'has articulation points: {articulation_points}')

            # Try to remove ONE node in the supernode
            for node_to_remove in supernode_nodes:
                if node_to_remove in removed_nodes:
                    continue

                # Don't remove articulation points (aka "cut nodes")
                if node_to_remove in articulation_points:
                    continue

                # Can we reach all previous neighbors of supernode?
                node_to_remove_neighbors = self.get_embedded_neighbors(node_to_remove)
                if node_to_remove_neighbors:
                    rest_nodes_reachable = [self.get_embedded_neighbors(node)
                                            for node in supernode_nodes
                                            if node != node_to_remove]
                    rest_nodes_reachable = itertools.chain(*rest_nodes_reachable)

                    if not all([neighbor in rest_nodes_reachable
                                for neighbor in node_to_remove_neighbors
                                # chain connectivity already ensured
                                if neighbor not in supernode_nodes]):
                        continue

                # We can now safely remove the node
                self.remove_node(node_to_remove)
                removed_in_this_iteration = True
                removed_nodes.add(node_to_remove)
                logger.info(f'✂ Removed node: {node_to_remove}')

                # Leave at least one node left of every supernode
                if len(removed_nodes) == (len(supernode_nodes) - 1):
                    return

                break

            if not removed_in_this_iteration:
                return

    def remove_node(self, node: int) -> None:
        # Embedding
        self.G_embedding.remove_node(node)

        # Mapping
        supernode = self.get_supernode(node)
        self.mapping.remove_mapping(supernode, node)

    def check_supernode_connectiveness(self, supernode: int) -> bool:
        """Checks that no supernodes are split up into multiple groups
        by the mutation. All nodes in a supernode must have an edge to at least
        one other supernode.
        """
        supernode_nodes = self.get_nodes_in_supernode(supernode)
        for node in supernode_nodes:
            embedded_neighbors = self.get_embedded_neighbors(node)

            reached = False
            for neighbor in embedded_neighbors:
                neighbor_supernode = self.get_supernode(neighbor)
                if neighbor_supernode == supernode:
                    reached = True

            if not reached:
                return False

        return True


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
