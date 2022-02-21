import itertools
import logging
from copy import deepcopy

from src.embedding.graph_mapping import GraphMapping
from src.graph.chimera_graph import ChimeraGraphLayout
from src.graph.embedding_graph import EmbeddingGraph
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.util.util import get_first_from_set

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
        # Full graph
        self.G_embedding = EmbeddingGraph(self.G_layout.get_size())
        # View graph
        self.G_embedding_view = EmbeddingGraph(H.nodes_count)

        self.mapping = GraphMapping()

    def get_embedded_nodes(self) -> list[int]:
        """Returns all embedded nodes.

        Returns:
            list[int]: The embedded nodes.
        """
        return self.G_embedding.get_embedded_nodes()

    def get_embedded_nodes_not_in_chain(self) -> list[int]:
        res = []
        for node in self.G_embedding.get_embedded_nodes():
            if self.G_embedding.get_node_chains(node) == {0}:
                res.append(node)
        return res

    def get_reachable_neighbors(self, source):
        return self.G_layout.get_neighbor_nodes(source)

    def get_free_neighbors(self, source) -> list:
        neighbors = self.G_layout.get_neighbor_nodes(source)
        neighbors_used = self.G_embedding.get_embedded_nodes()
        neighbors_free = [neighbor for neighbor in neighbors
                          if neighbor not in neighbors_used]
        if not neighbors_free:
            raise NoFreeNeighborNodes(source)
        return neighbors_free

    def get_embedded_neighbors(self, source) -> list:
        return self.G_embedding.get_neighbor_nodes(source)

    def embed_edge(self, source, target) -> None:
        self.G_embedding.embed_edge(source, target)
        node_from_H = self.mapping.add_mapping_new_node_H(source)
        node_to_H = self.mapping.add_mapping_new_node_H(target)
        self.G_embedding_view.embed_edge(node_from_H, node_to_H)

    def embed_edge_with_mapping(self, source_H, source_G, target_H, target_G) -> None:
        self.G_embedding.embed_edge(source_G, target_G)
        self.mapping.set_mapping(source_H, source_G)
        self.mapping.set_mapping(target_H, target_G)
        self.G_embedding_view.embed_edge(source_H, target_H)

    def remove_edge_inconsistently(self, source, target):
        self.G_embedding.remove_edge(source, target)

    def exists_edge(self, source, target):
        return self.G_layout.exists_edge(source, target)

    def add_chain_to_used_nodes(self, source, target, shifted_target, shifted_target_partner=None):
        """
        Adds a new chain. Does NOT check if this chain is even possible in the
        graph at the moment. TODO: "insource" the check to this class
        """
        # --- Adjust mapping
        # mapping for source stays the same
        from_node_H = self.mapping.get_node_H(node_G=source)
        # mapping for target is adjusted so that from_node_H maps to source AND target
        to_node_H = self.mapping.get_node_H(node_G=target)

        # TODO: simplify to set_mapping(target, from_node_H) # other-way around
        self.mapping.remove_mapping(to_node_H, target)
        self.mapping.extend_mapping(from_node_H, target)
        self.mapping.extend_mapping(to_node_H, shifted_target)

        # --- Adjust embedding
        to_node_connected_neighbors = self.get_embedded_neighbors(target)

        # Special case: target might have been in chain
        to_node_chain = self.G_embedding.get_first_node_chain_other_than_default(
            target)
        to_node_chain_nodes = self.G_embedding.get_chain_nodes(
            to_node_chain) if to_node_chain else []

        # Two chains constructed?
        new_nodes = [shifted_target]
        if shifted_target_partner:
            new_nodes.append(shifted_target_partner)

        # Delete all edges outgoing from target
        self.G_embedding.remove_all_edges_from_node(target)

        for prev_connected_neighbor in to_node_connected_neighbors:
            print('🌹 here for the next prev_connected_neigbhor: ',
                  prev_connected_neighbor)

            # Special previous connected neighbors
            if prev_connected_neighbor in [source, shifted_target]:
                continue

            # Embed all other pre-existing edges
            # target might have been in a chain with the current neighbor
            # -> if in super-node
            chain = to_node_chain if prev_connected_neighbor in to_node_chain_nodes else 0
            embedded = False
            for node_in_chain in new_nodes:
                # No direct edge to neighbor needed, just to the whole super-node,
                # thus to any node in the chain # TODO: method to calculate any connection to super-node
                neighbors = self.get_reachable_neighbors(node_in_chain)
                neighbors_chain = [self.G_embedding.get_first_node_chain_other_than_default(
                    node) for node in neighbors]
                print(f'Node: {node_in_chain} (chain: {chain})')
                print(f'Reachable neighbors: {neighbors}')
                print(f'Reachable neighbors chain: {neighbors_chain}')

                if chain != 0:
                    for i, reachable_neighbor in enumerate(neighbors):
                        print(
                            f'Reachable neighbor {reachable_neighbor} is in chain {neighbors_chain[i]}')
                        # Might be a chain, so give another chance
                        if chain == neighbors_chain[i]:
                            print(
                                f'Check exists edge {reachable_neighbor}, {node_in_chain}')
                            if self.G_layout.exists_edge(reachable_neighbor, node_in_chain):
                                self.G_embedding.embed_edge(
                                    reachable_neighbor, node_in_chain, chain=chain)
                                embedded = True
                                break
                else:
                    # Directly
                    if self.G_layout.exists_edge(prev_connected_neighbor, node_in_chain):
                        # TODO: outsource exists_edge check to embed_edge method
                        # and do a simple try catch here
                        self.G_embedding.embed_edge(
                            prev_connected_neighbor, node_in_chain, chain=chain)
                        embedded = True
                        break

                    # or prev connected node itself in a chain
                    prev_connected_neighbor_chain = self.G_embedding.get_first_node_chain_other_than_default(
                        prev_connected_neighbor)
                    for i, reachable_neighbor in enumerate(neighbors):
                        if prev_connected_neighbor_chain == neighbors_chain[i]:
                            if self.G_layout.exists_edge(reachable_neighbor, node_in_chain):
                                self.G_embedding.embed_edge(
                                    reachable_neighbor, node_in_chain, chain=chain)
                                embedded = True
                                break

            if not embedded:
                print('FATAL ERROR here, but going on')
                raise RuntimeError(
                    'Error adding a chain to used nodes. Should never happen!')

        # --- Chain
        # If source has been in a chain before, we extend this chain
        source_chain = self.G_embedding.get_first_node_chain_other_than_default(
            source)
        if source_chain:
            self.G_embedding.embed_edge(source, target, chain=source_chain)
        else:
            self.G_embedding.add_chain(source, target)

        self.G_embedding.embed_edge(target, shifted_target)
        # TODO: adjust embedding for self.G_embedding_view (???)

    def extend_one_node_to_chain(self, source, target, extend_G):
        """Extends one node to a chain.

        Note that an edge from extend_G to frm must be viable. TODO: add a check
        """
        extend_H = self.mapping.get_node_H(node_G=extend_G)
        self.mapping.extend_mapping(extend_H, source)
        self.mapping.extend_mapping(extend_H, target)

        # The target might be a chain, so we need to choose the right chain then
        extend_G_node_chain = self.G_embedding.get_first_node_chain_other_than_default(
            extend_G)
        logger.info(f'🎂 Extend_G node chain: {extend_G_node_chain}')
        if extend_G_node_chain:
            logger.info('👀 (1) Reuse existing chain')
            # TODO: What about a node being included in multiple chains temporarily?
            # Right now, this is never the case, or is it?
            self.G_embedding.embed_edge(
                source, target, chain=extend_G_node_chain)
        else:
            logger.info('👀 (2) Add chain')
            self.G_embedding.add_chain(source, target)
        # avoid inconsistent state
        # TODO: right now not really needed as chain gets added later
        self.G_embedding.embed_edge(extend_G, source)

    def is_valid_embedding(self) -> bool:
        for frm in self.H.get_nodes():
            expected_targets = self.H.get_neighbor_nodes(frm)
            actual_targets = self.G_embedding_view.get_neighbor_nodes(frm)
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

    def get_mapping_H_to_G_node(self, node_H) -> set:
        try:
            return self.mapping.get_mapping_H_to_G()[node_H]
        except KeyError:
            return set()

    def try_to_add_missing_edges(self) -> int:
        """
        Tries to add missing edges if possible.

        Returns
        -------
        How many missing edges were successfully added.
        """
        missing_edges_added = 0

        for source in self.H.get_nodes():
            expected_targets = self.H.get_neighbor_nodes(source)
            actual_targets = self.G_embedding_view.get_neighbor_nodes(source)
            # actual_targets = [self.G_embedding_view.get_neighbor_nodes(frm)
            #               for frm in self.mapping.get_node_G(node_H=frm)]
            # actual_targets = list(itertools.chain(*actual_targets))  # flatten

            for target in expected_targets:
                if target not in actual_targets:
                    # logger.info(f'missing edge from H: {source}-{target}')
                    # Can we add this edge with the current embedding?
                    possible_edges = list(itertools.product(
                        self.mapping.get_node_G(node_H=source), self.mapping.get_node_G(node_H=target)))
                    # product since we are dealing with possible chains

                    for possible_edge in possible_edges:
                        if self.G_layout.exists_edge(possible_edge[0], possible_edge[1]):
                            self.embed_edge(possible_edge[0], possible_edge[1])
                            logger.info(
                                f'added missing edge from H: {source}-{target}')
                            missing_edges_added += 1
                            break  # successfully added missing edge

        return missing_edges_added


############################### Exceptions #####################################
class NoFreeNeighborNodes(Exception):
    def __init__(self, source):
        self.source = source

    def __str__(self):
        return f'NoFreeNeighborNodes from node: {self.source}'
