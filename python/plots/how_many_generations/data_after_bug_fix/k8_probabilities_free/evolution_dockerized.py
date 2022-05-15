# Move this file to src/solver to start
# Disable logger!

import logging
import multiprocessing
import os
import shutil
import time
from functools import partial
from random import random
from typing import Optional

import numpy as np
from src.drawing.draw import DrawEmbedding
from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger
from tqdm import tqdm

init_logger()
logger = logging.getLogger('evolution')

################################# Params #######################################

max_total = int(os.getenv('MAX_TOTAL', 300))
max_generations = int(os.getenv('MAX_GENERATIONS', 600))
remove_redundancy_probability = float(os.getenv('PROB_REMOVE_REDUNDANCY', 0.01))

# Chimera graph
m = int(os.getenv('GRID_M', 5))  # grid size
n = int(os.getenv('GRID_N', 5))  # grid size
t = 4  # shore size


############################### Evolution ######################################
# d = DrawEmbedding(m, n, t)


def different_params():
    # # --- Clear
    # try:
    #     shutil.rmtree('./out/')
    # except FileNotFoundError:
    #     pass
    # os.mkdir('./out')

    prob_steps = int(os.getenv('PROB_STEPS', 20))
    for prob in np.linspace(0.0, 1.0, num=prob_steps):
        prob = round(prob, 2)
        print(f'Started probability {prob}')
        graph_number = int(os.getenv('K_GRAPH', 8))
        graph = TestGraph.k(graph_number)
        start_time = time.time()
        start_multiprocessing((graph, prob), f'k{graph_number}prob{prob}')
        duration = time.time() - start_time
        print(f'Duration for probability {prob}: {duration} s')


def start_multiprocessing(plot_params, name: str):
    processes = multiprocessing.cpu_count() * int(os.getenv('CORE_PERCENTAGE', 75)) // 100
    with multiprocessing.Pool(processes) as pool:
        # Multiprocessing
        res = list(tqdm(
            pool.imap_unordered(partial(do_once, plot_params), range(max_total)),
            total=max_total)
        )

        # Save to file
        with open(f'./out/howManyGenerations_{m}x{n}_{max_total}_{max_generations}gen_{name}.txt', 'w') as f:
            for generations_needed in res:
                f.write(str(generations_needed) + '\n')


def do_once(plot_params, j) -> int:
    solver = EmbeddingSolver(plot_params[0], m, n, t)

    # --- Init
    solver.initialize_embedding()

    if solver.found_embedding():
        print('🎉 Directly found embedding after initialization')
        return 0

    # --- Start solver
    for i in range(max_generations):
        child = do_one_generation(i, solver, plot_params)

        if not child:
            logger.info('🔳 Stopping algorithm...')
            return -1

        solver.commit(child)

        # Check if done
        if child.is_valid_embedding():
            child.remove_redundancy()
            # save_embedding(*solver.get_embedding(), d, f'{j}-{i}final', plot_params,
            #                title=f'Generation {i} (final with redundancy removed)')
            return i+1
        else:
            logger.info('✅ Generation passed')

    return -1


def do_one_generation(i: int, solver: EmbeddingSolver, plot_params) -> Optional[Embedding]:
    logger.info('')
    logger.info(f'🔄 Generation: {i}')

    evo_params = EvolutionParams(
        population_size=int(os.getenv('POPSIZE', 6)),
        max_mutation_trials=int(os.getenv('MAX_MUTATION_TRIALS', 30)),
        mutation_extend_to_free_neighbors_probability=plot_params[1]
    )

    child = solver.generate_population_and_select(evo_params)
    if not child:
        return None

    # Leave "room" on graph for next generation
    if random() < remove_redundancy_probability:
        child.remove_redundancy()

    return child


################################ Main ##########################################

# def save_embedding(nodes: set[int], edges: set[tuple[int, int, int]],
#                    mapping_G_to_H, d: DrawEmbedding, name: str, plot_params, title=''):
#     logger.info('')
#     logger.info('🎈 Current embedding')
#     logger.info(f'edges: {edges}')
#     logger.info(f'mapping_G_to_H: {mapping_G_to_H}')

#     d.draw_whole_embedding_step(nodes, edges, mapping_G_to_H, title=title)

#     folder_path = f'./out/popsize{plot_params[1]}'
#     if not os.path.exists(folder_path):
#         os.mkdir(folder_path)
#     d.save_and_clear(os.path.join(folder_path, f'{name}.svg'))


if __name__ == "__main__":
    different_params()
