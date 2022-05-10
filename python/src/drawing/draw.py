import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib import gridspec
from matplotlib.axes import Axes


class DrawEmbedding():

    def init_figure(self):
        self.fig = plt.figure()
        plt.subplots_adjust(left=0.0, right=1.0,
                            bottom=0.0, top=1.0,
                            wspace=0.1, hspace=0.1)

    def __init__(self, m, n, t):
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

        self.col = 0
        self.m = m
        self.n = n

        self.init_chimera_graph(m, n, t)

    def init_chimera_graph(self, m, n, t):
        self.chimera_G = dnx.chimera_graph(m, n, t)
        self.pos_chimera = dnx.chimera_layout(self.chimera_G)

    def draw_chimera_graph(self):
        """Draws a Chimera graph."""

        # Nodes & Edges
        nx.draw_networkx(self.chimera_G,
                         pos=self.pos_chimera,
                         width=2,
                         with_labels=False,
                         node_color='#858585',
                         edge_color='#BABABA')

        # Labels
        # Shift labels
        pos_labels = {node: [pos[0] - 0.020, pos[1]]
                      for (node, pos) in self.pos_chimera.items()}
        nx.draw_networkx_labels(self.chimera_G,
                                pos=pos_labels,
                                font_size=8,
                                font_color='#858585')

    def draw_embedding(self, nodes: set[int], edges: set[tuple[int, int, int]],
                       mapping_G_to_H):
        """Draws the embedding onto the Chimera graph.

        Args:
            nodes (set[int]): nodes
            edges (set[tuple[int, int, int]]): set of tuples (node1, node2, chain)
        """
        labels = {}
        for node in nodes:
            labels[node] = mapping_G_to_H[node]

        G = nx.Graph()
        G.add_nodes_from(nodes)

        # Labels
        nx.draw_networkx_labels(G,
                                pos=self.pos_chimera,
                                labels=labels,
                                font_size=14,
                                font_color='whitesmoke',
                                font_family='serif')

        # Nodes & Edges
        remember_nodes_in_chain = []
        for i, edge in enumerate(edges):
            node1 = edge[0]
            node2 = edge[1]

            # Color edges inside a supernode with the same color
            # and between supernodes with the first color
            node1_H = mapping_G_to_H[node1]
            node2_H = mapping_G_to_H[node2]
            if node1_H != node2_H:
                chain_color = self.supernode_default_color
            else:
                try:
                    chain_color = self.remember_colors[node1_H]
                except KeyError:
                    chain_color = self.supernode_colors[i % len(self.supernode_colors)]
                    self.remember_colors[node1_H] = chain_color

            # Nodes
            if node1_H == node2_H:
                # Draw over nodes when they are in the same chain
                self.draw_node(node1, chain_color)
                remember_nodes_in_chain.append(node1)
                self.draw_node(node2, chain_color)
                remember_nodes_in_chain.append(node2)
            else:
                # Don't draw over nodes if they are not in the same chain now
                # but have been
                if not node1 in remember_nodes_in_chain:
                    self.draw_node(node1, self.supernode_default_color)
                if not node2 in remember_nodes_in_chain:
                    self.draw_node(node2, self.supernode_default_color)

            # Edges
            G.add_edge(node1, node2)
            nx.draw_networkx_edges(G,
                                   pos=self.pos_chimera,
                                   width=3,
                                   style='solid',
                                   edge_color=chain_color)
            G.remove_edge(node1, node2)

    def draw_node(self, node: int, color):
        G = nx.Graph()
        G.add_node(node)
        nx.draw_networkx_nodes(G,
                               pos=self.pos_chimera,
                               node_color=change_brightness(color, amount=0.8),
                               linewidths='2',
                               edgecolors=color)

    def draw_chimera_and_embedding(self, nodes: set[int], edges: set[tuple[int, int, int]],
                                   mapping_G_to_H):
        self.draw_chimera_graph()
        self.draw_embedding(nodes, edges, mapping_G_to_H)

    def show_embedding(self):
        plt.show()

    ############################ Embedding step ################################

    def draw_whole_embedding_step(self, nodes: set[int], edges: set[tuple[int, int, int]],
                                  mapping_G_to_H, title=''):
        self.init_figure()
        self.fig.suptitle(title, fontsize=20)
        self.draw_chimera_and_embedding(nodes, edges, mapping_G_to_H)
        self.total_steps += 1

    # def construct_subplot_to_the_right(self) -> Axes:
    #     """Constructs a new subplot to the right."""
    #     self.col += 1
    #     gs = gridspec.GridSpec(1, self.col)

    #     # Reposition subplots
    #     for i, ax in enumerate(self.fig.axes):
    #         # ax.set_position(gs[i, 0].get_position(self.fig))
    #         ax.set_subplotspec(gs[0, i])

    #     # Add new subplot
    #     ax = self.fig.add_subplot(gs[0, self.col-1])
    #     ax.set_position(gs[0, self.col-1].get_position(self.fig))

    #     return ax

    def save_and_clear(self, path):
        self.fig.set_size_inches(self.m*3, self.n*3)
        self.fig.savefig(path, bbox_inches='tight')
        plt.close(self.fig)


def change_brightness(color, amount=1):
    """
    Lightens the given color by multiplying luminosity by the given amount.
    Input can be matplotlib color string, hex string, or RGB tuple.

    Examples:
    >> lighten_color('g', 0.3)
    >> lighten_color('#F034A3', 0.6)
    >> lighten_color((.3,.55,.1), 0.5)
    """
    # adapted from
    # https://gist.github.com/ihincks/6a420b599f43fcd7dbd79d56798c4e5a
    # https://stackoverflow.com/a/49601444/9655481

    import colorsys

    import matplotlib.colors as mc
    import numpy as np

    try:
        c = mc.cnames[color]
    except:
        c = color

    c = np.array(colorsys.rgb_to_hls(*mc.to_rgb(c)))
    hls = (c[0], max(0, min(1, amount * c[1])), c[2])
    rgb = colorsys.hls_to_rgb(*hls)
    return mc.to_hex(rgb)
