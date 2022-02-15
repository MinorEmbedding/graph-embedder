import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx


class Draw():

    def __init__(self):
        # self.pos = {
        #     0: (0., -0.5),
        #     1: (0.25, -0.5),
        #     2: (0.75, -0.5),
        #     3: (1, -0.5),
        #     4: (0.5, 0.),
        #     5: (0.5, -0.25),
        #     6: (0.5, -0.75),
        #     7: (0.5, -1.)
        # }
        self.chain_colors = ['#55C1D9', '#F29E38',
                             '#F23827', '#D748F5', '#39DBC8', '#F5428A',
                             '#3CDE73', '#11F0EB', '#E9B952', '#7D2EFF',
                             '#DBDE5D', '#3A2CE0', '#DE6E31', '#E0165C']

    def draw_chimera_graph(self, m, n, t):
        """Draws a Chimera graph."""
        G = dnx.chimera_graph(m, n, t)
        self.pos_chimera = dnx.chimera_layout(G)

        nx.draw_networkx(G,
                         pos=self.pos_chimera,
                         width=2,
                         with_labels=True,
                         node_color='#858585',
                         edge_color='#BABABA')
        # dnx.draw_chimera(G,
        #                  width=2,
        #                  node_color='#858585',
        #                  edge_color='#BABABA')

    def _draw(self, nodes, edges, mapping_G_to_H):
        """Draws the embedding onto the Chimera graph.

        Arguments
        ---------
        - nodes: nodes list
        - edges: tuple (from, to, chain)
        """
        labels = {}
        for node in nodes:
            labels[node] = mapping_G_to_H[node]

        chains = [edge[2] for edge in edges]
        for chain in range(max(chains)+1):
            chain_edges = [(edge[0], edge[1])
                           for edge in edges if edge[2] == chain]

            graph = nx.Graph()
            graph.add_nodes_from(nodes)
            graph.add_edges_from(chain_edges)

            chain_color = self.chain_colors[chain % len(self.chain_colors)]
            nx.draw_networkx(graph,
                             labels=labels,
                             pos=self.pos_chimera,
                             width=3,
                             style='solid',
                             node_color='#363636',
                             edge_color=chain_color,
                             font_color='whitesmoke',
                             font_size=15,
                             font_family='serif')

    def draw_embedding(self, nodes, edges, mapping_G_to_H):
        self._draw(nodes, edges, mapping_G_to_H)
        plt.show()

    def save_and_clear(self, path):
        # plt.subplots_adjust(left=0.0, right=1.0, bottom=0.0, top=1.0)
        px = 1/plt.rcParams['figure.dpi']  # pixel in inches
        fig = plt.gcf()
        fig.set_size_inches(1000*px, 1000*px)
        fig.savefig(path, bbox_inches='tight')
        plt.clf()

    ############################ Big Plot ######################################

    def init_big_plot(self, row_count):
        self.big_plot_row_count = row_count
        # https://matplotlib.org/stable/gallery/subplots_axes_and_figures/figure_size_units.html
        px = 1/plt.rcParams['figure.dpi']  # pixel in inches
        fig = plt.figure(figsize=(row_count*200*px*3, row_count*200*px*3))
        plt.axis('off')
        plt.subplots_adjust(left=0.0, right=1.0, bottom=0.0, top=1.0,
                            wspace=0.0, hspace=0.0)
        self.big_plot_index = 1  # to start with (gets incremented later)

    def draw_to_big_plot(self, nodes, edges, mapping_G_to_H):
        plt.subplot(self.big_plot_row_count,
                    self.big_plot_row_count, self.big_plot_index)
        self.draw_chimera_graph(1, 1, 4)
        self._draw(nodes, edges, mapping_G_to_H)
        plt.draw()
        self.big_plot_index += 1

    def save_big_plot(self, filename):
        plt.savefig(f'./{filename}.svg')
