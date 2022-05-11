def change_brightness(color, amount=1):
    """
    Lightens/Darkens the given color by multiplying luminosity by the given amount.
    Input can be matplotlib color string, hex string, or RGB tuple.

    Examples:
    >> change_brightness('g', 0.3)
    >> change_brightness('#F034A3', 0.6)
    >> change_brightness((.3,.55,.1), 0.5)
    """
    # adapted from
    # https://gist.github.com/ihincks/6a420b599f43fcd7dbd79d56798c4e5a
    # https://stackoverflow.com/a/49601444/9655481

    import colorsys

    import matplotlib.colors as mc
    import numpy as np

    try:
        c = mc.cnames[color]
    except:
        c = color

    c = np.array(colorsys.rgb_to_hls(*mc.to_rgb(c)))
    hls = (c[0], max(0, min(1, amount * c[1])), c[2])
    rgb = colorsys.hls_to_rgb(*hls)
    return mc.to_hex(rgb)
