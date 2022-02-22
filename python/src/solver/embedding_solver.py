import logging
import random
from typing import Optional

from src.embedding.embedding import Embedding, NoFreeNeighborNodes
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.util.stack import Stack
from src.util.util import any_of_one_in_other, get_first_from

logger = logging.getLogger('evolution')


class EmbeddingSolver():

    def __init__(self, H: UndirectedGraphAdjList):
        self.H = H
        if H.nodes_count < 2:
            raise NameError('The minor to embed must have at least two nodes')

        self.embedding = Embedding(H)
        self.non_viable_mutations = []

    def init_basic_path(self) -> None:
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
        The length is determined by the number of vertices of the minor to embed.
        """
        # Init with path graph as long as H
        # TODO: what if no path graph embedding is possible? When is this the case?
        # Start at vertex 0
        source = 0
        for _ in range(self.H.nodes_count-1):
            # Choose random neighbor
            neighbors = self.embedding.get_free_neighbors(source)
            target = random.choice(list(neighbors))

            # Embed
            self.embedding.embed_edge(source, target)

            source = target

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
                g = get_first_from(gs)

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
                    to_g = random.choice(list(free_neighbors_g))
                    free_neighbors_g.remove(to_g)

                    # Embed
                    self.embedding.embed_edge_with_mapping(
                        h, g, neighbor_h, to_g)
                    logger.info(f'Embedded edge: {g}-{to_g}')

                # Prepare queue to continue with adjacent nodes
                queue.append(neighbor_h)
                visited[h] = True

    def init_dfs(self):
        """Inits G using depth first search."""
        visited = [False] * self.H.nodes_count
        recursion_stack = Stack()
        self.dfs(0, visited, recursion_stack)  # start with node 0

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
                from_g = get_first_from(from_g)  # no chains yet

            # Get to_g
            # to_gs = self.embedding.get_mapping_H_to_G_node(to_h)
            # to_g = get_first_from_set(to_gs)  # no chains yet
            # if not to_g:  # this is often the case
            #     free_neighbors_g = self.embedding.get_free_neighbors(from_g)
            #     to_g = random.choice(free_neighbors_g)
            free_neighbors_g = self.embedding.get_free_neighbors(from_g)
            to_g = random.choice(list(free_neighbors_g))

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
        self.reset()

    def reset(self):
        self.non_viable_mutations = []

    def extend_random_supernode(self):
        """Randomly merges two nodes into one super node or extends existing
        super nodes by merging another node into it. This process might also
        create up to two new super nodes."""
        # Choose source
        embedded_nodes = self.embedding.get_embedded_nodes()
        if not embedded_nodes:
            logger.info('❌ Extend random supernode operation failed: '
                        'No nodes embedded yet')
            return None
        source = random.choice(list(embedded_nodes))

        # Choose target
        targets = self.embedding.get_embedded_neighbors(source)
        target = random.choice(list(targets))

        logger.info(f'🔗 Trying to construct supernode: {source}, {target}')

        # Avoid unnecessary calculations
        if (source, target) in self.non_viable_mutations:
            logger.info('Already considered but not viable -> skip')
            return None

        target_supernode_nodes = self.embedding.get_nodes_in_supernode_of(target)
        if len(target_supernode_nodes) > 1:
            res = self._construct_supernode_where_target_in_chain(
                source, target, target_supernode_nodes)
            if res:
                return res
        else:
            # Adjust so that new super node placement is viable
            target_neighbors = self.embedding.get_embedded_neighbors(target)
            # no need to reach source from shifted_target
            target_neighbors.discard(source)
            try:
                target_free_neighbors = self.embedding.get_free_neighbors(target)
            except NoFreeNeighborNodes:
                logging.info(f'Target {target} has no free neighbors')
                return None

            # --- Try out possible positions for shifted_target
            # Always check if we can reach all super nodes previously connected
            # to target from the shifted_target
            for shifted_target in target_free_neighbors:
                logger.info(f'▶ Try out shifted target on node: {shifted_target}')
                res = self._construct_supernode_with_shifted_target(source, target,
                                                                    shifted_target, target_neighbors,
                                                                    target_free_neighbors)
                if res:
                    return res

        # If it didn't work, mark the mutation as failed
        self.non_viable_mutations.append((source, target))
        logger.info('❌ Extend random supernode operation failed: '
                    'No viable supernode placement')
        return None

    def _construct_supernode_where_target_in_chain(self, source: int, target: int, target_supernode_nodes: set[int]):
        """Constructs a supernode in the case where the target is in another
        supernode consisting of more than 1 node, which we call "chain".

        In this case we shrink the chain by removing the target from it.
        """
        playground = self.embedding.get_playground()

        target_neighbors = playground.get_embedded_neighbors(target)
        embedded_edge = False
        for neighbor in target_neighbors:
            if target_neighbors in target_supernode_nodes:
                embedded_edge = True
                playground.embed_edge(target, neighbor)

        if not embedded_edge:
            logger.error(f'❌ Fatal error: Could not embed edge from '
                         f'target {target} to one of its supernode neighbors: '
                         f'{target_supernode_nodes}')
            return None

        playground.construct_supernode(source, target)
        return playground

    def _construct_supernode_with_shifted_target(self, source: int, target: int,
                                                 shifted_target: int, target_neighbors: set[int],
                                                 target_free_neighbors: set[int]):
        """Tries to embed the shifted target, so that the node placement is viable.

        This means that we check if the new place for ``target`` - which is
        ``shifted_target`` - allows for the following edges:
            - ``target`` - ``shifted_target``
            - ``shifted_target`` - ``target_neighbors`` (all embedded ``target`` neighbors)
        """
        playground = self.embedding.get_playground()

        # Get reachable neighbors
        shifted_target_reachable = \
            playground.get_reachable_neighbors(shifted_target)
        shifted_target_reachable.discard(source)  # prevent cycles
        shifted_target_reachable.discard(target)  # prevent cycles

        if not target_neighbors:
            logger.info(f'No target neighbors to check')

        for neighbor in target_neighbors:
            supernode = playground.get_supernode(neighbor)
            supernode_nodes = playground.get_nodes_in_supernode(supernode)

            logger.info(f'Can we reach neighbor {neighbor} '
                        f'(supernode: {supernode} -> {supernode_nodes})')
            other = any_of_one_in_other(shifted_target_reachable, supernode_nodes)
            if other == -1:
                logger.info(f'Could not reach with any of {shifted_target_reachable}')

                # Try out to construct another chain
                shifted_target_partner = get_first_from(target_free_neighbors)
                logger.info(f'🔗 Trying to construct another supernode: '
                            f'{shifted_target}, {shifted_target_partner}')

                shifted_target_partner_reachable = \
                    playground.get_reachable_neighbors(shifted_target_partner)
                shifted_target_partner_reachable.discard(source)  # prevent cycles
                shifted_target_partner_reachable.discard(target)  # prevent cycles

                other = any_of_one_in_other(
                    shifted_target_partner_reachable, supernode_nodes)
                if other == -1:
                    logger.error(f'Could also not reach with any of '
                                 f'{shifted_target_partner_reachable}')
                    return None

                playground.construct_supernode(shifted_target, shifted_target_partner)

            playground.add_to_supernode(supernode, node_in_supernode=other,
                                        node_to_include=shifted_target)

        playground.embed_edge_with_source_supernode_mapping(target, shifted_target)
        playground.construct_supernode(source, target)

        return playground

    def mutate(self):
        """
        Mutates the embedding. Supports adding random chains rights now.
        """
        logger.info('--- MUTATION')
        # --- Delete & Insert edge
        # Delete an edge between two random nodes that were already embedded.
        # Insert a new edge between two other random nodes that were already embedded.

        # Add random chain
        return self.extend_random_supernode()

        # TODO: Remove chain mutation

        # --- Perspective change
        # TODO: with low probability: view reduced graph from completely different view
        # maybe from this perspective, we can leverage some better mutations
        # and reduce the costs faster

    def local_maximum(self):
        self.embedding.try_embed_missing_edges()

    def found_embedding(self) -> bool:
        return self.embedding.is_valid_embedding()
