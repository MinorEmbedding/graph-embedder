# Move this file to src/solver to start
# This is a file used for docker with environment variables

import logging
import multiprocessing
import os
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

max_total = int(os.getenv('MAX_TOTAL', 1000))
max_generations = 100
remove_redundancy_probability = 0.01

# Chimera graph
m = int(os.getenv('GRID_M', 5))  # grid size
n = int(os.getenv('GRID_N', 5))  # grid size
t = 4  # shore size


############################### Evolution ######################################

def do_all_k_graphs():
    for i in range(int(os.getenv('K_MIN', 5)), int(os.getenv('K_MAX', 10)) + 1):
        print(f'Started k{i}')
        start_time = time.time()
        start_multiprocessing(TestGraph.k(i), f'k{i}')
        duration = time.time() - start_time
        print(f'Duration for k{i}: {duration} s')


def start_multiprocessing(H: UndirectedGraphAdjList, name: str):
    processes = multiprocessing.cpu_count() * int(os.getenv('CORE_PERCENTAGE', 50)) // 100
    with multiprocessing.Pool(processes) as pool:
        # Multiprocessing
        res = list(tqdm(
            pool.imap_unordered(partial(do_once, H), range(max_total)),
            total=max_total)
        )

        # Save to file
        with open(f'./out/how_many_generations_{m}x{n}_{name}.txt', 'w') as f:
            for generations_needed in res:
                f.write(str(generations_needed) + '\n')


def do_once(H: UndirectedGraphAdjList, i) -> int:
    solver = EmbeddingSolver(H, m, n, t)

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
    do_all_k_graphs()
