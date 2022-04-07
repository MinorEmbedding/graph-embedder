import logging
from dataclasses import dataclass
from typing import Optional

from src.embedding.embedding import Embedding, NoFreeNeighborNodes
from src.graph.undirected_graph import UndirectedGraphAdjList
from src.solver.initialization import Initialization
from src.solver.supernode_extension import SupernodeExtension
from src.util.stack import Stack
from src.util.util import any_of_one_in_other, get_first_from

logger = logging.getLogger('evolution')


@dataclass
class EvolutionParams():
    population_size: int
    max_mutation_trials: int
    mutation_trials_until_extend_to_free_neighbors: int


class EmbeddingSolver():

    def __init__(self, H: UndirectedGraphAdjList):
        self.H = H
        if H.nodes_count < 2:
            raise NameError('The minor to embed must have at least two nodes')

        self._embedding = Embedding(H)
        self.non_viable_mutations = []
        self.initialization = Initialization(self._embedding)
        self._supernode_extension = SupernodeExtension(self._embedding)

    def initialize_embedding(self):
        self.initialization.init_dfs()
        self._local_maximum()

    def get_embedding(self):
        return self._embedding.get_embedding(G_to_H_mapping=True)

    def commit(self, playground: Embedding):
        self._embedding = playground
        self.reset()

    def reset(self):
        self.non_viable_mutations = []

    def generate_population_and_select(self, params: EvolutionParams) -> Optional[Embedding]:
        """Generates a new population & selects and returns the best individual
        from it."""
        population = self._generate_children(params)
        if not population:
            logger.info(f'🔳 Population generation failed')
            return None
        elif len(population) < params.population_size:
            logger.info(f'🔳 {params.max_mutation_trials} mutations to construct '
                        ' a new child failed, will return a smaller population: '
                        f'{len(population)}/{params.population_size}')

        selected_population = self._select_best_child(population)
        return selected_population

        # TODO: "before all fails" strategy
        # Before all fails: try to remove unnecessary supernode nodes
        # and try once more
        # self._embedding.remove_redundant_supernode_nodes()
        # logger.info(f'🔳 Last trial, remove redundant nodes')

    def _generate_children(self, params: EvolutionParams) -> list[Embedding]:
        """Generates children (Embeddings) for one population."""
        population = []  # list of Embeddings

        for _ in range(params.population_size):
            child = self._generate_child(params)
            if child:
                population.append(child)
            else:
                # early return as it is unlikely that we will be able
                # to generate more children
                return population

        return population

    def _generate_child(self, params: EvolutionParams) -> Optional[Embedding]:
        logger.info('')
        logger.info(f'--- Try find a new viable mutation')
        mutation = None

        for trial in range(params.max_mutation_trials):
            # Do one mutation
            logger.info('--- MUTATION')
            mutation = self._supernode_extension.extend_random_supernode()
            if mutation:
                return mutation
            elif trial >= params.mutation_trials_until_extend_to_free_neighbors:
                self._supernode_extension.extend_random_supernode_to_free_neighbors()

        logger.info(f'🔳 All {params.max_mutation_trials} mutations failed, '
                    'could not construct a child -> Abort')
        return None

    def _select_best_child(self, population: list[Embedding]):
        # Try to optimize to local maximum first
        improvements = [child.try_embed_missing_edges() for child in population]
        best_child_index = improvements.index(max(improvements))
        return population[best_child_index]

    def _local_maximum(self) -> int:
        return self._embedding.try_embed_missing_edges()

    def found_embedding(self) -> bool:
        return self._embedding.is_valid_embedding()
