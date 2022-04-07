import logging
import os
import shutil
from random import random
from typing import Optional

from src.drawing.draw import DrawEmbedding
from src.embedding.embedding import Embedding
from src.graph.test_graph import TestGraph
from src.solver.embedding_solver import EmbeddingSolver, EvolutionParams
from src.util.logging import init_logger

init_logger()
logger = logging.getLogger('evolution')


################################# Params #######################################

params = EvolutionParams(
    population_size=7,
    max_mutation_trials=30,
    mutation_trials_until_extend_to_free_neighbors=15
)

max_total = 1
max_generations = 300
remove_redundant_nodes_probability = 0.1


############################### Evolution ######################################

def main_loop():
    for i in range(max_total):
        logger.info('')
        logger.info('#############')
        logger.info('🎈 NEW MAIN 🎈')
        logger.info('#############')
        logger.info('')
        logger.info(f'Calling main: {i}')

        d = DrawEmbedding(5, 5, 4)
        res = main(d)
        save_final(d)
        if res:
            break


def main(d: DrawEmbedding) -> bool:
    # logger.info('--- Main ---')

    # --- Clear
    # Clear out directory
    try:
        shutil.rmtree('./out/')
    except FileNotFoundError:
        pass
    os.mkdir('./out')

    # --- Setup
    H = TestGraph.k(12)

    solver = EmbeddingSolver(H)
    solver.initialize_embedding()
    save_embedding(*solver.get_embedding(), d, -1,
                   title=f'Initial embedding')

    if solver.found_embedding():
        logger.info('🎉 Directly found embedding after initialization')
        # save_final(d)
        output_embedding(*solver.get_embedding(), d)
        return True

    # --- Start solver
    for i in range(max_generations):
        child = do_one_generation(i, solver)

        if not child:
            logger.info('🔳 Stopping algorithm...')
            return False

        solver.commit(child)
        save_embedding(*solver.get_embedding(), d, i,
                       title=f'Generation {i}')
        # Check if done
        if child.is_valid_embedding():
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
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
    if random() < remove_redundant_nodes_probability:
        logger.info('Try to remove redundant supernode nodes')
        child.remove_redundant_supernode_nodes()
        # best_mutation.remove_unnecessary_edges_between_supernodes()

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
                   mapping_G_to_H, d: DrawEmbedding, i: int, title=''):
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
    main_loop()
