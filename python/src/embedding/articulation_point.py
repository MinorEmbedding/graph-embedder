
from src.embedding.embedding import EmbeddingGraph
from src.util.util import get_first_from


class ArticulationPointCalculator:

    def __init__(self, G: EmbeddingGraph) -> None:
        self.G = G

    def calc_articulation_points(self, nodes: set[int]) -> set[int]:
        """Returns the articulation points aka "cut nodes" in the given subgraph"""
        self.nodes = nodes

        self.time = 0
        self.discovery_time = {node: -1 for node in nodes}
        self.min_level = {node: -1 for node in nodes}
        self.articulation_points = set()

        self.visited = {node: False for node in nodes}
        self.parent = {node: -1 for node in nodes}
        self._dfs_articulation_points(get_first_from(nodes))

        return self.articulation_points.copy()

    def _dfs_articulation_points(self, node: int) -> None:
        self.visited[node] = True

        children_count = 0
        self.discovery_time[node] = self.time
        self.time += 1
        self.min_level[node] = self.time

        # DFS recursion
        for neighbor in self.G.get_neighbor_nodes(node):
            # Only consider nodes in subgraph
            if neighbor not in self.nodes:
                continue

            if not self.visited[neighbor]:
                children_count += 1
                self.parent[neighbor] = node
                # Recurse
                self._dfs_articulation_points(neighbor)

                # Check if the subtree rooted with neighbor has a connection to
                # one of the ancestors of node
                self.min_level[node] =\
                    min(self.min_level[node], self.min_level[neighbor])

                # Check if articulation point
                # Node is an articulation point in the following cases
                if self.parent[node] == -1:
                    # (1) node is root of DFS tree and has two or more children
                    if children_count > 1:
                        self.articulation_points.add(node)
                else:
                    # (2) If node is not root and discovery time of node is less
                    # or equal than min_level of one of its children
                    if self.discovery_time[node] <= self.min_level[neighbor]:
                        self.articulation_points.add(node)

            elif neighbor != self.parent[node]:  # only if there is a "back-edge"
                # Update min_level of node
                self.min_level[node] =\
                    min(self.min_level[node], self.discovery_time[neighbor])
