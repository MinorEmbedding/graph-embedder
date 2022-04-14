import logging
import random
from collections import namedtuple
from dataclasses import dataclass
from math import ceil
from typing import Optional

import numpy as np
from src.embedding.articulation_point import ArticulationPointCalculator
from src.embedding.embedding import Embedding, NoFreeNeighborNodes
from src.util.util import any_of_one_in_other, get_first_from

SourceTargetPair = namedtuple('SourceTargetPair', 'source target')
logger = logging.getLogger('evolution')


@dataclass
class TargetParams():
    target: int
    target_supernode: int
    shifted_target: int
    target_neighbors: set[int]


class SupernodeExtension():

    def __init__(self, embedding: Embedding) -> None:
        self._embedding = embedding
        self._non_viable_extensions = []
        self.selection_chances = []

    def extend_random_supernode_to_free_neighbors(self) -> Optional[Embedding]:
        """Chooses a random supernode and extends it to a free neighbor."""
        logger.info('Trying to extend random supernode to free neighbors')
        playground = self._embedding.get_playground()

        supernode = self._choose_random_supernode_with_bias()
        supernode_nodes = self._embedding.get_nodes_in_supernode(supernode)

        max_trials = 10
        tried_nodes = set()
        for _ in range(min(len(supernode_nodes), max_trials)):
            source = random.choice(list(supernode_nodes - tried_nodes))

            try:
                free_neighbors = playground.get_free_neighbors(source)
            except:
                tried_nodes.add(source)
                continue

            target = random.choice(list(free_neighbors))
            playground.construct_supernode(source, target)
            logger.info(f'-> Extended node {source} (supernode: {supernode}) to {target}')
            return playground

        logger.info(f'-> ❌ failed (for supernode {supernode})')
        return None

    def _choose_random_supernode_with_bias(self) -> int:
        """Randomly chooses a supernode from H such that supernodes, which have
        the least number of edges to other supernodes embedded in G, are favored.
        We also favor supernodes that have the smallest number of nodes.
        Thus, we won't pick an element according to a uniform distribution, but to
        a custom one (using numpy).
        """
        supernodes = list(self._embedding.get_nodes_H())

        if not len(self.selection_chances):
            degree_percentages = self._embedding.get_supernode_degree_percentages()

            # Favor smaller supernodes
            # Those get assigned an increased chance of being selected
            max_favor_small_supernodes = 0.7 * max(degree_percentages.values())
            lin_increases = np.linspace(
                max_favor_small_supernodes, 0, num=len(supernodes))
            supernodes_sorted = self._embedding.get_sorted_supernodes_by_size()
            # we use the supernodes as indices for the linspace of increases here
            increases = [lin_increases[node] for node in supernodes_sorted]

            # Calculate selection chances
            selection_chances_without_increase = np.array(
                [(1-p) for p in degree_percentages.values()])
            selection_chances = np.array([(1-p) + increases[i]
                                          for i, p in enumerate(degree_percentages.values())])
            selection_chances_without_increase *= 1 / \
                np.sum(selection_chances_without_increase)
            selection_chances *= 1 / np.sum(selection_chances)
            np.set_printoptions(precision=2)
            logger.info(
                f'Selection chances (without increases) are: {selection_chances_without_increase}')
            logger.info(f'Selection chances are: {selection_chances}')
            self.selection_chances = selection_chances

        # Choose according to calculated chances
        supernode = np.random.choice(supernodes, p=self.selection_chances)
        return supernode

    def extend_random_supernode(self) -> Optional[Embedding]:
        """Chooses a random supernode and extends it to an already embedded
        neighbor node by "bumping" the neighbor to one of its free neighbors.

        When the embedded neighbor is bumped to a free neighbor, this algorithm
        might construct another supernode for the neighbor in order to reach
        every node that could previously be reached from the neighbor node."""
        pair = self._choose_source_and_target()
        if not pair:
            return None
        source, target = pair.source, pair.target
        logger.info(f'🔗🔗 Trying to construct: {source}, {target}')
        if (source, target) in self._non_viable_extensions:
            logger.info('❌ Already considered but not viable -> skip')
            # TODO: Don't count this case as wasted mutation
            return None

        try:
            target_free_neighbors = self._embedding.get_free_neighbors(target)
        except NoFreeNeighborNodes:
            logger.info(f'❌ Target {target} has no free neighbors')
            return None

        playground = self._embedding.get_playground()

        # --- Try out possible positions for shifted_target
        # Always check if we can reach all super nodes previously connected
        # to target from the shifted_target
        target_neighbors = playground.get_embedded_neighbors(target)
        target_neighbors.discard(source)  # no need to reach source from shifted_target
        target_supernode = playground.get_supernode(target)
        logger.info(f'Target neighbors are: {target_neighbors}')

        for shifted_target in target_free_neighbors:
            logger.info(f'▶ Try out shifted target on node: {shifted_target}')
            p = TargetParams(target, target_supernode, shifted_target, target_neighbors)
            res = self._construct_supernode_with_shifted_target(playground, source, p)
            if res:
                return res

        # If it didn't work, mark the mutation as failed
        self._non_viable_extensions.append((source, target))
        logger.info('❌ No viable supernode placement')
        return None

    def _choose_random_embedded_node(self) -> int:
        embedded_nodes = self._embedding.get_embedded_nodes()
        node = random.choice(list(embedded_nodes))
        return node

    def _construct_supernode_with_shifted_target(self,
                                                 playground: Embedding,
                                                 source: int,
                                                 p: TargetParams) -> Optional[Embedding]:
        """Tries to embed the shifted target, so that the node placement is viable.

        This means that we check if the new place for ``target`` - which is
        ``shifted_target`` - allows for the following edges:
            - ``target`` - ``shifted_target``
            - ``shifted_target`` - ``target_neighbors`` (all embedded ``target`` neighbors)
        """
        # Get reachable neighbors
        # has to be calculated prior to supernode handover (!)
        shifted_target_reachable = playground.get_reachable_neighbors(p.shifted_target)
        shifted_target_reachable.discard(source)
        shifted_target_reachable.discard(p.target)

        # supernode handover from target to shifted_target
        playground.construct_supernode(p.target, p.shifted_target)
        playground.construct_supernode(source, p.target)

        if not p.target_neighbors:
            logger.info(f'No target neighbors to check: {p.target_neighbors}')
            return playground

        # Check that previous connections still work
        res, neighbors_done = self._construct_with_one_chain(
            playground, p, shifted_target_reachable)
        if not res:
            res = self._construct_with_another_chain(
                playground, source, p, neighbors_done)
        if not res:
            return None

        # Check if strategy really worked
        if res.check_supernode_connectiveness(p.target_supernode):
            return res
        else:
            logger.info(f'Supernode sanity not ensured.')
            return None

    def _construct_with_one_chain(self, playground: Embedding, p: TargetParams,
                                  shifted_target_reachable: set[int]) -> tuple[Optional[Embedding], set[int]]:
        """Tries to construct the supernode extension with just one chain."""
        neighbors_done = set()

        for neighbor in p.target_neighbors:
            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, neighbor, {p.shifted_target}, shifted_target_reachable)

            if target_neighbor != None:  # might be node "0"
                logger.info(f'Could reach {target_neighbor}')
                neighbors_done.add(neighbor)
                playground.embed_edge(p.shifted_target, target_neighbor)
            else:
                logger.info(f'Could not reach '
                            f'(reachable nodes are: {shifted_target_reachable})')
                # Continue normally with next neighbor as next one might be
                # reachable with this strategy (we then try to reach the
                # remaining neighbors with the next strategy down below)

        return playground, neighbors_done

    def _construct_with_another_chain(self, playground: Embedding, source: int,
                                      p: TargetParams, neighbors_done: set[int]) -> Optional[Embedding]:
        """Tries to construct another chain to see if we can reach
        remaining neighbors."""
        try:
            shifted_target_free_neighbors = playground.get_free_neighbors(
                p.shifted_target)
        except NoFreeNeighborNodes:
            logger.info(f'Could not construct another supernode '
                        f'from shifted_target: {p.shifted_target} '
                        f'(no free neighbor of shifted_target)')
            return None
        shifted_target_partner = get_first_from(shifted_target_free_neighbors)

        logger.info(f'🔗 Trying to construct another supernode: '
                    f'{p.shifted_target}, {shifted_target_partner}')
        # Chain shifted target and shifted target partner
        # prior to calling this, we need to make sure that shifted_target
        # now has the prior supernode of target
        # The call itself needs to take place prior to embedding edges outgoing
        # from shifted_target_partner (otherwise shifted_target_partner would
        # not have a viable supernode)
        playground.construct_supernode(p.shifted_target, shifted_target_partner)

        # Do not try to compute shifted_target reachable again here (!)
        # as these are different now since shifted_target has the previous
        # supernode of target
        shifted_target_partner_reachable = playground.get_reachable_neighbors(
            shifted_target_partner)
        shifted_target_partner_reachable.discard(source)
        shifted_target_partner_reachable.discard(p.target)

        for neighbor in p.target_neighbors:
            # Already checked neighbors with the one-chain strategy
            if neighbor in neighbors_done:
                continue

            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, neighbor, {p.shifted_target, shifted_target_partner},
                shifted_target_partner_reachable)

            if target_neighbor != None:
                logger.info(f'Could reach (with 2nd chain) {target_neighbor}')
                playground.embed_edge(shifted_target_partner, target_neighbor)
            else:
                logger.info(f'Could also not reach (reachable nodes are: '
                            f'{shifted_target_partner_reachable})')
                return None  # did not achieve a viable mutation even with 2nd strategy

        return playground

    def get_reachable_node_in_neighbor_supernode(self, playground: Embedding,
                                                 neighbor: int, ignore_neighbors: set[int],
                                                 reachable: set[int]) -> Optional[int]:
        """We need to discard some nodes here as they are not actually part of the
        neighbor supernode. This is because we construct supernodes first to be
        able to embed edges correctly. However, we then need to compare with
        the a priori state and not this temporary state, which is why we
        have to discard some nodes."""
        neighbor_supernode = playground.get_supernode(neighbor)
        neighbor_supernode_nodes = playground.get_nodes_in_supernode(neighbor_supernode)
        for ignore in ignore_neighbors:
            neighbor_supernode_nodes.discard(ignore)

        logger.info(f'? Can we reach neighbor {neighbor} '
                    f'(supernode: {neighbor_supernode} -> {neighbor_supernode_nodes})')
        reachable_neighbor = any_of_one_in_other(reachable, neighbor_supernode_nodes)
        return reachable_neighbor

    def _choose_source_and_target(self) -> Optional[SourceTargetPair]:
        supernode = self._choose_random_supernode_with_bias()
        supernode_nodes = self._embedding.get_nodes_in_supernode(supernode)

        max_trials = 5
        for _ in range(max_trials):
            source = random.choice(list(supernode_nodes))
            target = self._choose_neighbor_not_in_same_supernode(source, supernode)
            if target:
                return SourceTargetPair(source, target)

        logger.info(f'❌ Could not find a valid source/target pair')
        return None

    def _choose_neighbor_not_in_same_supernode(self, source: int,
                                               source_supernode: int) -> Optional[int]:
        """Tries to randomly choose a neighbor (target) for the given source node
        that is not in the same supernode as the source node AND that is not
        an articulation point in its own supernode.
        Returns nothing if this condition cannot be met for the given source node
        and its neighbors."""
        targets_orig = self._embedding.get_embedded_neighbors(source)
        targets = []
        for target in targets_orig:
            # Is target in the same supernode as source?
            if self._embedding.get_supernode(target) == source_supernode:
                continue

            # Is target an articulation point?
            target_supernode_nodes = self._embedding.get_nodes_in_supernode_of(target)
            articulation_points = ArticulationPointCalculator(self._embedding.G_embedding)\
                .calc_articulation_points(target_supernode_nodes)
            if target in articulation_points:
                continue

            targets.append(target)

        if not targets:
            return None

        target = random.choice(list(targets))
        return target
