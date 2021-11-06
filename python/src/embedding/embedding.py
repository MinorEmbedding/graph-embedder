import itertools
from copy import deepcopy
from typing import List

from src.embedding.graph_mapping import GraphMapping
from src.graphs.chimera_graph import ChimeraGraphLayout, GraphEmbedding
from src.graphs.undirected_graphs import UndirectedGraphAdjList


########################### Embedding blueprint ################################
class Embedding():
    def __init__(self, H: UndirectedGraphAdjList):

        # TODO: explain difference (!) - crucial for understanding
        self.H = H
        self.G_layout = ChimeraGraphLayout()
        self.G_embedding = GraphEmbedding(8)
        self.G_embedding_view = GraphEmbedding(H.nodes_count)

        self.mapping = GraphMapping()

    def get_embedded_nodes(self) -> List:
        return self.G_embedding.get_embedded_nodes()

    def get_reachable_neighbors(self, from_node):
        return self.G_layout.get_neighbor_nodes(from_node)

    def get_free_neighbors(self, from_node) -> List:
        neighbors = self.G_layout.get_neighbor_nodes(from_node)
        neighbors_used = self.G_embedding.get_embedded_nodes()
        neighbors_free = [neighbor for neighbor in neighbors
                          if neighbor not in neighbors_used]
        if not neighbors_free:
            raise NoFreeNeighborNodes(from_node)
        return neighbors_free

    def get_connected_neighbors(self, from_node) -> List:
        return self.G_embedding.get_neighbor_nodes(from_node)

    def embed_edge(self, node_from, node_to) -> None:
        self.G_embedding.embed_edge(node_from, node_to)
        node_from_H = self.mapping.add_mapping_new_node_H(node_from)
        node_to_H = self.mapping.add_mapping_new_node_H(node_to)
        self.G_embedding_view.embed_edge(node_from_H, node_to_H)

    def remove_edge_inconsistently(self, from_node, to_node):
        self.G_embedding.remove_edge(from_node, to_node)

    def add_chain_to_used_nodes(self, from_node, to_node, to_node_new=None):
        """
        Adds a new chain. Does NOT check if this chain is even possible in the
        graph at the moment. TODO: "insource" the check to this class
        """
        # --- Adjust mapping
        # mapping for from_node stays the same
        from_node_H = self.mapping.get_node_H(node_G=from_node)
        # mapping for to_node is adjusted so that from_node_H maps to from_node AND to_node
        to_node_H = self.mapping.get_node_H(node_G=to_node)

        self.mapping.remove_mapping(to_node_H, to_node)
        self.mapping.extend_mapping(from_node_H, to_node)
        self.mapping.extend_mapping(to_node_H, to_node_new)

        # --- Adjust embedding
        # Delete all edges from node_to and add respective edges from node_to_new
        to_node_connected_neighbors = self.get_connected_neighbors(to_node)
        self.G_embedding.delete_all_edges_from_node(to_node)
        for prev_connected_neighbor in to_node_connected_neighbors:
            # Avoid edge from node to itself
            if prev_connected_neighbor in [from_node, to_node_new]:
                continue

            # to_node_new might be a chain itself
            nodes_in_chain = self.G_embedding.get_nodes_in_same_chain(
                to_node_new)
            embedded = False
            for node_in_chain in nodes_in_chain:
                if self.G_layout.exists_edge(prev_connected_neighbor, node_in_chain):
                    self.G_embedding.embed_edge(
                        prev_connected_neighbor, node_in_chain
                    )
                    embedded = True
                    break
            if not embedded:
                raise RuntimeError('Error adding a chain to used nodes')

        # --- Chain
        self.G_embedding.add_chain(from_node, to_node)
        # Add missing edge node_to---node_to_new
        self.G_embedding.embed_edge(to_node, to_node_new)

        # TODO: adjust embedding for self.G_embedding_small_view (???)

    def extend_one_node_to_chain(self, frm, to, extend_G):
        """Extends one node to a chain.

        Note that en edge from replace_G to frm must be viable. TODO: add a check
        """
        extend_H = self.mapping.get_node_H(node_G=extend_G)
        self.mapping.extend_mapping(extend_H, frm)
        self.mapping.extend_mapping(extend_H, to)
        chain = self.G_embedding.add_chain(frm, to)
        # avoid inconsistent state
        self.G_embedding.embed_edge(extend_G, frm, chain=chain)

    def is_valid_embedding(self) -> bool:
        for frm in self.H._get_nodes():
            expected_tos = self.H._get_neighbor_nodes(frm)
            actual_tos = self.G_embedding_view.get_neighbor_nodes(frm)
            if actual_tos != expected_tos:
                return False
        return True

    def get_playground(self):
        return deepcopy(self)

    def get_embedding(self):
        return self.G_embedding.get_embedding()

    def get_mapping_H_to_G(self):
        return self.mapping.get_mapping_H_to_G()

    def get_mapping_G_to_H(self):
        return self.mapping.get_mapping_G_to_H()

    def try_to_add_missing_edges(self) -> int:
        """
        Tries to add missing edges if possible.

        Returns
        -------
        How many missing edges were successfully added.
        """
        missing_edges_added = 0

        for frm in self.H._get_nodes():
            expected_tos = self.H._get_neighbor_nodes(frm)
            actual_tos = self.G_embedding_view.get_neighbor_nodes(frm)
            # actual_tos = [self.G_embedding_view.get_neighbor_nodes(frm)
            #               for frm in self.mapping.get_node_G(node_H=frm)]
            # actual_tos = list(itertools.chain(*actual_tos))  # flatten

            for to in expected_tos:
                if to not in actual_tos:
                    # print(f'missing edge from H: {frm}-{to}')
                    # Can we add this edge with the current embedding?
                    possible_edges = list(itertools.product(
                        self.mapping.get_node_G(node_H=frm), self.mapping.get_node_G(node_H=to)))
                    # product since we are dealing with possible chains

                    for possible_edge in possible_edges:
                        if self.G_layout.exists_edge(possible_edge[0], possible_edge[1]):
                            self.embed_edge(possible_edge[0], possible_edge[1])
                            print(f'added missing edge from H: {frm}-{to}')
                            missing_edges_added += 1
                            break  # successfully added missing edge

        return missing_edges_added


############################### Exceptions #####################################
class NoFreeNeighborNodes(Exception):
    def __init__(self, from_node):
        self.from_node = from_node

    def __str__(self):
        return f'NoFreeNeighborNodes from node: {self.from_node}'