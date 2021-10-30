import random

from src.embedding.embedding import Embedding
from src.graphs.undirected_graphs import UndirectedGraphAdjList


class EmbeddingSolver():

    def __init__(self, H: UndirectedGraphAdjList):
        self.H = H
        if H.nodes_count < 2:
            raise NameError('The minor to embed must have at least two nodes')

        self.embedding = Embedding(H)

    def init_basic_path(self) -> None:
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
        The length is determined by the number of vertices of the minor to embed.
        """
        # Init with path graph as long as H
        # TODO: what if no path graph embedding is possible? When is this the case?
        # Start at vertex 0
        node_from = 0
        for _ in range(self.H.nodes_count-1):
            # Choose random neighbor
            neighbors = self.embedding.get_free_neighbors(node_from)
            node_to = random.choice(neighbors)

            # Embed
            self.embedding.embed_edge(node_from, node_to)

            node_from = node_to

        self.embedding.try_to_add_missing_edges()

    def get_embedding(self):
        return self.embedding.get_embedding()

    def _add_random_chain(self):
        # --- Randomly collapse two nodes to one
        nodes_embedded = self.embedding.get_embedded_nodes()
        # TODO: make sure we don't use nodes that are already in a chain
        from_node = random.choice(nodes_embedded)
        node_tos = self.embedding.get_connected_neighbors(from_node)
        to_node = random.choice(node_tos)
        print(f'Trying to chain nodes {from_node} and {to_node}')

        # --- Adjust so that new chain is viable
        # find new place for previous node_to in the graph
        # so that the layout permits the following edges
        # node_to   --- node_to'
        # node_to'  --- all_nodes reachable from node_to
        to_node_free_neighbors = self.embedding.get_free_neighbors(to_node)
        to_node_connected_neighbors = self.embedding.get_connected_neighbors(
            to_node)

        # --- 1) Try out all possible positions for node_to_new
        for to_node_new in to_node_free_neighbors:
            print(f'node_to_new: {to_node_new}')

            # from node_to_new: can we reach all nodes previously connected to node_to?
            node_to_new_reachable_neighbors = self.embedding.get_reachable_neighbors(
                to_node_new)
            can_reach = [neighbor in node_to_new_reachable_neighbors
                         for neighbor in to_node_connected_neighbors if neighbor != from_node]
            if all(can_reach):
                # Try out on playground
                playground = self.embedding.get_playground()
                playground.add_chain_to_used_nodes(
                    from_node, to_node, to_node_new)
                playground.try_to_add_missing_edges()
                return playground

        # --- 2) If step 1) did not work, try to construct another new chain (-> two chains in total)
        to_node_new = to_node_free_neighbors[0]
        to_node_new_free_neighbors = self.embedding.get_free_neighbors(
            to_node_new)
        to_node_new_chain_partner = to_node_new_free_neighbors[0]

        # from to_node_new AND to_node_new_chain_partner:
        # Can we now reach all nodes previously connected to node_to
        chain_reachable_nodes = self.embedding.get_reachable_neighbors(
            to_node_new)
        chain_reachable_nodes.extend(
            self.embedding.get_reachable_neighbors(to_node_new_chain_partner))
        can_reach = [neighbor in chain_reachable_nodes
                     for neighbor in to_node_connected_neighbors if neighbor != from_node]
        if all(can_reach):
            # Try out on playground
            playground = self.embedding.get_playground()

            playground.extend_one_node_to_chain(
                to_node_new, to_node_new_chain_partner, extend_G=to_node)

            # edge to_node---to_node_new
            # will be removed when adding this chain:
            playground.add_chain_to_used_nodes(
                from_node, to_node, to_node_new)

            print(
                f'to_node_new to chain partner: {to_node_new}---{to_node_new_chain_partner}')
            playground.try_to_add_missing_edges()
            return playground

        # 3. If all of that fails, mark the mutation as failed
        return None

    def mutate(self):
        """
        Mutates the embedding. Supports adding random chains rights now.
        """
        print('--- MUTATION')
        # --- Delete & Insert edge
        # Delete an edge between two random nodes that were already embedded.
        # Insert a new edge between two other random nodes that were already embedded.

        # Add random chain
        return self._add_random_chain()

        # TODO: Remove chain mutation

        # --- Perspective change
        # TODO: with low probability: view reduced graph from completely different view
        # maybe from this perspective, we can leverage some better mutations
        # and reduce the costs faster

    def found_embedding(self) -> bool:
        return self.embedding.is_valid_embedding()
