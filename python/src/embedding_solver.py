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

    def _get_neighbors_not_used(self, from_vertex):
        neighbors = self._G_layout.get_neighbors(from_vertex)
        neighbors_used = self._G_embedding.get_embedded_nodes()
        neighbors_not_used = [neighbor for neighbor in neighbors
                              if neighbor not in neighbors_used]
        if not neighbors_not_used:
            raise ValueError('No more free neighbor nodes found')
        return neighbors_not_used

    def init_basic_path(self):
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
         The length is determined by the number of vertices of the minor to embed.
         """

        # Init with path graph as long as H
        # TODO: what if no path graph embedding is possible? When is this the case?
        # Start at vertex 0
        node = 0
        for i in range(self._H.nodes_count-1):
            neighbors = self._get_neighbors_not_used(node)
            next_vertex_id = random.choice(neighbors)
            self._G_embedding.embed_edge(node, next_vertex_id)
            node = next_vertex_id

    def get_current_embedding(self):
        return self._G_embedding.get_current_embedding()

    def calculate_cost(self):
        pass
        # Regard embedded graph as its own graph G'
        # starting from the first node that was once added # TODO: compare against other nodes
        # compare adj list from G' to adj list from H
        # H_matrix = self._H._get_adj_matrix()
        # HE_big_matrix = self._G_embedding.get_adj_matrix()

        # nodes, edges = self.get_current_embedding()
