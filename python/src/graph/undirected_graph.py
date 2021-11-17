class UndirectedGraphAdjList:
    """
    An undirected Graph representation with costs using an adjacency list.
    """

    def __init__(self, nodes_count):
        self.nodes_count = nodes_count
        self._adj_list: dict[int, AdjListEntryWithCosts] = dict()
        for i in range(nodes_count):
            self._adj_list[i] = AdjListEntryWithCosts()

    def _validate_index(self, index):
        """Checks if the given index is valid for the current Graph."""
        if not 0 <= index < self.nodes_count:
            raise GraphIndexError(self.nodes_count, index)

    ############################### Edges ######################################

    def get_edges(self):
        """Returns all edges of this Graph."""
        edges = set()
        for frm in self._adj_list.keys():
            for (to, cost) in self._adj_list[frm].get_reachable_with_costs():
                # frm, to = to, frm
                # would be fatal here inside the loop (!)
                if frm < to:
                    edges.add((frm, to, cost))
                else:
                    edges.add((to, frm, cost))
        return edges

    def set_edge(self, frm, to, cost=0):
        """Sets an edge between nodes frm and to and assigns the given cost."""
        self._validate_index(frm)
        self._validate_index(to)

        self._adj_list[frm].add_edge_to(to, cost)
        self._adj_list[to].add_edge_to(frm, cost)  # undirected graph

    def remove_edge(self, frm, to):
        """Removes the edge (if present) between nodes frm and to."""
        self._validate_index(frm)
        self._validate_index(to)

        self._adj_list[frm].remove_edge_to(to)
        self._adj_list[to].remove_edge_to(frm)  # undirected graph

    def remove_all_edges_from_node(self, frm):
        """Removes all edges connected to the given node."""
        self._validate_index(frm)

        tos = self._adj_list[frm].get_reachable()
        if not tos:
            return  # nothing to delete

        # Delete outgoing edges (by resetting)
        self._adj_list[frm] = AdjListEntryWithCosts()

        # Delete incoming edges
        for to in tos:
            self._adj_list[to].remove_edge_to(frm)

    ################################# Nodes ####################################

    def get_nodes(self):
        """Returns all nodes of this Graph."""
        return self._adj_list.keys()

    def add_node(self):
        """Adds a new node to this Graph."""
        self.nodes_count += 1
        # add node to the end
        self._adj_list[self.nodes_count-1] = AdjListEntryWithCosts()

    def get_neighbor_nodes(self, frm) -> list:
        """Returns all neighboring nodes
        (nodes connected via an edge to this node)."""
        self._validate_index(frm)
        return list(self._adj_list[frm].get_reachable())

    def get_reachable_nodes_with_costs(self, frm):
        """Returns all neighboring nodes with costs
        (nodes connected via en edge to this node)."""
        self._validate_index(frm)
        return self._adj_list[frm].get_reachable_with_costs()

    def has_neighbor_nodes(self, frm):
        """Checks if the given node has neighboring nodes
        (equal to outgoing edges)."""
        return len(self.get_neighbor_nodes(frm)) > 0


class GraphIndexError(Exception):
    def __init__(self, nodes_count, index):
        message = f'Graph contains {nodes_count} nodes, cannot access node no. {index}'
        super().__init__(message)


class AdjListEntryWithCosts():
    """
    One entry of an adjacency list. Allows for costs. Makes sure that an edge
    from node u to node v cannot exist multiple times with different costs.
    """

    def __init__(self):
        self._to_nodes = set()
        self._costs = dict()

    ############################### Edges ######################################

    def add_edge_to(self, to, cost):
        """Adds an edge to a node with given costs."""
        self._to_nodes.add(to)
        self._costs[to] = cost  # may alter the costs for an existing edge

    def remove_edge_to(self, to):
        """Removes an edge to a node."""
        self._to_nodes.remove(to)
        del self._costs[to]

    ################################# Nodes ####################################

    def get_reachable(self) -> set:
        """Returns all reachable nodes (from the current node)."""
        return self._to_nodes

    def get_reachable_with_costs(self) -> list:
        """Returns all reachable nodes with costs (from the current node)."""
        return [(to, self._costs[to]) for to in self._to_nodes]
