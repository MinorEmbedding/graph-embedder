import dwave_networkx as dnx
import matplotlib.pyplot as plt
import networkx as nx
from dwave_networkx.drawing.chimera_layout import draw_chimera

from src.embedding_solver import EmbeddingSolver
from src.graphs.undirected_graphs import UndirectedGraphAdjList


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
    print('--- Main() ---')

    # --- Init minor H
    # h, pos = define_minor()
    # nx.draw_networkx(h, pos=pos)

    # Define minor H
    H = UndirectedGraphAdjList(5)
    H._set_edge(0, 1)
    H._set_edge(1, 2)
    H._set_edge(1, 3)
    H._set_edge(2, 3)
    H._set_edge(3, 4)
    H._set_edge(4, 0)

    # --- Draw one unit cell of a Chimera graph
    g = dnx.chimera_graph(1, 1, 4)
    dnx.draw_chimera(g)

    ############################################################################
    solver = EmbeddingSolver(H)
    solver.init_basic_path()

    # --- Epochs
    while True:
        # --- Mutations
        # Strategy1: Pick the best mutation from 5
        # Strategy2 (for now): Take the first mutation that yields better costs
        while True:
            mutation = solver.mutate()
            cost = solver.calculate_cost()
            break  # go on with next epoch

        # Draw
        nodes, edges = solver.get_embedding()
        draw_embedding(nodes, edges)

    ############################################################################


def draw_embedding(nodes, edges):
    g_view_draw = nx.Graph()
    g_view_draw.add_nodes_from(nodes)
    edges = [(edge[0], edge[1])
             for edge in edges]  # strip chains for right now
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
    # pos = dict()
    # for node in nodes:
    #     pos[node] = pos_all[node]
    # print(pos)

    nx.draw_networkx(g_view_draw, pos=pos, node_color='b', node_shape='*',
                     style='dashed', edge_color='b', width=3)

    # g_view_dnx = dnx.chimera_graph(1, 1, 4,
    #                                node_list=nodes,
    #                                edge_list=edges)
    # dnx.draw_chimera(g_view_dnx, node_color='b', node_shape='*',
    #                  style='dashed', edge_color='b', width=3)

    plt.show()


if __name__ == "__main__":
    main()
