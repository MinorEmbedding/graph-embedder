import random

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

    def _get_free_neighbors(self, from_vertex):
        neighbors = self._G_layout.get_neighbors(from_vertex)
        neighbors_used = self._G_embedding.get_embedded_nodes()
        neighbors_free = [neighbor for neighbor in neighbors
                          if neighbor not in neighbors_used]
        if not neighbors_free:
            raise ValueError('No more free neighbor nodes found')
        return neighbors_free

    def init_basic_path(self):
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

            node = next_node

    def get_current_embedding(self):
        return self._G_embedding.get_current_embedding()

    def calculate_cost(self):
        cost = 0

        print('--- COST calculation')
        for frm in self._H._get_nodes():
            expected_tos = self._H._get_edges_from_node(frm)
            # Premise: embedded graph has at any time the same number of nodes
            # as the minor graph H that we try to embed
            actual_tos = self._G_reduced_embedding._get_edges_from_node(frm)
            print(f'{expected_tos} vs. {actual_tos}')

            if expected_tos == actual_tos:
                cost -= 10  # small reward

            for to in expected_tos:
                if to not in actual_tos:
                    cost += 20  # big punishment

            for to in actual_tos:
                if to not in expected_tos:
                    cost += 5  # small punishment since unnecessary

        return cost

        # nodes, edges = self._G_reduced_embedding.get_current_embedding()
        # print('REDUCED EMBEDDING')
        # print(nodes)
        # print(edges)
