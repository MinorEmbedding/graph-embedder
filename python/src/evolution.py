import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from dwave_networkx.drawing.chimera_layout import draw_chimera

from chimera_graph_view import ChimeraGraphView


def define_minor():
    # in this case: Haus vom Nikolaus

    h_nodes = [0, 1, 2, 3, 4]
    h_edges = [(0, 1), (0, 4), (1, 2), (1, 3), (2, 3), (3, 4), (4, 0)]

    h = nx.Graph()
    h.add_nodes_from(h_nodes)
    h.add_edges_from(h_edges)

    pos = {
        0: (0, 0),
        1: (0, 2),
        2: (1, 3),
        3: (2, 2),
        4: (2, 0)
    }
    return h, pos


def main():
    print('Started program')

    # --- Init minor H
    h, pos = define_minor()
    # nx.draw_networkx(h, pos=pos)

    # --- Init Chimera graph G
    g = dnx.chimera_graph(1, 1, 4)
    dnx.draw_chimera(g)

    g_view = ChimeraGraphView(len(h.nodes()))
    g_view.init_basic_graph()

    nodes = g_view.chimera.get_nodes()
    edges = g_view.chimera.get_edges()
    print(nodes)
    print(edges)

    # --- Draw
    g_view_draw = nx.Graph()
    g_view_draw.add_nodes_from(nodes)
    g_view_draw.add_edges_from(edges)
    pos = {
        0: (0., -0.5),
        1: (0.25, -0.5),
        2: (0.75, -0.5),
        3: (1, -0.5),
        4: (0.5, 0.),
        5: (0.5, -0.25),
        6: (0.5, -0.75),
        7: (0.5, -1.)
    }
    nx.draw_networkx(g_view_draw, pos=pos, node_color='b', node_shape='*',
                     style='dashed', edge_color='b', width=3)

    # g_view_dnx = dnx.chimera_graph(1, 1, 4,
    #                                node_list=nodes,
    #                                edge_list=edges)
    # dnx.draw_chimera(g_view_dnx, node_color='b', node_shape='*',
    #                  style='dashed', edge_color='b', width=3)

    plt.show()

    # TODO: Implement evolution


if __name__ == "__main__":
    main()
