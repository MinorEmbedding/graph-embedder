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

max_total = 1
max_mutations_trials = 12
population_size = 5
max_generations = 20


############################### Evolution ######################################

def main_loop():
    for i in range(max_total):
        logger.info('')
        logger.info('#############')
        logger.info('🎈 NEW MAIN 🎈')
        logger.info('#############')
        logger.info('')
        logger.info(f'Calling main: {i}')

        d = DrawEmbedding(3, 3, 4)
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
    os.mkdir('./out/')

    # --- Setup
    H = TestGraph.k(6)

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
    for i in range(max_generations):
        logger.info('')
        logger.info(f'🔄 Generation: {i}')

        # Generate children for one population
        population = []  # list of Embeddings
        for _ in range(population_size):
            logger.info('')
            logger.info(f'--- Try find a new viable mutation')
            mutation = None

            for mutation in range(max_mutations_trials):
                # Do one mutation
                mutation = solver.mutate()
                if mutation:
                    population.append(mutation)
                    break  # try to construct next child

            if not mutation:
                if not len(population):
                    logger.info(f'🔳 All {max_mutations_trials} mutations failed, '
                                'could not construct a single child -> Abort')
                    return False
                logger.info(f'🔳 Mutation failed, will continue '
                            f'with smaller population: {len(population)}/{population_size}')
                break  # since it is improbable that we will be able to generate more children

        # Choose best child
        improvements = [mutation.try_embed_missing_edges() for mutation in population]
        best_mutation_index = improvements.index(max(improvements))
        best_mutation = population[best_mutation_index]

        # save_embedding(*best_mutation.get_embedding(G_to_H_mapping=True), d, i,
        #                title=f'Generation {i} (before remove)')

        best_mutation.remove_unnecessary_edges_between_supernodes()
        solver.commit(best_mutation)
        save_embedding(*solver.get_embedding(), d, i,
                       title=f'Generation {i}')

        # Check if we found valid embedding
        if best_mutation.is_valid_embedding():
            logger.info('🎉🎉🎉🎉🎉🎉 Found embedding')
            return True
        else:
            logger.info('✅ Generation passed')

    return False


################################ Output ########################################

def output_embedding(nodes, edges, mapping_G_to_H, d: DrawEmbedding):
    logger.info('*** Embedding ***')
    logger.info(nodes)
    logger.info(edges)
    logger.info(mapping_G_to_H)

    d.draw_embedding(nodes, edges, mapping_G_to_H)
    d.show_embedding()


def save_embedding(nodes: set[int], edges: set[tuple[int, int, int]],
                   mapping_G_to_H, d: DrawEmbedding, i: int, title=''):
    logger.info('')
    logger.info('🎈 Current embedding')
    logger.info(f'edges: {edges}')
    logger.info(f'mapping_G_to_H: {mapping_G_to_H}')

    d.draw_whole_embedding_step(nodes, edges, mapping_G_to_H, title=title)
    # d.draw_embedding(nodes, edges, mapping_G_to_H)
    # d.save_and_clear(f'./out/{i}.svg')


def save_final(d: DrawEmbedding) -> None:
    d.save_and_clear(f'./out/steps.svg')

################################ Main ##########################################


if __name__ == "__main__":
    main_loop()
