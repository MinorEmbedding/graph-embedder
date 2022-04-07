import logging
import random
from collections import namedtuple
from dataclasses import dataclass
from typing import Optional

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

    def reset(self) -> None:
        self._non_viable_extensions = []

    def extend_random_supernode_to_free_neighbors(self) -> Optional[Embedding]:
        """Chooses a random supernode an extends it to a free neighbor."""
        logger.info('Trying to extend random supernode to free neighbors')
        playground = self._embedding.get_playground()
        embedded_nodes = playground.get_embedded_nodes()

        max_trials = 10
        tried_nodes = set()
        for _ in range(min(len(embedded_nodes), max_trials)):
            source = random.choice(list(embedded_nodes - tried_nodes))

            try:
                free_neighbors = playground.get_free_neighbors(source)
            except:
                tried_nodes.add(source)
                continue

            target = random.choice(list(free_neighbors))
            playground.construct_supernode(source, target)
            logger.info(f'-> Extended node {source} to {target}')
            return playground

        logger.info('-> ❌ failed')
        return None

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
        shifted_target_reachable = \
            playground.get_reachable_neighbors(p.shifted_target)
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
        shifted_target_partner_reachable = \
            playground.get_reachable_neighbors(shifted_target_partner)
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
        for _ in range(5):
            pair = self._choose_source_and_target_not_in_same_supernode()
            if pair:
                return pair
        logger.info(f'Could not find valid source/target pair')
        return None

    def _choose_source_and_target_not_in_same_supernode(self) -> Optional[SourceTargetPair]:
        source = self._choose_random_embedded_node()
        source_supernode = self._embedding.get_supernode(source)

        targets = self._embedding.get_embedded_neighbors(source)
        targets = [t for t in targets
                   if self._embedding.get_supernode(t) != source_supernode]
        if not targets:
            return None

        target = random.choice(list(targets))
        return SourceTargetPair(source, target)
