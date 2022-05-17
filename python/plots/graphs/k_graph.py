import matplotlib.pyplot as plt
import networkx as nx


def draw_k_graph(n: int):
    G = nx.complete_graph(n)
    pos = nx.circular_layout(G)

    fig, ax = plt.subplots(figsize=(5, 5))
    plt.tight_layout()

    # nx.draw(G)
    nx.draw_networkx_nodes(G,
                           pos=pos,
                           node_size=500,
                           node_color='#5B5B5B',
                           linewidths=3,
                           edgecolors='#858585')
    nx.draw_networkx_edges(G,
                           pos=pos,
                           width=2,
                           style='solid',
                           edge_color='#5B5B5B')

    plt.savefig(f'./out/k{i}.pdf', bbox_inches='tight', pad_inches=0)


for i in range(1, 41):
    draw_k_graph(i)
