class GraphMapping():
    """
    Invertable unique dict using sets as values to encode the mapping between
    the small view of a graph and the big view.
    """

    def __init__(self):
        self.H_to_G = dict()  # values are ints
        self.G_to_H = dict()  # values are sets

    def get_node_G(self, node_H):
        # we explicitly do NOT handle KeyErrors here as they should never happen
        return self.H_to_G[node_H]

    def get_node_H(self, node_G):
        # we explicitly do NOT handle KeyErrors here as they should never happen
        return self.G_to_H[node_G]

    def set_mapping(self, node_H: int, node_G: int):
        self.H_to_G[node_H] = set([node_G])
        self.G_to_H[node_G] = node_H

    def extend_mapping(self, node_H: int, node_G: int):
        try:
            self.H_to_G[node_H].add(node_G)
        except KeyError:
            self.H_to_G[node_H] = set([node_G])
        self.G_to_H[node_G] = node_H

    def remove_mapping(self, node_H: int, node_G: int):
        self.H_to_G[node_H].remove(node_G)
        del self.G_to_H[node_G]

    def add_mapping_new_node_H(self, node_G: int):
        # Check if there exists already a mapping
        try:
            node_H = self.G_to_H[node_G]
            return node_H
        except KeyError:
            # Make new mapping to new node
            node_H = len(self.H_to_G.keys())
            self.set_mapping(node_H, node_G)
            return node_H

    def get_mapping_H_to_G(self):
        return self.H_to_G

    def get_mapping_G_to_H(self):
        return self.G_to_H
