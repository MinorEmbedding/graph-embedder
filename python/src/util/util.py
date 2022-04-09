from typing import Optional


def get_first_from(s: set[int]) -> int:
    # https://stackoverflow.com/a/48874729/9655481
    for e in s:
        return e
    raise KeyError('Set is empty')


def any_of_one_in_other(set1: set[int], set2: set[int]) -> Optional[int]:
    # https://stackoverflow.com/a/16138094/9655481
    for element in set1:
        if element in set2:
            return element
    return None
