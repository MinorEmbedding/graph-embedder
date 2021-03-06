import logging
import os
import shutil
import signal
from random import random
from typing import Optional

from src.drawing.draw import DrawEmbedding
from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.results.degree_percentage import DegreePercentageData
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger

init_logger()
logger = logging.getLogger('evolution')

stop = False


################################# Params #######################################

params = EvolutionParams(
    population_size=12,
    max_mutation_trials=30,
    mutation_extend_to_free_neighbors_probability=0.3  # should be <=0.5
)

max_total = 1
max_generations = 1000
remove_redundancy_probability = 0.01

# Chimera graph
m = 2  # grid size
n = 2  # grid size
t = 4  # shore size


################################  Setup ########################################

H = TestGraph.crossed_house()
solver = EmbeddingSolver(H, m, n, t)
dp = DegreePercentageData(len(H.get_nodes()))


############################### Evolution ######################################

def main_loop():
    for i in range(max_total):
        logger.info('')
        logger.info('#############')
        logger.info('🎈 NEW MAIN 🎈')
        logger.info('#############')
        logger.info('')
        logger.info(f'Calling main: {i}')

        d = DrawEmbedding(m, n, t)
        res = main(d)
        save_final(d)
        if res:
            break
    dp.plot_blocking()


def signal_handler(sig, frame):
    # https://stackoverflow.com/questions/1112343/how-do-i-capture-sigint-in-python
    global stop
    stop = True


def main(d: DrawEmbedding) -> bool:
    # logger.info('--- Main ---')

    # --- Clear
    try:
        shutil.rmtree('./out/')
    except FileNotFoundError:
        pass
    os.mkdir('./out')

    # --- Init
    solver.initialize_embedding()
    save_embedding(*solver.get_embedding(), d, 'initial',
                   title=f'Initial embedding')
    dp.save_current_degree_percentages(-1, solver._embedding)

    if solver.found_embedding():
        logger.info('🎉 Directly found embedding after initialization')
        print('🎉 Directly found embedding after initialization')
        # save_final(d)
        output_embedding(*solver.get_embedding(), d)
        return True

    # --- Start solver
    for i in range(max_generations):
        if stop:
            print('Stop spawning new generations')
            break

        child = do_one_generation(i, solver)

        if not child:
            logger.info('🔳 Stopping algorithm...')
            return False

        solver.commit(child)

        # Save embedding every x steps
        save_embedding_steps = 1
        if (i % save_embedding_steps == 0) \
                or (i == max_generations - 2) or (i == max_generations - 1):
            save_embedding(*solver.get_embedding(), d, str(i),
                           title=f'Generation {i}')

        # Save degree percentage data
        dp.save_current_degree_percentages(i, solver._embedding)

        # Check if done
        if child.is_valid_embedding():
            save_embedding(*solver.get_embedding(), d, str(i),
                           title=f'Generation {i} (final)')
            child.remove_redundancy()
            save_embedding(*solver.get_embedding(), d, f'{i}final',
                           title=f'Generation {i} (final with redundancy removed)')
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
            print('🎉🎉🎉🎉🎉🎉 Found embedding')
            return True
        else:
            logger.info('✅ Generation passed')

    return False


def do_one_generation(i: int, solver: EmbeddingSolver) -> Optional[Embedding]:
    logger.info('')
    logger.info(f'🔄 Generation: {i}')

    child = solver.generate_population_and_select(params)
    if not child:
        return None

    # Leave "room" on graph for next generation
    if random() < remove_redundancy_probability:
        child.remove_redundancy()

    return child


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: DrawEmbedding):
    logger.info('*** Embedding ***')
    logger.info(nodes)
    logger.info(edges)
    logger.info(mapping_G_to_H)

    d.draw_chimera_and_embedding(nodes, edges, mapping_G_to_H)
    d.show_embedding()


def save_embedding(nodes: set[int], edges: set[tuple[int, int, int]],
                   mapping_G_to_H, d: DrawEmbedding, i: str, title=''):
    logger.info('')
    logger.info('🎈 Current embedding')
    logger.info(f'edges: {edges}')
    logger.info(f'mapping_G_to_H: {mapping_G_to_H}')

    d.draw_whole_embedding_step(nodes, edges, mapping_G_to_H, title=title)
    d.save_and_clear(f'./out/{i}.svg')


def save_final(d: DrawEmbedding) -> None:
    logger.info('Save final')
    pass
    # d.save_and_clear(f'./out/steps.svg')

    ################################ Main ##########################################


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    main_loop()
