supernode_colors = [
    '#FF4A6D', '#2EB4FF', '#FF8C36', '#2BD8B7', '#AE1CFF',
    '#2BC2D6', '#DBDE5D', '#11F0EB', '#7D2EFF', '#2B76EF',
    '#E0165C', '#3CDE73', '#F23827', '#F29E38', '#D33EDB',
    '#0EA54E'
]

remember_colors = dict()
i = 0

def get_supernode_color(node_H: int):
    global i

    if node_H in remember_colors:
        return remember_colors[node_H]

    # Choose color
    color = supernode_colors[i % len(supernode_colors)]
    i += 1

    return color
