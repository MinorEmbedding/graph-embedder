import logging
import random
from typing import Optional

from src.embedding.embedding import Embedding, NoFreeNeighborNodes
from src.util.util import any_of_one_in_other, get_first_from

logger = logging.getLogger('evolution')


class SupernodeExtension():

    def __init__(self, embedding: Embedding) -> None:
        self._embedding = embedding
        self._non_viable_extensions = []

    def _choose_random_embedded_node(self) -> Optional[int]:
        embedded_nodes = self._embedding.get_embedded_nodes()
        if not embedded_nodes:
            logger.info('❌ No nodes embedded yet')
            return None
        source = random.choice(list(embedded_nodes))
        return source

    def extend_random_supernode_to_free_neighbors(self) -> None:
        logger.info('Trying to extend random supernode to free neighbors')

        embedded_nodes = self._embedding.get_embedded_nodes()
        if not embedded_nodes:
            logger.info('❌ No nodes embedded yet')
            return

        max_trials = 10
        tried_nodes = set()
        for _ in range(min(len(embedded_nodes), max_trials)):
            source = random.choice(list(embedded_nodes - tried_nodes))

            # Check free neighbors
            try:
                free_neighbors = self._embedding.get_free_neighbors(source)
            except:
                tried_nodes.add(source)
                continue

            target = random.choice(list(free_neighbors))
            self._embedding.construct_supernode(source, target)
            logger.info(f'-> Extended node {source} to {target}')

            return

        logger.info('-> ❌ failed')

    def extend_random_supernode(self):
        """Randomly merges two nodes into one super node or extends existing
        super nodes by merging another node into it. This process might also
        create up to two new super nodes."""
        source = self._choose_random_embedded_node()
        if not source:
            return None

        # Choose target
        targets = self._embedding.get_embedded_neighbors(source)
        target = random.choice(list(targets))

        logger.info(f'🔗🔗 Trying to construct supernode: {source}, {target}')

        # Avoid unnecessary calculations
        if (source, target) in self._non_viable_extensions:
            logger.info('❌ Already considered but not viable -> skip')
            return None

        # Get free neighbors
        try:
            target_free_neighbors = self._embedding.get_free_neighbors(target)
        except NoFreeNeighborNodes:
            logger.info(f'❌ Target {target} has no free neighbors')
            return None

        playground = self._embedding.get_playground()

        # Both nodes in same supernode?
        if (playground.get_supernode(source) == playground.get_supernode(target)):
            logger.info(f'🔗 Source ({source}) and target ({target}) node '
                        + 'are in same supernode -> Easy chain extension')
            shifted_target = random.choice(list(target_free_neighbors))
            playground.construct_supernode(target, shifted_target)
            return playground

        # Adjust so that new super node placement is viable
        target_neighbors = self._embedding.get_embedded_neighbors(target)
        # no need to reach source from shifted_target
        target_neighbors.discard(source)
        logger.info(f'Target neighbors are: {target_neighbors}')

        # --- Try out possible positions for shifted_target
        # Always check if we can reach all super nodes previously connected
        # to target from the shifted_target
        for shifted_target in target_free_neighbors:
            logger.info(f'▶ Try out shifted target on node: {shifted_target}')
            res = self._construct_supernode_with_shifted_target(playground, source, target,
                                                                shifted_target, target_neighbors)
            if res:
                return res

        # If it didn't work, mark the mutation as failed
        self._non_viable_extensions.append((source, target))
        logger.info('❌ No viable supernode placement')
        return None

    def get_reachable_node_in_neighbor_supernode(self, playground: Embedding, ignore_neighbors: set[int],
                                                 reachable: set[int], neighbor: int) -> Optional[int]:
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

    def _construct_supernode_with_shifted_target(self, playground: Embedding, source: int, target: int,
                                                 shifted_target: int, target_neighbors: set[int]) -> Optional[Embedding]:
        """Tries to embed the shifted target, so that the node placement is viable.

        This means that we check if the new place for ``target`` - which is
        ``shifted_target`` - allows for the following edges:
            - ``target`` - ``shifted_target``
            - ``shifted_target`` - ``target_neighbors`` (all embedded ``target`` neighbors)
        """
        playground = self._embedding.get_playground()

        target_original_supernode = playground.get_supernode(target)

        # Get reachable neighbors
        # has to be calculated prior to supernode handover (!)
        shifted_target_reachable = \
            playground.get_reachable_neighbors(shifted_target)
        shifted_target_reachable.discard(source)  # no need to check this
        shifted_target_reachable.discard(target)  # no need to check this

        # supernode handover from target to shifted_target
        playground.construct_supernode(target, shifted_target)
        playground.construct_supernode(source, target)

        if not target_neighbors:
            logger.info(f'No target neighbors to check: {target_neighbors}')
            return playground

        # --- Check that previous connections still work
        # --- Try with one chain first
        neighbors_done = set()
        for neighbor in target_neighbors:
            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, {shifted_target}, shifted_target_reachable, neighbor)

            if target_neighbor != None:
                logger.info(f'Could reach {target_neighbor}')
                neighbors_done.add(neighbor)
                playground.embed_edge(shifted_target, target_neighbor)
            else:
                logger.info(f'Could not reach '
                            f'(reachable nodes are: {shifted_target_reachable})')
                # Continue normally with next neighbor as next one might be
                # reachable with this strategy (we then try to reach the
                # remaining neighbors with the next strategy down below)

        # Check if this strategy was successfull
        if neighbors_done == target_neighbors:
            if playground.check_supernode_connectiveness(target_original_supernode):
                logger.info(f'Worked with 1st strategy (seldom !)')
                return playground
            else:
                logger.info(f'Supernode sanity not ensured.')
                return None

        # --- Try out to construct another chain to see if we can reach
        # remaining neighbors
        try:
            shifted_target_free_neighbors = playground.get_free_neighbors(shifted_target)
        except NoFreeNeighborNodes:
            logger.info(f'Could not construct another supernode '
                        f'from shifted_target: {shifted_target} '
                        f'(no free neighbor of shifted_target)')
            return None
        shifted_target_partner = get_first_from(shifted_target_free_neighbors)

        logger.info(f'🔗 Trying to construct another supernode: '
                    f'{shifted_target}, {shifted_target_partner}')
        # Chain shifted target and shifted target partner
        # prior to calling this, we need to make sure that shifted_target
        # now has the prior supernode of target
        # The call itself needs to take place prior to embedding edges outgoing
        # from shifted_target_partner (otherwise shifted_target_partner would
        # not have a viable supernode)
        playground.construct_supernode(shifted_target, shifted_target_partner)

        # Do not try to compute shifted_target reachable again here (!)
        # as these are different now since shifted_target has the previous
        # supernode of target
        shifted_target_partner_reachable = \
            playground.get_reachable_neighbors(shifted_target_partner)
        shifted_target_partner_reachable.discard(source)  # no need to check this
        shifted_target_partner_reachable.discard(target)  # no need to check this

        for neighbor in target_neighbors:
            if neighbor in neighbors_done:
                continue

            target_neighbor = self.get_reachable_node_in_neighbor_supernode(
                playground, {shifted_target, shifted_target_partner}, shifted_target_partner_reachable, neighbor)

            if target_neighbor != None:
                logger.info(f'Could reach (with 2nd chain) {target_neighbor}')
                playground.embed_edge(shifted_target_partner, target_neighbor)
            else:
                logger.info(f'Could also not reach (reachable nodes are: '
                            f'{shifted_target_partner_reachable})')
                return None  # did not achieve a viable mutation even with 2nd strategy

        if playground.check_supernode_connectiveness(target_original_supernode):
            return playground
        else:
            logger.info(f'Supernode sanity not ensured.')
            return None
