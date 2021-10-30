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

    def __init__(self, nodes_count):
        self.nodes_count = nodes_count
        self._adj_list = dict()
        for i in range(nodes_count):
            self._adj_list[i] = AdjListEntryWithCosts()

    def _set_edge(self, frm, to, cost=0):
        try:
            self._adj_list[frm].add(to, cost)
            # set vice-versa to preserve symmetric matrix (undirected graph)
            self._adj_list[to].add(frm, cost)
        except:
            raise IndexError(
                f'Graph only contains {self.nodes_count} vertices')

    def _remove_edge(self, frm, to):
        self._adj_list[frm].remove(to)
        self._adj_list[to].remove(frm)

    def _delete_all_edges_from_node(self, frm):
        tos = self._adj_list[frm].get_to_nodes()
        if not tos:
            return

        # Delete all outgoing edges
        self._adj_list[frm] = AdjListEntryWithCosts()

        # Delete specific incoming edges
        for to in tos:
            self._adj_list[to].remove(frm)

    def _get_edges_from_node(self, from_node):
        try:
            return self._adj_list[from_node].get()
        except:
            raise IndexError(
                f'Graph only contains {self.nodes_count} vertices')

    def _get_neighbor_nodes(self, from_node):
        neighbors = self._get_edges_from_node(from_node)
        # We only need the neighbor nodes, no costs
        neighbors = [neighbor[0] for neighbor in neighbors]
        return neighbors

    def _has_node_edges(self, node):
        return len(self._adj_list[node].get()) > 0

    def _get_nodes(self):
        return self._adj_list.keys()

    def _add_node(self):
        self.nodes_count += 1
        self._adj_list[self.nodes_count-1] = AdjListEntryWithCosts()

    def _get_edges(self):
        edges = set()
        for frm in self._adj_list.keys():
            for (to, cost) in self._adj_list[frm].get():
                # frm, to = to, frm
                # would be fatal here inside the loop (!)
                if frm < to:
                    edges.add((frm, to, cost))
                else:
                    edges.add((to, frm, cost))
        return edges


class AdjListEntryWithCosts():
    """
    One entry of an adjacency list. Allows for costs. Makes sure that an edge
    from node u to node v cannot exist multiple times with different costs.
    """

    def __init__(self):
        self.to_nodes = set()
        self.costs = dict()

    def add(self, to, cost):
        self.to_nodes.add(to)
        self.costs[to] = cost  # may alter the costs for an existing edge

    def remove(self, to):
        self.to_nodes.remove(to)
        del self.costs[to]

    def get_to_nodes(self):
        return self.to_nodes

    def get(self):
        return [(to, self.costs[to]) for to in self.to_nodes]
