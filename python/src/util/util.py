from typing import Optional


def get_first_from_set(s: set[int]) -> Optional[int]:
    # https://stackoverflow.com/a/48874729/9655481
    for e in s:
        return e
    return None
