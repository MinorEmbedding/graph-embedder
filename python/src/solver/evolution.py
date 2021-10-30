from src.drawing.draw import Draw
from src.graphs.undirected_graphs import UndirectedGraphAdjList
from src.solver.embedding_solver import EmbeddingSolver


def init_H():
    # K4 graph
    H = UndirectedGraphAdjList(4)
    H._set_edge(0, 1)
    H._set_edge(0, 2)
    H._set_edge(0, 3)
    H._set_edge(1, 2)
    H._set_edge(1, 3)
    H._set_edge(2, 3)
    return H


def main():
    print('--- Main ---')

    # --- Setup
    d = Draw()
    H = init_H()
    d.draw_chimera_graph(1, 1, 4)  # one unit cell of Chimera graph

    # --- Start solving
    solver = EmbeddingSolver(H)
    solver.init_basic_path()
    found_embedding = solver.found_embedding()
    if found_embedding:
        print('✨🎉 Found embedding')
        return

    # --- Mutation
    playground = solver.mutate()
    if not playground:
        print('Not a viable mutation')
        return

    # --- Output
    nodes, edges = playground.get_embedding()
    mapping = playground.get_mapping()

    print('*** Final mapping ***')
    print(mapping)
    print('*** Final embedding ***')
    print(nodes)
    print(edges)
    print(f'Is correct: {playground.is_valid_embedding()}')

    d.draw_embedding(nodes, edges)


if __name__ == "__main__":
    main()
