import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx


class Draw():

    def __init__(self):
        self.pos = {
            0: (0., -0.5),
            1: (0.25, -0.5),
            2: (0.75, -0.5),
            3: (1, -0.5),
            4: (0.5, 0.),
            5: (0.5, -0.25),
            6: (0.5, -0.75),
            7: (0.5, -1.)
        }
        self.chain_colors = ['#55C1D9', '#F29E38',
                             '#F23827', '#D748F5', '#39DBC8']

    def draw_chimera_graph(self, m, n, t):
        """Draws a Chimera graph."""
        G = dnx.chimera_graph(m, n, t)
        dnx.draw_chimera(G,
                         width=2,
                         node_color='#858585',
                         edge_color='#BABABA')

    def draw_embedding(self, nodes, edges):
        """Draws the embedding onto the Chimera graph.

        Arguments
        ---------
        - nodes: nodes list
        - edges: tuple (from, to, chain)
        """
        chains = [edge[2] for edge in edges]
        for chain in range(max(chains)+1):
            chain_edges = [(edge[0], edge[1])
                           for edge in edges if edge[2] == chain]

            graph = nx.Graph()
            graph.add_nodes_from(nodes)
            graph.add_edges_from(chain_edges)

            chain_color = self.chain_colors[chain % len(self.chain_colors)]
            nx.draw_networkx(graph,
                             pos=self.pos,
                             width=3,
                             style='solid',
                             node_color='#363636',
                             edge_color=chain_color,
                             font_color='whitesmoke',
                             font_size=15,
                             font_family='serif')

        plt.show()
