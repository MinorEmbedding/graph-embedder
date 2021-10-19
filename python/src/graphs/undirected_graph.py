class UndirectedGraph:
    """
    An undirected Graph representation using an adjacency matrix with costs.

    - `-1` in the adjacency matrix means: no edge
    - `0` in the adjacency matrix means: edge with cost 0 (default for cost)
    """

    def __init__(self, vertices_count):
        self.vertices_count = vertices_count
        self.adj_matrix = [[-1]*vertices_count for x in range(vertices_count)]

    def add_edge(self, frm, to, cost=0):
        self.adj_matrix[frm][to] = cost
        # set vice-versa to presever symmetric matrix (undirected graph)
        self.adj_matrix[to][frm] = cost

    def get_edges(self):
        edges = []
        for i in range(self.vertices_count):
            for j in range(self.vertices_count):
                if (self.adj_matrix[i][j] != -1):
                    edges.append((i, j, self.adj_matrix[i][j]))
        return edges

    def get_adj_matrix(self):
        return self.adj_matrix
