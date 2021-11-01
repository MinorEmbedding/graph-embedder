from src.drawing.draw import Draw
from src.graphs.undirected_graphs import UndirectedGraphAdjList
from src.solver.embedding_solver import EmbeddingSolver


def init_H():
    # I letter graph
    # H = UndirectedGraphAdjList(6)
    # H._set_edge(0, 3)
    # H._set_edge(3, 2)
    # H._set_edge(3, 5)
    # H._set_edge(1, 2)
    # H._set_edge(2, 4)

    # K4 graph
    H = UndirectedGraphAdjList(4)
    H._set_edge(0, 1)
    H._set_edge(0, 2)
    H._set_edge(0, 3)
    H._set_edge(1, 2)
    H._set_edge(1, 3)
    H._set_edge(2, 3)

    # Pyramid graph
    # H = UndirectedGraphAdjList(5)
    # H._set_edge(0, 1)
    # H._set_edge(0, 2)
    # H._set_edge(0, 3)
    # H._set_edge(0, 4)
    # H._set_edge(1, 2)
    # H._set_edge(2, 3)
    # H._set_edge(3, 4)

    # Tree graph
    # H = UndirectedGraphAdjList(6)
    # H._set_edge(0, 1)
    # H._set_edge(0, 2)
    # H._set_edge(0, 3)
    # H._set_edge(1, 4)
    # H._set_edge(1, 5)
    # H._set_edge(2, 5)  # comment this line out to get a tree

    # K5 graph (for later use with multiple chimera cells)
    # H = UndirectedGraphAdjList(5)
    # H._set_edge(0, 1)
    # H._set_edge(0, 2)
    # H._set_edge(0, 3)
    # H._set_edge(0, 4)
    # H._set_edge(1, 2)
    # H._set_edge(1, 3)
    # H._set_edge(1, 4)
    # H._set_edge(2, 3)
    # H._set_edge(2, 4)
    # H._set_edge(3, 4)

    return H


def main():
    print('--- Main ---')

    # --- Setup
    d = Draw()
    H = init_H()
    d.draw_chimera_graph(1, 1, 4)  # one unit cell of Chimera graph

    # --- Start solving
    while True:
        solver = EmbeddingSolver(H)
        solver.init_dfs()

        found_embedding = solver.found_embedding()
        if found_embedding:
            print('🎉 Directly found embedding after initialization')
            output_embedding(*solver.get_embedding(), d)
            return

        # --- Mutation
        playground = solver.mutate()
        if not playground:
            print('Not a viable mutation')
            return

        if playground.is_valid_embedding():
            output_embedding(*playground.get_embedding(), d)
            return


def output_embedding(nodes, edges, mapping_G_to_H, d: Draw):
    print()
    print('--- Output ---')

    print('*** Final mapping ***')
    print(mapping_G_to_H)
    print('*** Final embedding ***')
    print(nodes)
    print(edges)
    print(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)


if __name__ == "__main__":
    main()
