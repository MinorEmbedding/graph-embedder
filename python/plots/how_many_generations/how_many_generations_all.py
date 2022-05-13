from collections import namedtuple

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

names = ['nikolaus', 'k5', 'k6', 'k7', 'k8', 'k9']


# --- Prepare data
Data = namedtuple('Data', ['unique', 'counts'])
datas = []
for name in names:
    with open(f'./data/how_many_generations_{name}.txt', 'r') as f:
        data = f.read().splitlines()
        data = [int(value) for value in data]
        unique, counts = np.unique(data, return_counts=True)
        datas.append(Data(unique, counts))


# --- Plot setup
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Helvetica"
})

# https://stackoverflow.com/a/39566040/9655481
MEDIUM_SIZE = 8
MEDIUM_SIZE = 14
BIGGER_SIZE = 16

plt.rc('font', size=MEDIUM_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=MEDIUM_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('axes', titlesize=BIGGER_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
# plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title


# --- Plot
rows = 2
cols = 3
fig, axs = plt.subplots(rows, cols, figsize=(14, 9))

i = 0
for r in range(rows):
    for c in range(cols):
        ax = axs[r, c]

        if i == len(datas):
            break

        # color = '#2981B3'
        color = '#E53054'
        ax.bar(datas[i][0], datas[i].counts, color=color)
        # ax.axhline(0, color='grey', linewidth=0.8)
        ax.set_title(names[i])

        major = 5 if max(datas[i].unique) <= 60 else 10
        ax.xaxis.set_major_locator(ticker.MultipleLocator(major))

        minor = 1 if max(datas[i].unique) <= 60 else 5
        ax.xaxis.set_minor_locator(ticker.MultipleLocator(minor))

        i += 1

# Label
axs[1, 0].set_ylabel('Occurrences')
axs[1, 0].set_xlabel('Generations needed')

# ax.set_title('Generations needed for the crossed house puzzle')
plt.subplots_adjust(left=0.05, top=0.96, right=0.98, bottom=0.06, hspace=0.2, wspace=0.1)
# plt.tight_layout(pad=0.2, w_pad=0.1, h_pad=0.1, rect=(0.05, 0.05, 0.95, 0.95))
plt.show()
