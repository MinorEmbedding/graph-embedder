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
                gs = self.embedding.get_nodes_G(h)
                g = get_first_from(gs)

            free_neighbors_g = self.embedding.get_free_neighbors(g)
            if len(free_neighbors_g) < len(neighbors_h):
                raise RuntimeError(
                    'Not enough free neighbors to embed node of H in G (breadth first search)')

            for neighbor_h in neighbors_h:
                neighbor_h_on_g = self.embedding.get_nodes_G(
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
                from_g = self.embedding.get_nodes_G(from_h)
                from_g = get_first_from(from_g)  # no chains yet

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
            logger.info('❌ No nodes embedded yet')
            return None
        source = random.choice(list(embedded_nodes))

        # Choose target
        targets = self.embedding.get_embedded_neighbors(source)
        target = random.choice(list(targets))

        logger.info(f'🔗🔗 Trying to construct supernode: {source}, {target}')

        # Avoid unnecessary calculations
        if (source, target) in self.non_viable_mutations:
            logger.info('❌ Already considered but not viable -> skip')
            return None

        try:
            target_free_neighbors = self.embedding.get_free_neighbors(target)
        except NoFreeNeighborNodes:
            logger.info(f'❌ Target {target} has no free neighbors')
            return None

        # Adjust so that new super node placement is viable
        target_neighbors = self.embedding.get_embedded_neighbors(target)
        # no need to reach source from shifted_target
        target_neighbors.discard(source)
        logger.info(f'Target neighbors are: {target_neighbors}')

        # --- Try out possible positions for shifted_target
        # Always check if we can reach all super nodes previously connected
        # to target from the shifted_target
        for shifted_target in target_free_neighbors:
            logger.info(f'▶ Try out shifted target on node: {shifted_target}')
            res = self._construct_supernode_with_shifted_target(source, target,
                                                                shifted_target, target_neighbors)
            if res:
                return res

        # If it didn't work, mark the mutation as failed
        self.non_viable_mutations.append((source, target))
        logger.info('❌ No viable supernode placement')
        return None

    def get_reachable_node_in_neighbor_supernode(self, playground: Embedding, ignore_neighbors: set[int],
                                                 reachable: set[int], neighbor: int) -> Optional[int]:
        """We need to discard some nodes here as they are not actually part of the
        neighbor supernode. This is because we construct supernodes first to be
        able to embed edges correctly. However, we then need to compare with
        the a priori state and not this temporary state, which is why we
        have to discard some nodes."""
        neighbor_supernode = playground.get_supernode(neighbor)
        neighbor_supernode_nodes = playground.get_nodes_in_supernode(neighbor_supernode)
        for ignore in ignore_neighbors:
            neighbor_supernode_nodes.discard(ignore)

        logger.info(f'? Can we reach neighbor {neighbor} '
                    f'(supernode: {neighbor_supernode} -> {neighbor_supernode_nodes})')
        reachable_neighbor = any_of_one_in_other(reachable, neighbor_supernode_nodes)
        return reachable_neighbor

    def check_supernode_sanity(self, playground: Embedding, supernode: int) -> bool:
        """Checks that no supernodes are split up into multiple groups
        by the mutation. All nodes in a supernode must have an edge to at least
        one other supernode.
        """
        supernode_nodes = playground.get_nodes_in_supernode(supernode)
        for node in supernode_nodes:
            embedded_neighbors = playground.get_embedded_neighbors(node)

            reached = False
            for neighbor in embedded_neighbors:
                neighbor_supernode = playground.get_supernode(neighbor)
                if neighbor_supernode == supernode:
                    reached = True

            if not reached:
                return False

        return True

    def _construct_supernode_with_shifted_target(self, source: int, target: int,
                                                 shifted_target: int, target_neighbors: set[int]) -> Optional[Embedding]:
        """Tries to embed the shifted target, so that the node placement is viable.

        This means that we check if the new place for ``target`` - which is
        ``shifted_target`` - allows for the following edges:
            - ``target`` - ``shifted_target``
            - ``shifted_target`` - ``target_neighbors`` (all embedded ``target`` neighbors)
        """
        playground = self.embedding.get_playground()

        target_original_supernode = playground.get_supernode(target)

        # Get reachable neighbors
        # has to be calculated prior to supernode handover (!)
        shifted_target_reachable = \
            playground.get_reachable_neighbors(shifted_target)
        shifted_target_reachable.discard(source)  # no need to check this
        shifted_target_reachable.discard(target)  # no need to check this

        # supernode handover from target to shifted_target
        playground.construct_supernode(target, shifted_target)
        playground.construct_supernode(source, target)

        if not target_neighbors:
            logger.info(f'No target neighbors to check: {target_neighbors}')
            return playground

        # --- Check that previous connections still work
        # --- Try with one chain first
        neighbors_done = set()
        for neighbor in target_neighbors:
            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, {shifted_target}, shifted_target_reachable, neighbor)

            if target_neighbor != None:
                logger.info(f'Could reach {target_neighbor}')
                neighbors_done.add(neighbor)
                playground.embed_edge(shifted_target, target_neighbor)
            else:
                logger.info(f'Could not reach '
                            f'(reachable nodes are: {shifted_target_reachable})')
                # Continue normally with next neighbor as next one might be
                # reachable with this strategy

        # Check if this strategy was successfull
        if neighbors_done == target_neighbors:
            if self.check_supernode_sanity(playground, target_original_supernode):
                logger.info(f'Worked with 1st strategy (seldom !)')
                return playground
            else:
                logger.info(f'Supernode sanity not ensured.')
                return None

        # --- Try out to construct another chain to see if we can reach
        # remaining neighbors
        try:
            shifted_target_free_neighbors = playground.get_free_neighbors(shifted_target)
        except NoFreeNeighborNodes:
            logger.error(f'Could not construct another supernode '
                         f'from shifted_target: {shifted_target} '
                         f'(no free neighbor of shifted_target)')
            return None
        shifted_target_partner = get_first_from(shifted_target_free_neighbors)

        logger.info(f'🔗 Trying to construct another supernode: '
                    f'{shifted_target}, {shifted_target_partner}')
        # Chain shifted target and shifted target partner
        # prior to calling this, we need to make sure that shifted_target
        # now has the prior supernode of target
        # The call itself needs to take place prior to embedding edges outgoing
        # from shifted_target_partner (otherwise shifted_target_partner would
        # not have a viable supernode)
        playground.construct_supernode(shifted_target, shifted_target_partner)

        # Do not try to compute shifted_target reachable again here (!)
        # as these are different now since shifted_target has the previous
        # supernode of target
        shifted_target_partner_reachable = \
            playground.get_reachable_neighbors(shifted_target_partner)
        shifted_target_partner_reachable.discard(source)  # no need to check this
        shifted_target_partner_reachable.discard(target)  # no need to check this

        for neighbor in target_neighbors:
            if neighbor in neighbors_done:
                continue

            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, {shifted_target, shifted_target_partner}, shifted_target_partner_reachable, neighbor)

            if target_neighbor != None:
                logger.info(f'Could reach (with 2nd chain) {target_neighbor}')
                playground.embed_edge(shifted_target_partner, target_neighbor)
            else:
                logger.error(f'Could also not reach (reachable nodes are: '
                             f'{shifted_target_partner_reachable})')
                return None  # did not achieve a viable mutation even with 2nd strategy

        if self.check_supernode_sanity(playground, target_original_supernode):
            return playground
        else:
            logger.info(f'Supernode sanity not ensured.')
            return None

    def mutate(self) -> Optional[Embedding]:
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

    def local_maximum(self) -> int:
        return self.embedding.try_embed_missing_edges()

    def found_embedding(self) -> bool:
        return self.embedding.is_valid_embedding()
