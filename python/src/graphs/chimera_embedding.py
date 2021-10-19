from src.graphs.chimera_graph import ChimeraGraph


class ChimeraEmbedding(ChimeraGraph):
    def __init__(self):
        super().__init__()

    def add_edge_embedding(self, frm, to):
        cost = self._get_edge_cost(frm, to)
        if cost == -1:  # no edge
            raise IndexError(f'Cannot add an embedding edge that does not \
                exist in the layout in the first place')
        self._set_edge(frm, to, cost=1)

    def remove_edge_embedding(self, frm, to):
        cost = self._get_edge_cost(frm, to)
        if cost < 1:  # no edge OR no edge embedding
            raise IndexError(f'Cannot remove an edge that is not embedded')
        self._set_edge(frm, to, cost=0)
