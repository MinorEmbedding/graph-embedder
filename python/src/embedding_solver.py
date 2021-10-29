import copy
import itertools
import random
from typing import List

from src.graphs.chimera_graph import ChimeraGraphLayout, GraphEmbedding
from src.graphs.undirected_graphs import UndirectedGraphAdjList


class EmbeddingSolver():

    def __init__(self, H: UndirectedGraphAdjList):
        self._H = H
        if H.nodes_count < 2:
            raise NameError('The minor to embed must have at least two nodes')

        # TODO: explain difference (!) - crucial for understanding
        self._G_layout = ChimeraGraphLayout()
        self._G_embedding = GraphEmbedding(8)
        self._G_reduced_embedding = GraphEmbedding(H.nodes_count)

        # Mapping from nodes H to nodes in Chimera graph
        self.embedding = dict()
        for i in range(H.nodes_count):
            self.embedding[i] = set()

    def _get_free_neighbors(self, from_node) -> List:
        neighbors = self._G_layout.get_neighbor_nodes(from_node)
        neighbors_used = self._G_embedding.get_embedded_nodes()
        neighbors_free = [neighbor for neighbor in neighbors
                          if neighbor not in neighbors_used]
        if not neighbors_free:
            raise ValueError('No more free neighbor nodes found')
        return neighbors_free

    def init_basic_path(self) -> None:
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
         The length is determined by the number of vertices of the minor to embed.
         """
        # Init with path graph as long as H
        # TODO: what if no path graph embedding is possible? When is this the case?
        # Start at vertex 0
        node = 0
        for i in range(self._H.nodes_count-1):
            # Choose random neighbor
            neighbors = self._get_free_neighbors(node)
            next_node = random.choice(neighbors)

            # Embed
            self._G_embedding.embed_edge(node, next_node)
            self._G_reduced_embedding.embed_edge(i, i+1)

            # Mapping
            self.embedding[i].add(node)

            node = next_node

            # Map last node
            if i == self._H.nodes_count-2:
                self.embedding[i+1].add(node)

        print(self.embedding)

        self.try_to_add_missing_edges()

    def found_embedding(self) -> bool:
        for frm in self._H._get_nodes():
            expected_tos = self._H._get_neighbor_nodes(frm)
            actual_tos = self._G_reduced_embedding.get_neighbor_nodes(frm)
            if actual_tos != expected_tos:
                return False
        return True

    def try_to_add_missing_edges(self) -> int:
        missing_edges_added = 0

        for frm in self._H._get_nodes():
            expected_tos = self._H._get_neighbor_nodes(frm)
            actual_tos = self._G_reduced_embedding.get_neighbor_nodes(frm)

            for to in expected_tos:
                if to not in actual_tos:
                    # print(f'missing edge from H: {frm}-{to}')

                    # Can we add this edge with the current embedding?
                    possible_edges = itertools.product(
                        self.embedding[frm], self.embedding[to])
                    # product since we are dealing with possible chains
                    for possible_edge in possible_edges:
                        if self._G_layout.exists_edge(possible_edge[0], possible_edge[1]):
                            self._G_reduced_embedding.embed_edge(frm, to)
                            self._G_embedding.embed_edge(
                                possible_edge[0], possible_edge[1])
                            print(f'added missing edge from H: {frm}-{to}')
                            missing_edges_added += 1
                            break  # successfully added missing edge

        return missing_edges_added

    def try_to_add_missing_edges_special(self, G_embedding, embedding) -> int:
        missing_edges_added = 0

        for frm in self._H._get_nodes():
            expected_tos = self._H._get_neighbor_nodes(frm)
            actual_tos = [G_embedding.get_neighbor_nodes(
                frm) for frm in embedding[frm]]
            actual_tos = itertools.chain(*actual_tos)

            for to in expected_tos:
                if to not in actual_tos:
                    # print(f'missing edge from H: {frm}-{to}')
                    # Can we add this edge with the current embedding?
                    possible_edges = itertools.product(
                        embedding[frm], embedding[to])
                    # product since we are dealing with possible chains
                    for possible_edge in possible_edges:
                        if self._G_layout.exists_edge(possible_edge[0], possible_edge[1]):
                            G_embedding.embed_edge(
                                possible_edge[0], possible_edge[1])
                            # no need to add additional mappings here
                            # since no new or changed nodes
                            print(f'added missing edge from H: {frm}-{to}')
                            missing_edges_added += 1
                            break  # successfully added missing edge

        return missing_edges_added

    def get_embedding(self):
        return self._G_embedding.get_embedding()

    def calculate_cost(self):
        cost = 0

        print('--- COST calculation')
        for frm in self._H._get_nodes():
            expected_tos = self._H._get_neighbor_nodes(frm)
            # Premise: embedded graph has at any time the same number of nodes
            # as the minor graph H that we try to embed
            actual_tos = self._G_reduced_embedding.get_neighbor_nodes(frm)
            print(f'Node {frm}: {expected_tos} vs. {actual_tos}')

            if expected_tos == actual_tos:
                cost -= 10  # small reward

            for to in expected_tos:
                if to not in actual_tos:
                    cost += 20  # big punishment

            for to in actual_tos:
                if to not in expected_tos:
                    cost += 5  # small punishment since this edge is unnecessary

            # TODO: Punish Chains

        return cost

    def calculate_mutation_cost(self, mutation):
        return 0

    def dict_get_key_for_value(self, my_dict, value_to_find) -> object:
        for key, value in my_dict.items():
            if value_to_find in value:
                return key
        raise NameError(
            'Reverse dict key lookup did not work (should never happen)')

    def mutate(self):
        """
        Mutates the embedding.
        """
        print('MUTATION')
        # --- Delete & Insert edge
        # Delete an edge between two random nodes that were already embedded.
        # Insert a new edge between two other random nodes that were already embedded.

        # # --- Add random chain
        G_embedding_playground = copy.deepcopy(self._G_embedding)
        G_reduced_embedding_playground = copy.deepcopy(
            self._G_reduced_embedding)
        embedding_playground = copy.deepcopy(self.embedding)

        # --- Randomly collapse two nodes to one
        embedded_nodes = self._G_embedding.get_embedded_nodes()
        node_from = random.choice(embedded_nodes)
        node_tos = self._G_embedding.get_neighbor_nodes(node_from)
        node_to = random.choice(node_tos)
        print(f'Trying to chain nodes {node_from} and {node_to}')

        # --- Adjust so that new chain fits
        # find new place for previous node_to in the graph
        # so that the layout permits the following edges
        # node_to   --- node_to'
        # node_to'  --- all_nodes reachable from node_to
        free_neighbors_from_node_to = self._get_free_neighbors(node_to)
        if not free_neighbors_from_node_to:
            print('No free neighbors left over for a new chain')
            return False
        neighbors_of_node_to = self._G_embedding.get_neighbor_nodes(
            node_to)

        # TODO: dont take nodes that are already in a chain

        # 1. Try out all possible positions for node_to'
        for node_to_prime in free_neighbors_from_node_to:
            print(f"node_to': {node_to_prime}")

            # From node_to_prime: Can we reach all nodes previously connected to node_to?
            reachable_nodes_from_node_to_prime = self._G_layout.get_neighbor_nodes(
                node_to_prime)
            can_reach = [neighbor in reachable_nodes_from_node_to_prime
                         for neighbor in neighbors_of_node_to]
            if all(can_reach):
                # try out on "playground"
                G_embedding_playground.add_chain(node_from, node_to)

                embedding_playground[node_from].add(node_to)
                embedding_playground[node_to] = set(node_to_prime)
                self.try_to_add_missing_edges_special(
                    G_embedding_playground, embedding_playground)
                return True

        # 2. If step 1 does not work, try to construct a new chain
        node_to_prime = free_neighbors_from_node_to[0]
        # TODO: use already calculated result from above
        free_neighbors_node_to_prime = self._get_free_neighbors(node_to_prime)
        if not free_neighbors_node_to_prime:
            print('No free neighbors for subsequent chain left over')
            return False
        # just use the first free spot
        node_to_prime_chain_partner = free_neighbors_node_to_prime[0]

        # From node_to_prime and node_to_prime_chain_partner:
        # Can we now reach all nodes previously connected to node_to?
        reachable_nodes_from_node_to_prime_chain = self._G_layout.get_neighbor_nodes(
            node_to_prime)
        reachable_nodes_from_node_to_prime_chain.extend(
            self._G_layout.get_neighbor_nodes(node_to_prime_chain_partner))
        can_reach = [neighbor in reachable_nodes_from_node_to_prime_chain
                     for neighbor in neighbors_of_node_to]
        if all(can_reach):
            # try out on "playground"
            G_embedding_playground.add_chain(node_from, node_to)
            embedding_playground[self.dict_get_key_for_value(embedding_playground,
                                                             node_from)] = set([node_from, node_to])
            print(
                f'node_to_prime to chain partner: {node_to_prime}---{node_to_prime_chain_partner}')
            G_embedding_playground.add_chain(
                node_to_prime, node_to_prime_chain_partner)
            embedding_playground[self.dict_get_key_for_value(embedding_playground, node_to)] = set(
                [node_to_prime, node_to_prime_chain_partner])
            self.try_to_add_missing_edges_special(
                G_embedding_playground, embedding_playground)
            print(embedding_playground)
            return G_embedding_playground

        # 3. If all of that fails, mark the mutation as failed
        return False

        # TODO: Remove chain mutation

        # --- Perspective change
        # TODO: with low probability: view reduced graph from completely different view
        # maybe from this perspective, we can leverage some better mutations
        # and reduce the costs faster
        pass


class Mutation():
    def __init__(self):
        pass
