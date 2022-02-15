import logging
import os
import shutil

from src.drawing.draw import Draw
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver
from src.util.logging import init_logger

init_logger()
logger = logging.getLogger('evolution')


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
    # logger.info('--- Main ---')

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
    save_embedding(*solver.get_embedding(), d, -1)

    if solver.found_embedding():
        logger.info('🎉 Directly found embedding after initialization')
        output_embedding(*solver.get_embedding(), d)
        return False

    # --- Start solver
    i = 0
    while i < solver_iterations:
        logger.info('')
        logger.info(f'🔄 New solver iteration: {i}')

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
            logger.info(
                f'❌ Not a viable mutation even after {mutation_trials} trials (in solver iteration: {i}')
            return False

        if playground.is_valid_embedding():
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
            output_embedding(*playground.get_embedding(), d)
            return True
        else:
            logger.info(
                '✅ Mutation succeeded, but is not yet a valid embedding')
            solver.commit(playground)
            save_embedding(*solver.get_embedding(), d, i)

        i += 1

    return False


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: Draw):
    d.draw_chimera_graph(3, 3, 4)
    logger.info('*** Embedding ***')
    logger.info(nodes)
    logger.info(edges)
    logger.info(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)


def save_embedding(nodes, edges, mapping_G_to_H, d: Draw, i: int):
    d.draw_chimera_graph(3, 3, 4)
    d._draw(nodes, edges, mapping_G_to_H)
    d.save_and_clear(f'./out/{i}.svg')


################################ Main ##########################################

if __name__ == "__main__":
    main_loop()
