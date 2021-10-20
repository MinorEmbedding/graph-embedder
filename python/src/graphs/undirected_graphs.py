class UndirectedGraphAdjMatrix:
    """
    An undirected Graph representation using an adjacency MATRIX with costs.
    The number of graph vertices is fixed after initialization.

    - `-1` in the adjacency matrix means: no edge
    - `0` in the adjacency matrix means: edge with cost 0 (default for cost)
    """

    def __init__(self, vertices_count):
        self.vertices_count = vertices_count
        self._adj_matrix = [[-1]*vertices_count for x in range(vertices_count)]

    def _set_edge(self, frm, to, cost=0):
        self._adj_matrix[frm][to] = cost
        # set vice-versa to preserve symmetric matrix (undirected graph)
        self._adj_matrix[to][frm] = cost

    def _get_edge_cost(self, frm, to):
        return self._adj_matrix[frm][to]

    def _get_edges(self):
        edges = []
        for i in range(self.vertices_count):
            for j in range(self.vertices_count):
                if (self._adj_matrix[i][j] != -1):
                    edges.append((i, j, self._adj_matrix[i][j]))
        return edges

    def _get_adj_matrix(self):
        return self._adj_matrix


class UndirectedGraphAdjList:
    """
    An undirected Graph representation using an adjacency LIST without costs.
    The number of graph vertices is fixed after initialization.

    This class may be primarily used for O(1) search for whether there is
    an edge from vertex u to vertex v.
    """

    def __init__(self, vertices_count):
        self.nodes_count = vertices_count
        self._adj_list = dict()
        for i in range(vertices_count):
            self._adj_list[i] = []

    def _set_edge(self, frm, to):
        try:
            self._adj_list[frm].append(to)
            # set vice-versa to preserve symmetric matrix (undirected graph)
            self._adj_list[to].append(frm)
        except:
            raise IndexError(
                f'Graph only contains {self.nodes_count} vertices')

    def _get_edges_from_node(self, from_vertex):
        try:
            return self._adj_list[from_vertex]
        except:
            raise IndexError(
                f'Graph only contains {self.nodes_count} vertices')

    def _get_nodes(self):
        return self._adj_list.keys()

    def _get_edges(self):
        return self._adj_list.values()
