import logging
import random

from src.embedding.embedding import Embedding
from src.util.stack import Stack
from src.util.util import get_first_from

logger = logging.getLogger('evolution')


class Initialization():

    def __init__(self, embedding: Embedding) -> None:
        self._embedding = embedding

    def init_basic_path(self) -> None:
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
        The length is determined by the number of vertices of the minor to embed.
        """
        # Init with path graph as long as H
        # TODO: what if no path graph embedding is possible? When is this the case?
        # Start at vertex 0

        source = 0
        for _ in range(self._embedding.H.nodes_count-1):
            # Choose random neighbor
            neighbors = self._embedding.get_free_neighbors(source)
            target = random.choice(list(neighbors))

            # Embed
            self._embedding.embed_edge(source, target)

            source = target

    def init_bfs(self):
        """Traverses through H and inits graph H usugin breadth first search on H."""
        # Mark all vertices as not visited
        visited = [False] * self._embedding.H.nodes_count

        queue = []

        # Mark source node as visited
        h = 0  # start with node 0 in graph H
        queue.append(h)
        visited[h] = True

        while queue:
            # Dequeue node & get neighbors
            h = queue.pop(0)
            neighbors_h = self._embedding.H.get_neighbor_nodes(h)
            # Filter out already visited nodes in H
            neighbors_h = [h for h in neighbors_h
                           if not visited[h]]

            # Embed edges to all neighbors
            if h == 0:
                g = 0  # start with node 0 for embedding in graph G
            else:
                gs = self._embedding.get_nodes_G(h)
                g = get_first_from(gs)

            free_neighbors_g = self._embedding.get_free_neighbors(g)
            if len(free_neighbors_g) < len(neighbors_h):
                raise RuntimeError(
                    'Not enough free neighbors to embed node of H in G (breadth first search)')

            for neighbor_h in neighbors_h:
                neighbor_h_on_g = self._embedding.get_nodes_G(
                    neighbor_h)
                # Neighbor already embedded embedded?
                if neighbor_h_on_g:
                    # Try to add edge
                    if self._embedding.exists_edge(g, neighbor_h_on_g):
                        self._embedding.embed_edge_with_mapping(
                            h, g, neighbor_h, neighbor_h_on_g)
                    else:
                        # do nothing (this edge is added later using chains)
                        pass
                else:
                    # Choose random free neighbor
                    to_g = random.choice(list(free_neighbors_g))
                    free_neighbors_g.remove(to_g)

                    # Embed
                    self._embedding.embed_edge_with_mapping(
                        h, g, neighbor_h, to_g)
                    logger.info(f'Embedded edge: {g}-{to_g}')

                # Prepare queue to continue with adjacent nodes
                queue.append(neighbor_h)
                visited[h] = True

    def init_dfs(self):
        """Inits G using depth first search."""
        visited = [False] * self._embedding.H.nodes_count
        recursion_stack = Stack()
        self._dfs(0, visited, recursion_stack)  # start with node 0

    def _dfs(self, to_h, visited, recursion_stack: Stack):
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
                from_g = self._embedding.get_nodes_G(from_h)
                from_g = get_first_from(from_g)  # no chains yet

            free_neighbors_g = self._embedding.get_free_neighbors(from_g)
            to_g = random.choice(list(free_neighbors_g))

            # Embed
            self._embedding.embed_edge_with_mapping(
                from_h, from_g, to_h, to_g)

        # DFS recursion
        neighbors_h = self._embedding.H.get_neighbor_nodes(to_h)
        for neighbor_h in neighbors_h:
            if not visited[neighbor_h]:
                self._dfs(neighbor_h, visited, recursion_stack)

        recursion_stack.pop()
