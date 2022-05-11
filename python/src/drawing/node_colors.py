import random

supernode_colors = set(['#55C1D9', '#F29E38', '#F23827', '#D748F5', '#39DBC8',
                        '#F5428A', '#3CDE73', '#11F0EB', '#7D2EFF', '#DBDE5D',
                        '#3A2CE0', '#DE6E31', '#E0165C'])

remember_colors = dict()
used_colors = set()


def get_supernode_color(node_H: int):
    global used_colors

    if node_H in remember_colors:
        return remember_colors[node_H]

    # Choose random color
    color = random.choice(list(supernode_colors - used_colors))  # "left-over" colors
    remember_colors[node_H] = color
    used_colors.add(color)
    if len(used_colors) == len(supernode_colors):
        used_colors = []  # reset used colors when every color was used once

    return color
