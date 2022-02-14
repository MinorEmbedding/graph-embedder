from src.drawing.draw import Draw
from src.graph.test_graph import TestGraph
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.solver.embedding_solver import EmbeddingSolver


def main():
    print('--- Main ---')

    # --- Setup
    d = Draw()
    H = TestGraph.k4()

    # --- Start solving
    while True:
        solver = EmbeddingSolver(H)
        solver.init_dfs()

        found_embedding = solver.found_embedding()
        if found_embedding:
            print('🎉 Directly found embedding after initialization')
            output_embedding(*solver.get_embedding(), d)
            return
        # after_init_embedding = solver.get_embedding()
        # output_embedding(*after_init_embedding, d)

        # --- Mutation
        failed_mutation_count = 0
        playground = None
        while failed_mutation_count < 10:
            playground = solver.mutate()
            if playground:
                break
            else:
                print('Not a viable mutation... Trying again')
                failed_mutation_count += 1

        if not playground:
            print('Could not achieve a viable mutation, even after 10 trials')
            return

        if playground.is_valid_embedding():
            output_embedding(*playground.get_embedding(), d)
            return


def output_embedding(nodes, edges, mapping_G_to_H, d: Draw):
    print()
    print('--- Output ---')
    d.draw_chimera_graph(3, 3, 4)
    print('*** Final embedding ***')
    print(nodes)
    print(edges)
    print(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)


if __name__ == "__main__":
    main()
