from src.drawing.draw import Draw
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.solver.embedding_solver import EmbeddingSolver


def init_H():
    # K4 graph
    H = UndirectedGraphAdjList(4)
    H.set_edge(0, 1)
    H.set_edge(0, 2)
    H.set_edge(0, 3)
    H.set_edge(1, 2)
    H.set_edge(1, 3)
    H.set_edge(2, 3)
    return H


def main():
    print('--- Main ---')

    # --- Setup
    d = Draw()
    H = init_H()
    d.draw_chimera_graph(1, 1, 4)  # one unit cell of Chimera graph

    # --- Start solving
    row_count = 12

    d.init_big_plot(row_count)
    found_embeddings = set()

    i = 0
    while True:
        print('====================')
        print(len(found_embeddings))

        if len(found_embeddings) == row_count*row_count:
            print(f'found {len(found_embeddings)} unique embeddings')
            break

        # --- Solve & Mutate
        solver = EmbeddingSolver(H)
        solver.init_basic_path()
        playground = solver.mutate()
        if not playground:
            print('Not a viable mutation')
            return

        # --- Output
        nodes, edges, mapping_G_to_H = playground.get_embedding()

        length1 = len(found_embeddings)
        found_embeddings.add(frozenset(edges))
        length2 = len(found_embeddings)

        if length1 == length2:
            print('already seen')
            continue

        print(f'{i}: Is correct: {playground.is_valid_embedding()}')
        d.draw_to_big_plot(nodes, edges, mapping_G_to_H)

        i += 1

    d.save_big_plot('k4_embedding')


if __name__ == "__main__":
    main()
