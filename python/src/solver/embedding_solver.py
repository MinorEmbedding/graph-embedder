import logging
import random

from src.embedding.embedding import Embedding
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.util.stack import Stack
from src.util.util import get_first_from_set

logger = logging.getLogger('evolution')


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

    def init_bfs(self):
        """Traverses through H and inits graph H usugin breadth first search on H."""
        # Mark all vertices as not visited
        visited = [False] * self.H.nodes_count

        queue = []

        # Mark source node as visited
        h = 0  # start with node 0 in graph H
        queue.append(h)
        visited[h] = True

        while queue:
            # Dequeue node & get neighbors
            h = queue.pop(0)
            neighbors_h = self.H.get_neighbor_nodes(h)
            # Filter out already visited nodes in H
            neighbors_h = [h for h in neighbors_h
                           if not visited[h]]

            # Embed edges to all neighbors
            if h == 0:
                g = 0  # start with node 0 for embedding in graph G
            else:
                gs = self.embedding.get_mapping_H_to_G_node(h)
                g = get_first_from_set(gs)

            free_neighbors_g = self.embedding.get_free_neighbors(g)
            if len(free_neighbors_g) < len(neighbors_h):
                raise RuntimeError(
                    'Not enough free neighbors to embed node of H in G (breadth first search)')

            for neighbor_h in neighbors_h:
                neighbor_h_on_g = self.embedding.get_mapping_H_to_G_node(
                    neighbor_h)
                # Neighbor already embedded embedded?
                if neighbor_h_on_g:
                    # Try to add edge
                    if self.embedding.exists_edge(g, neighbor_h_on_g):
                        self.embedding.embed_edge_with_mapping(
                            h, g, neighbor_h, neighbor_h_on_g)
                    else:
                        # do nothing (this edge is added later using chains)
                        pass
                else:
                    # Choose random free neighbor
                    to_g = random.choice(free_neighbors_g)
                    free_neighbors_g.remove(to_g)

                    # Embed
                    self.embedding.embed_edge_with_mapping(
                        h, g, neighbor_h, to_g)
                    logger.info(f'Embedded edge: {g}---{to_g}')

                # Prepare queue to continue with adjacent nodes
                queue.append(neighbor_h)
                visited[h] = True

        self.embedding.try_to_add_missing_edges()

    def init_dfs(self):
        """Inits G using depth first search."""
        visited = [False] * self.H.nodes_count
        recursion_stack = Stack()
        self.dfs(0, visited, recursion_stack)  # start with node 0
        self.embedding.try_to_add_missing_edges()

    def dfs(self, to_h, visited, recursion_stack: Stack):
        """Depth first search recurion"""
        # Update status (visited array & recursion stack)
        from_h = recursion_stack.peek()  # peek first
        recursion_stack.push(to_h)  # then push
        visited[to_h] = True

        if from_h != None:  # from_h is None at the initial call of dfs()
            # Get from_g
            if from_h == 0:
                from_g = 0
            else:
                from_g = self.embedding.get_mapping_H_to_G_node(from_h)
                from_g = get_first_from_set(from_g)  # no chains yet

            # Get to_g
            # to_gs = self.embedding.get_mapping_H_to_G_node(to_h)
            # to_g = get_first_from_set(to_gs)  # no chains yet
            # if not to_g:  # this is often the case
            #     free_neighbors_g = self.embedding.get_free_neighbors(from_g)
            #     to_g = random.choice(free_neighbors_g)
            free_neighbors_g = self.embedding.get_free_neighbors(from_g)
            to_g = random.choice(free_neighbors_g)

            # Embed
            self.embedding.embed_edge_with_mapping(
                from_h, from_g, to_h, to_g)

        # DFS recursion
        neighbors_h = self.H.get_neighbor_nodes(to_h)
        for neighbor_h in neighbors_h:
            if not visited[neighbor_h]:
                self.dfs(neighbor_h, visited, recursion_stack)

        recursion_stack.pop()

    def get_embedding(self):
        return self.embedding.get_embedding(G_to_H_mapping=True)

    def commit(self, playground: Embedding):
        self.embedding = playground

    def _add_random_chain(self):
        # --- Randomly collapse two nodes to one
        nodes_embedded = self.embedding.get_embedded_nodes_not_in_chain()
        if not nodes_embedded:
            logger.info('🔴 No nodes to embed (all nodes are in a chain)')
            return None

        from_node = random.choice(nodes_embedded)
        node_tos = self.embedding.get_connected_neighbors(from_node)
        to_node = random.choice(node_tos)
        logger.info(f'Trying to chain nodes {from_node} and {to_node}')

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
            logger.info(f'node_to_new: {to_node_new}')

            # from node_to_new: can we reach all nodes previously connected to node_to?
            node_to_new_reachable_neighbors = self.embedding.get_reachable_neighbors(
                to_node_new)
            can_reach = [neighbor in node_to_new_reachable_neighbors
                         for neighbor in to_node_connected_neighbors if neighbor != from_node]
            if all(can_reach):
                # Try out on playground
                playground = self.embedding.get_playground()
                try:
                    playground.add_chain_to_used_nodes(
                        from_node, to_node, to_node_new)
                except:
                    continue
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
            try:
                logger.info(
                    f'Trying to add chain to used nodes: {from_node}, {to_node}, {to_node_new}')
                playground.add_chain_to_used_nodes(
                    from_node, to_node, to_node_new)
            except:
                return None

            logger.info(
                f'to_node_new to chain partner: {to_node_new}---{to_node_new_chain_partner}')
            playground.try_to_add_missing_edges()
            return playground

        # 3. If all of that fails, mark the mutation as failed
        return None

    def mutate(self):
        """
        Mutates the embedding. Supports adding random chains rights now.
        """
        logger.info('--- MUTATION')
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
