import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib import gridspec
from matplotlib.axes import Axes


class DrawEmbedding():

    def __init__(self):
        self.total_steps = 0

        self.supernode_default_color = '#55C1D9'
        self.supernode_colors = ['#F29E38', '#F23827', '#D748F5', '#39DBC8',
                                 '#F5428A', '#3CDE73', '#11F0EB', '#E9B952',
                                 '#7D2EFF', '#DBDE5D', '#3A2CE0', '#DE6E31',
                                 '#E0165C']

        self.remember_colors = dict()

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

        Args:
            nodes (set[int]): nodes
            edges (tuple[int, int, int]): tuple (node1, node2, chain)
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
        for i, edge in enumerate(edges):
            node1 = edge[0]
            node2 = edge[1]

            # Color edges inside a supernode with the same color
            # and between supernodes with the first color
            node1_H = mapping_G_to_H[node1]
            node2_H = mapping_G_to_H[node2]
            print(f'💨 Edge {node1}-{node2} (in H: {node1_H}-{node2_H})')
            if node1_H != node2_H:
                chain_color = self.supernode_default_color
            else:
                try:
                    chain_color = self.remember_colors[node1_H]
                except KeyError:
                    chain_color = self.supernode_colors[i % len(self.supernode_colors)]
                    self.remember_colors[node1_H] = chain_color

            G.add_edge(node1, node2)
            nx.draw_networkx_edges(G,
                                   pos=self.pos_chimera,
                                   width=3,
                                   style='solid',
                                   edge_color=chain_color)
            G.remove_edge(node1, node2)

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
