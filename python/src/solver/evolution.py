import logging
import os
import shutil

from src.drawing.draw import DrawEmbedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver
from src.util.logging import init_logger

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

solver_iterations = 15
mutation_trials = 30
max_total = 1


############################### Evolution ######################################

def main_loop():
    i = 0
    while i < max_total:
        logger.info('')
        logger.info('#############')
        logger.info('🎈 NEW MAIN 🎈')
        logger.info('#############')
        logger.info('')
        logger.info(f'Calling main: {i}')

        res = main()
        if res:
            break
        i += 1


def main() -> bool:
    # logger.info('--- Main ---')

    # --- Clear
    # Clear out directory
    try:
        shutil.rmtree('./out/')
    except FileNotFoundError:
        pass
    os.mkdir('./out/')

    # --- Setup
    d = DrawEmbedding()
    H = TestGraph.k(4)

    solver = EmbeddingSolver(H)
    solver.init_dfs()
    solver.local_maximum()
    save_embedding(*solver.get_embedding(), d, -1,
                   title=f'Initial embedding')

    if solver.found_embedding():
        logger.info('🎉 Directly found embedding after initialization')
        save_final(d)
        output_embedding(*solver.get_embedding(), d)
        return True

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
                f'🔴 Not a viable mutation after '
                f'{mutation_trials} trials (in solver iteration: {i})')
            save_final(d)
            return False

        solver.commit(playground)
        # save_embedding(*solver.get_embedding(), d, i,
        #                title=f'{i}: Mutation')

        # Local maximum
        solver.local_maximum()
        save_embedding(*solver.get_embedding(), d, i,
                       title=f'{i}: Local maximum')

        if playground.is_valid_embedding():
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
            save_final(d)
            return True
        else:
            logger.info('✅ Mutation succeeded')

        i += 1

    save_final(d)
    return False


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: DrawEmbedding):
    logger.info('*** Embedding ***')
    logger.info(nodes)
    logger.info(edges)
    logger.info(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)
    d.show_embedding()


def save_embedding(nodes, edges, mapping_G_to_H, d: DrawEmbedding, i: int, title=''):
    d.draw_whole_embedding_step(nodes, edges, mapping_G_to_H, title=title)
    # d.draw_embedding(nodes, edges, mapping_G_to_H)
    # d.save_and_clear(f'./out/{i}.svg')


def save_final(d: DrawEmbedding) -> None:
    d.save_and_clear(f'./out/steps.svg')

################################ Main ##########################################


if __name__ == "__main__":
    main_loop()
