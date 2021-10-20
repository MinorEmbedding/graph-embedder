import random

from src.graphs.chimera_graph import ChimeraGraphEmbedding, ChimeraGraphLayout


class EmbeddingSolver():

    def __init__(self, minor_vertices_count):
        self._minor_vertices_count = minor_vertices_count

        self._G_layout = ChimeraGraphLayout()
        self._G_embedding = ChimeraGraphEmbedding()

    # edge-based
    # def _get_neighbors_not_used(self, from_vertex):
    #     neighbors = self._G_layout.get_neighbors(from_vertex)
    #     # TODO: reduce from O(n) to O(1) by keeping a separate list
    #     # for embedded edges (!)
    #     neighbors_used = [edge[1] for edge in self._G_embedding.get_embedded_edges()
    #                       if edge[0] == from_vertex]
    #     return [neighbor for neighbor in neighbors
    #             if neighbor not in neighbors_used]

    # node-based
    def _get_neighbors_not_used(self, from_vertex):
        neighbors = self._G_layout.get_neighbors(from_vertex)
        neighbors_used = self._G_embedding.get_embedded_nodes()
        return [neighbor for neighbor in neighbors
                if neighbor not in neighbors_used]

    def init_basic_path(self):
        """Inits the graph as path graph starting from vertex 0 in the Chimera graph.
         The length is determined by the number of vertices of the minor to embed.
         """

        # Init with path graph as long as H
        # Start at vertex 0
        vertex_id = 0
        for i in range(self._minor_vertices_count):
            print('---------')
            print(i)
            neighbors = self._get_neighbors_not_used(vertex_id)
            print(neighbors)
            next_vertex_id = random.choice(neighbors)
            print(next_vertex_id)
            self._G_embedding.embed_edge(vertex_id, next_vertex_id)
            vertex_id = next_vertex_id
        # TODO: what if no path graph embedding is possible? When is this the case?

    def get_current_embedding(self):
        nodes = [i for i in range(self._minor_vertices_count)]

        # costs are not necessary here anymore, strip them
        edges = [edge[0:2] for edge in self._G_embedding.get_embedded_edges()]

        return nodes, edges
