import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from src.drawing.color_util import change_brightness
from src.drawing.node_colors import get_supernode_color

default_embedded_color = '#6B6B6A'


class DrawEmbedding():

    def init_figure(self):
        self.fig = plt.figure()
        plt.subplots_adjust(left=0.0, right=1.0,
                            bottom=0.0, top=1.0,
                            wspace=0.1, hspace=0.1)

    def __init__(self, m, n, t):
        self.total_steps = 0

        # https://matplotlib.org/stable/gallery/subplots_axes_and_figures/figure_size_units.html
        self.px = 1/plt.rcParams['figure.dpi']  # pixel in inches
        plt.rcParams.update({
            'axes.titlesize': 22,
            'text.usetex': False,
            'font.family': 'Helvetica'
        })

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
                         node_color='#C8C8C8',
                         edge_color='#C8C8C8')

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
        for edge in edges:
            node1 = edge[0]
            node2 = edge[1]

            # Color edges inside a supernode with the same color
            # and between supernodes with the first color
            node1_H = mapping_G_to_H[node1]
            node2_H = mapping_G_to_H[node2]

            # Nodes
            self.draw_node(node1, get_supernode_color(node1_H))
            self.draw_node(node2, get_supernode_color(node2_H))

            # Edges
            G.add_edge(node1, node2)
            chain_color = get_supernode_color(node1_H)\
                if node1_H == node2_H else default_embedded_color
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
        self.fig.suptitle(title, fontsize=16)
        self.draw_chimera_and_embedding(nodes, edges, mapping_G_to_H)
        self.total_steps += 1

    def save_and_clear(self, path):
        self.fig.set_size_inches(self.m*3, self.n*3)
        self.fig.savefig(path, bbox_inches='tight')
        plt.close(self.fig)
