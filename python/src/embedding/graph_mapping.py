from typing import Optional


class GraphMapping():
    """
    Invertable unique dict using sets as values to encode the mapping between
    nodes in the input graph H to embed into the hardware graph G.
    """

    def __init__(self, size_H: int):
        self.H_to_G = dict()  # values are ints
        self.G_to_H = dict()  # values are sets
        self.max_supernodes_count = size_H

    def get_nodes_G(self, node_H: int) -> set[int]:
        # Note that we do not handle KeyErrors here as they should never occur
        # Might want to use method: get_supernode_create_if_not_available()
        return self.H_to_G[node_H].copy()

    def get_node_H(self, node_G: int) -> int:
        # Note that we do not handle KeyErrors here as they should never occur
        return self.G_to_H[node_G]

    def set_mapping(self, node_H: int, node_G: int):
        self.H_to_G[node_H] = set([node_G])
        self.G_to_H[node_G] = node_H

    def extend_mapping(self, node_H: int, node_G: int):
        """Places ``node_G`` into the super node representing ``node_H``."""
        try:
            self.H_to_G[node_H].add(node_G)
        except KeyError:
            self.H_to_G[node_H] = set([node_G])
        self.G_to_H[node_G] = node_H

    def remove_mapping(self, node_H: int, node_G: int):
        """Removes ``node_G`` from the super node represents ``node_H``."""
        self.H_to_G[node_H].remove(node_G)
        del self.G_to_H[node_G]

    def get_supernode_create_if_not_available(self, node_G: int) -> int:
        """Returns the supernode for ``node_G``.

        If there is no supernode of ``node_G``, create a new supernode and return it.
        """
        try:
            supernode = self.G_to_H[node_G]
        except KeyError:
            supernode = self._get_new_supernode()
            self.set_mapping(supernode, node_G)

        return supernode

    def _get_new_supernode(self) -> int:
        supernode = len(self.H_to_G.keys())
        if supernode >= self.max_supernodes_count:
            raise KeyError(f'Maximum of possible embeddings reached: '
                           f'{self.max_supernodes_count}')
        return supernode

    def get_mapping_H_to_G(self):
        return self.H_to_G.copy()

    def get_mapping_G_to_H(self):
        return self.G_to_H.copy()
