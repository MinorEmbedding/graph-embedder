import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib import gridspec
from matplotlib.axes import Axes


class DrawEmbedding():

    def __init__(self):
        self.total_steps = 0

        self.chain_colors = ['#55C1D9', '#F29E38',
                             '#F23827', '#D748F5', '#39DBC8', '#F5428A',
                             '#3CDE73', '#11F0EB', '#E9B952', '#7D2EFF',
                             '#DBDE5D', '#3A2CE0', '#DE6E31', '#E0165C']

        # https://matplotlib.org/stable/gallery/subplots_axes_and_figures/figure_size_units.html
        self.px = 1/plt.rcParams['figure.dpi']  # pixel in inches
        plt.rcParams.update({'axes.titlesize': 24})

        # Init figure
        self.fig = plt.figure()
        plt.subplots_adjust(left=0.0, right=1.0,
                            bottom=0.0, top=1.0,
                            wspace=0.1, hspace=0.1)

        self.col = 0

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

    def draw_embedding(self, nodes, edges, mapping_G_to_H):
        """Draws the embedding onto the Chimera graph.

        Arguments
        ---------
        - nodes: nodes list
        - edges: tuple (from, to, chain)
        """
        labels = {}
        for node in nodes:
            labels[node] = mapping_G_to_H[node]

        G = nx.Graph()
        G.add_nodes_from(nodes)

        # Nodes
        nx.draw_networkx_nodes(G,
                               pos=self.pos_chimera,
                               node_color='#00000066',
                               linewidths='2')

        # Labels
        nx.draw_networkx_labels(G,
                                pos=self.pos_chimera,
                                labels=labels,
                                font_size=15,
                                font_color='whitesmoke',
                                font_family='serif')

        # Edges
        chains = [edge[2] for edge in edges]
        for chain in range(max(chains)+1):
            chain_edges = [(edge[0], edge[1])
                           for edge in edges if edge[2] == chain]
            G.add_edges_from(chain_edges)

            chain_color = self.chain_colors[chain % len(self.chain_colors)]
            nx.draw_networkx_edges(G,
                                   pos=self.pos_chimera,
                                   width=3,
                                   style='solid',
                                   edge_color=chain_color)

            G.remove_edges_from(chain_edges)

    def draw_chimera_and_embedding(self, nodes, edges, mapping_G_to_H):
        self.draw_chimera_graph(3, 3, 4)
        self.draw_embedding(nodes, edges, mapping_G_to_H)

    def show_embedding(self):
        plt.show()

    ############################ Embedding step ################################

    def draw_whole_embedding_step(self, nodes, edges, mapping_G_to_H, title=''):
        ax = self.construct_subplot_to_the_right()
        ax.set_title(title)
        self.draw_chimera_and_embedding(nodes, edges, mapping_G_to_H)
        self.total_steps += 1

    def construct_subplot_to_the_right(self) -> Axes:
        """Constructs a new subplot to the right."""
        self.col += 1
        gs = gridspec.GridSpec(1, self.col)

        # Reposition subplots
        for i, ax in enumerate(self.fig.axes):
            # ax.set_position(gs[i, 0].get_position(self.fig))
            ax.set_subplotspec(gs[0, i])

        # Add new subplot
        ax = self.fig.add_subplot(gs[0, self.col-1])
        ax.set_position(gs[0, self.col-1].get_position(self.fig))

        return ax

    def save_and_clear(self, path):
        self.fig.set_size_inches(self.total_steps*1000*self.px, 800*self.px)
        self.fig.savefig(path, bbox_inches='tight')
        plt.clf()
