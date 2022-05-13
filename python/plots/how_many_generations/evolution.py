import logging
from random import random
from typing import Optional

from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger
from tqdm import tqdm

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

# Params used to generate "how_many_generations_nikolaus_10000"
# population_size=12
# max_mutation_trials=30
# mutation_extend_to_free_neighbors_probability=0.3
# max_total = 10000
# max_generations = 100
# remove_redundancy_probability = 0.01
# m = 2  # grid size
# n = 2  # grid size
# t = 4  # shore size

params = EvolutionParams(
    population_size=12,
    max_mutation_trials=30,
    mutation_extend_to_free_neighbors_probability=0.3  # should be <=0.5
)

max_total = 10000
max_generations = 100
remove_redundancy_probability = 0.01

# Chimera graph
m = 2  # grid size
n = 2  # grid size
t = 4  # shore size


################################  Setup ########################################

H = TestGraph.crossed_house()


############################### Evolution ######################################

def main_loop():
    with open('./out/how_many_generations2.txt', 'w') as f:
        for i in tqdm(range(max_total)):
            logger.info('')
            logger.info('#############')
            logger.info('🎈 NEW MAIN 🎈')
            logger.info('#############')
            logger.info('')
            logger.info(f'Calling main: {i}')

            generations_needed = main()
            f.write(str(generations_needed) + '\n')


def main() -> int:
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
    main_loop()
