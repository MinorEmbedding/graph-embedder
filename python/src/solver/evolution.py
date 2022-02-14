import os
import shutil

from src.drawing.draw import Draw
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver

################################# Params #######################################

solver_iterations = 10
mutation_trials = 100


############################### Evolution ######################################

def main_loop():
    while True:
        try:
            res = main()
            if res:
                break
        except:
            pass


def main() -> bool:
    print('--- Main ---')

    # --- Clear
    # Clear out directory
    # try:
    #     shutil.rmtree('./out/')
    # except FileNotFoundError:
    #     pass
    # os.mkdir('./out/')

    # --- Setup
    d = Draw()
    H = TestGraph.k(6)

    solver = EmbeddingSolver(H)
    solver.init_dfs()

    if solver.found_embedding():
        print('🎉 Directly found embedding after initialization')
        output_embedding(*solver.get_embedding(), d)
        return False

    # --- Start solver
    i = 0
    while i < solver_iterations:
        print()
        print(f'🔄 New solver iteration: {i}')
        print()

        # output_embedding(*solver.get_embedding(), d)
        # save_embedding(*solver.get_embedding(), d, i)

        # --- Mutation
        mutation_count = 0
        playground = None

        while mutation_count < mutation_trials:
            playground = solver.mutate()
            if playground:
                break
            else:
                mutation_count += 1

        if not playground:
            print(
                f'❌ Could not achieve a viable mutation, even after {mutation_trials} trials')
            print(f'In Mutation: {i}')
            return False

        if playground.is_valid_embedding():
            print('🎉🎉🎉🎉🎉🎉 Found embedding')
            output_embedding(*playground.get_embedding(), d)
            return True
        else:
            print('✅ Mutation succeeded, but is not yet a valid embedding')
            solver.commit(playground)

        i += 1

    return False


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: Draw):
    print()
    print('--- Output ---')
    d.draw_chimera_graph(3, 3, 4)
    print('*** Embedding ***')
    print(nodes)
    print(edges)
    print(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)


def save_embedding(nodes, edges, mapping_G_to_H, d: Draw, i: int):
    d.draw_chimera_graph(3, 3, 4)
    d._draw(nodes, edges, mapping_G_to_H)
    d.save_and_clear(f'./out/{i}.svg')


################################ Main ##########################################

if __name__ == "__main__":
    main_loop()
