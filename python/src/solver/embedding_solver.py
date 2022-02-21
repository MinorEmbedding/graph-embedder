import logging
import random

from src.embedding.embedding import Embedding, NoFreeNeighborNodes
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
            target = random.choice(neighbors)

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
        self.reset()

    def reset(self):
        self.non_viable_mutations = []

    def _add_random_chain(self):
        # --- Randomly merge two nodes into one super-node (add a chain between them)
        nodes_embedded = self.embedding.get_embedded_nodes()
        if not nodes_embedded:
            logger.info('🔴 No nodes to embed (all nodes are in a chain)')
            return None

        source = random.choice(nodes_embedded)
        targets = self.embedding.get_embedded_neighbors(source)

        # Filter out nodes that are in the same chain since another chain doesn't
        # make sense in this case
        source_chain = self.embedding.G_embedding.get_first_node_chain_other_than_default(
            source)
        if source_chain:
            targets_filtered = []
            for target in targets:
                target_chain = self.embedding.G_embedding.get_first_node_chain_other_than_default(
                    target)
                if (not target_chain) or (target_chain and source_chain != target_chain):
                    targets_filtered.append(target)
            targets = targets_filtered
            if not targets:
                logger.info(
                    f'Could not find a valid chain partner for node {source}')
                return

        target = random.choice(targets)

        logger.info(f'Trying to chain nodes {source} and {target}')

        # Avoid unnecessary calculations
        if (source, target) in self.non_viable_mutations:
            logger.info(
                'Already considered this pair for a random chain -> skip')
            return None

        # --- Adjust so that new chain is viable
        # find new place for previous node_to in the graph
        # so that the layout permits the following edges
        # node_to   --- node_to'
        # node_to'  --- all nodes reachable from node_to
        try:
            target_free_neighbors = self.embedding.get_free_neighbors(target)
        except NoFreeNeighborNodes:
            return None

        target_embedded_neighbors = set(self.embedding.get_embedded_neighbors(
            target))
        # Ignore source as we don't need to reach it from target'
        # since the chain gets expanded to target and there will definitely
        # be an edge target --- target'
        target_embedded_neighbors.discard(source)
        target_embedded_neighbors_chain = [self.embedding.G_embedding.get_node_chains(
            node, include_default_chain=False) for node in target_embedded_neighbors]
        target_embedded_neighbors_chain = [get_first_from_set(
            chain) if chain else None for chain in target_embedded_neighbors_chain]

        # --- 1) Try out all possible positions for shifted_target
        # TODO: ❗ Instead of precalculating if insertion would work and then
        # execute it, combine this and do the actual embedding on the fly
        for shifted_target in target_free_neighbors:
            logger.info(f'shifted_target: {shifted_target}')

            # from shifted_target: can we reach all nodes previously connected to target?
            shifted_target_reachable_neighbors = set(self.embedding.get_reachable_neighbors(
                shifted_target))

            # Remove to prevent cycles
            shifted_target_reachable_neighbors.discard(source)
            shifted_target_reachable_neighbors.discard(target)

            # Precalculate chains of reachable neighbors
            reachable_neighbors_chain = [self.embedding.G_embedding.get_first_node_chain_other_than_default(node)
                                         for node in shifted_target_reachable_neighbors]

            # --- Check if we can reach all previous neighbors
            can_reach_all = True
            print(f'🟢 shifted_target: {shifted_target}')
            for i, neighbor in enumerate(target_embedded_neighbors):
                chain = target_embedded_neighbors_chain[i]
                print(
                    f'🟢 target embedded neighbors: {neighbor} - in chain: {chain}')
                if chain:
                    # If we deal with a neighbor node that is in a chain, we only need
                    # to connect node_to' to a node that is in the same chain (but it
                    # does not have to be a direct neighbor of node_to')

                    # The question is: can we reach a node in that chain?
                    if chain not in reachable_neighbors_chain:
                        can_reach_all = False
                        break  # and go on with next shifted_target
                else:
                    # Deal with neighbors that are not in a chain
                    if neighbor not in shifted_target_reachable_neighbors:
                        # print(
                        #     f'{neighbor} not in {node_to_new_reachable_neighbors}')
                        can_reach_all = False
                        break  # and go on with next shifted_target

            # Embed on playground
            if can_reach_all:
                playground = self.embedding.get_playground()
                playground.add_chain_to_used_nodes(
                    source, target, shifted_target)
                return playground

        # --- 2) If step 1) did not work, try to construct another new chain (-> two chains in total)
        shifted_target = target_free_neighbors[0]
        shifted_target_reachable_neighbors = self.embedding.get_free_neighbors(
            shifted_target)
        shifted_target_partner = shifted_target_reachable_neighbors[0]
        logger.info(
            f'Trying to construct another chain: {shifted_target}, {shifted_target_partner}')

        # from shifted_target AND shifted_target_partner:
        # Can we now reach all nodes previously connected to node_to
        chain_reachable_nodes = self.embedding.get_reachable_neighbors(
            shifted_target)
        chain_reachable_nodes.extend(
            self.embedding.get_reachable_neighbors(shifted_target_partner))
        can_reach = [neighbor in chain_reachable_nodes
                     for neighbor in target_embedded_neighbors if neighbor != source]
        if all(can_reach):
            # Try out on playground
            playground = self.embedding.get_playground()

            playground.extend_one_node_to_chain(
                shifted_target, shifted_target_partner, extend_G=target)

            # edge target---shifted_target
            # will be removed and inserted again when adding this chain:
            logger.info(
                f'Trying to add chain to used nodes: {source}, {target}, {shifted_target}')
            playground.add_chain_to_used_nodes(
                source, target, shifted_target, shifted_target_partner)

            logger.info(
                f'shifted_target to chain partner: {shifted_target}---{shifted_target_partner}')
            return playground

        # 3. If all of that fails, mark the mutation as failed
        self.non_viable_mutations.append((source, target))
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

    def local_maximum(self):
        self.embedding.try_to_add_missing_edges()

    def found_embedding(self) -> bool:
        return self.embedding.is_valid_embedding()
