# Move this file to src/solver to start

import logging
import multiprocessing
import time
from functools import partial
from random import random
from typing import Optional

from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger
from tqdm import tqdm

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

params = EvolutionParams(
    population_size=12,
    max_mutation_trials=30,
    mutation_extend_to_free_neighbors_probability=0.3  # should be <=0.5
)

max_total = 100
max_generations = 100
remove_redundancy_probability = 0.01

# Chimera graph
t = 4  # shore size


############################### Evolution ######################################

def different_grid_sizes():
    for m in range(9, 10):
        print(f'Started with grid size {m}x{m}')

        graph = TestGraph.k(8)
        params = [graph, m]

        start_time = time.time()
        start_multiprocessing(params, 'k8_different_grid_sizes')
        duration = time.time() - start_time
        print(f'Duration for grid size {m}x{m}: {duration} s')


def start_multiprocessing(params, name: str):
    with multiprocessing.Pool() as pool:
        # Multiprocessing
        res = list(tqdm(
            pool.imap_unordered(partial(do_once, params), range(max_total)),
            total=max_total)
        )

        # Save to file
        with open(f'./out/how_many_generations_{params[1]}x{params[1]}_{name}.txt', 'w') as f:
            for generations_needed in res:
                f.write(str(generations_needed) + '\n')


def do_once(params, j) -> int:
    solver = EmbeddingSolver(params[0], params[1], params[1], t)

    # --- Init
    solver.initialize_embedding()

    if solver.found_embedding():
        print('🎉 Directly found embedding after initialization')
        return 0

    # --- Start solver
    for i in range(max_generations):
        child = do_one_generation(i, solver)

        if not child:
            logger.info('🔳 Stopping algorithm...')
            return -1

        solver.commit(child)

        # Check if done
        if child.is_valid_embedding():
            child.remove_redundancy()
            return i+1
        else:
            logger.info('✅ Generation passed')

    return -1


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


################################ Main ##########################################


if __name__ == "__main__":
    different_grid_sizes()
