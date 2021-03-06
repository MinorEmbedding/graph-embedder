class UndirectedGraphAdjList:
    """
    Undirected Graph using an adjacency list.
    """

    def __init__(self, nodes_count: int):
        """Initializes a new Graph.

        Args:
            nodes_count (int): The number of nodes this Graph \
                has initially.
        """
        self._adj_list: dict[int, AdjListEntryWithCosts] = dict()
        self.current_index = -1
        self.nodes_count = 0  # gets incremented in for loop
        for _ in range(nodes_count):
            self.add_node()

    def _validate_index(self, index: int):
        """Checks if the given index is valid for the current Graph."""
        try:
            self._adj_list[index]
        except KeyError:
            raise GraphNodeIndexError(index)

    ############################### Edges ######################################

    def get_edges(self) -> set[tuple[int, int, int]]:
        """Returns all edges of this Graph.

        Returns:
            set[tuple[int, int, int]]: A set of entries (frm, to, cost) \
            describing all edges of this Graph with their respective costs.
        """
        edges = set()
        for frm in self._adj_list.keys():
            for (to, cost) in self._adj_list[frm].get_neighbors_with_costs():
                # frm, to = to, frm
                # would be fatal here inside the loop (!)
                if frm < to:
                    edges.add((frm, to, cost))
                else:
                    edges.add((to, frm, cost))
        return set(edges)

    def exists_edge(self, frm: int, to: int) -> bool:
        """Checks if an edge between two nodes exists.

        Args:
            frm (int): One node of the edge.
            to (int): The other node of the edge.

        Returns:
            bool: True if the edge exists.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` or ``to`` \
            does not exist in the Graph.
        """
        self._validate_index(frm)
        self._validate_index(to)
        return to in self.get_neighbor_nodes(frm)

    def set_edge(self, frm: int, to: int, cost=0) -> None:
        """Sets an edge between nodes ``frm`` and ``to`` and assigns
        the given ``cost``.

        Warning:
            This may override an existing edge with new costs.

        Args:
            frm (int): One node of the edge.
            to (int): The other node of the edge.
            cost (int): The costs for the edge

        Raises:
            GraphNodeIndexError: If the given node ``frm`` or ``to`` \
            does not exist in the Graph.
        """
        self._validate_index(frm)
        self._validate_index(to)

        self._adj_list[frm].set_edge_to(to, cost)
        self._adj_list[to].set_edge_to(frm, cost)  # undirected graph

    def remove_edge(self, frm: int, to: int) -> None:
        """Removes the edge (if present) between nodes ``frm`` and ``to``.

        Args:
            frm (int): One node of the edge.
            to (int): The other node of the edge.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` or ``to`` \
            does not exist in the Graph.
        """
        self._validate_index(frm)
        self._validate_index(to)

        self._adj_list[frm].remove_edge_to(to)
        self._adj_list[to].remove_edge_to(frm)  # undirected graph

    def remove_all_edges_from_node(self, frm: int) -> None:
        """Removes all edges connected to the given node.

        Args:
            frm (int): node from which all edges should be removed.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` \
            does not exist in the Graph.
        """
        self._validate_index(frm)

        tos = self._adj_list[frm].get_neighbors()
        if not tos:
            return  # nothing to delete

        for to in tos:
            # Delete outgoing edges (by resetting)
            self._adj_list[frm].remove_edge_to(to)

            # Delete incoming edges
            self._adj_list[to].remove_edge_to(frm)

    ################################# Nodes ####################################

    def get_nodes(self) -> set[int]:
        """Returns all nodes of this Graph.

        Returns:
            set[int]: All nodes of this Graph.
        """
        return set(self._adj_list.keys())

    def add_node(self) -> int:
        """Adds a new node to this Graph.

        Returns:
            int: The new node number.
        """
        self.nodes_count += 1
        self._adj_list[self.nodes_count-1] = AdjListEntryWithCosts()
        return self.nodes_count-1

    def remove_node(self, node: int) -> None:
        self.remove_all_edges_from_node(node)

        # We don't delete the key completely as we use the following encoding:
        # Nodes are only embedded nodes later if they have neighbors
        # del self._adj_list[node]

    def get_neighbor_nodes(self, source: int) -> set[int]:
        """Returns all neighboring nodes.

        Neighboring nodes are nodes that are connected via an edge to this node.

        Args:
            source (int): Node from which the neighboring nodes should be retrieved.

        Return:
            set[int]: Set of integers describing the neighboring nodes.

        Raises:
            GraphNodeIndexError: If the given node ``source`` \
            does not exist in the Graph.
        """
        self._validate_index(source)
        return self._adj_list[source].get_neighbors()

    def get_neighbor_nodes_with_costs(self, frm: int) -> list[tuple[int, int]]:
        """Returns all neighboring nodes with costs.

        Neighboring nodes are nodes that are connected via an edge to this node.

        Args:
            frm (int): Node from which the neighboring nodes \
                should be retrieved.

        Return:
            list[tuple[int, int]]: List of tuples (to, cost) \
            describing edges with costs.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` \
            does not exist in the Graph.
        """
        self._validate_index(frm)
        return self._adj_list[frm].get_neighbors_with_costs()

    def has_neighbor_nodes(self, frm: int) -> bool:
        """Checks if the given node has neighboring nodes.

        Neighboring nodes are nodes that are connected via an edge to this node.

        Args:
            frm (int): Node from which the neighboring nodes should be retrieved.

        Returns:
            bool: True if the given node has neighboring nodes.

        Raises:
            GraphNodeIndexError: If the given node ``frm`` \
            does not exist in the Graph.
        """
        return len(self.get_neighbor_nodes(frm)) > 0


class GraphNodeIndexError(Exception):
    """Error that occurs when accessing a node that does not exist in the Graph.

    Args:
        index (int): Invalid node (represented as integer index in the Graph).
    """

    def __init__(self, index: int):
        message = f'Node {index} does not exist in the graph'
        super().__init__(message)


class AdjListEntryWithCosts():
    """One entry of an adjacency list.

    Describes edges to other nodes and assigns costs to these edges.

    Warning:
        This AdjacencyList does *not* check for invalid indexing.
        Expect KeyErrors to arise if the caller does not handle indexing correctly.

    Note:
        It is guaranteed that an edge from node u to node v cannot exist \
            multiple times with different costs.
    """

    def __init__(self):
        self._to_nodes = set()
        self._costs = dict()

    ############################### Edges ######################################

    def set_edge_to(self, to: int, cost: int) -> None:
        """Sets an undirected edge with cost to a node.

        Warning:
            This may override an existing edge with new costs.

        Args:
            to (int): Other node to specify the edge.
            cost (int): Cost for the edge
        """
        self._to_nodes.add(to)
        self._costs[to] = cost  # may alter the costs for an existing edge

    def remove_edge_to(self, to: int) -> None:
        """Removes an edge to a node.

        Args:
            to (int): Other node to specify the edge that should be removed.
        """
        self._to_nodes.remove(to)
        del self._costs[to]

    ################################# Nodes ####################################

    def get_neighbors(self) -> set[int]:
        """Returns all neighboring nodes.

        Returns:
            set[int]: All neighboring nodes (from the current node).
        """
        return self._to_nodes.copy()

    def get_neighbors_with_costs(self) -> list[tuple[int, int]]:
        """Returns all neighboring nodes with costs (from the current node).

        Returns:
            list[tuple[int, int]]: All neighboring nodes (from the \
                current node) with costs.
        """
        return [(to, self._costs[to]) for to in self._to_nodes]
