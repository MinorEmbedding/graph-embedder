from collections import namedtuple

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

names = [str(i) for i in range(2, 10)]


# --- Prepare data
Data = namedtuple('Data', ['unique', 'counts'])
datas = []
for name in names:
    # filename = f'./data/population_size_100_runs/how_many_generations_5x5_100_600_max_gen_k8_population_size_{name}.txt'
    filename = f'./data_after_bug_fix/k_graphs/howManyGenerations_2x2_200_600gen_6popsize_k{name}.txt'
    with open(filename, 'r') as f:
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
cols = 4
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
        # ax.set_title(f'population size {names[i]}')
        ax.set_title(f'$K_{names[i]}$')

        # Same scale for most subfigures
        # maximum = max(datas[i].unique)
        # print(maximum)
        # if maximum < 100:
        #     ax.set_xlim(-5, 100)

        ax.set_xlim(left=-5)

        # major = 5 if max(datas[i].unique) <= 60 else 10
        # ax.xaxis.set_major_locator(ticker.MultipleLocator(major))

        # minor = 1 if max(datas[i].unique) <= 60 else 5
        # ax.xaxis.set_minor_locator(ticker.MultipleLocator(minor))

        # https://stackoverflow.com/a/38096332/9655481
        ax.yaxis.get_major_locator().set_params(integer=True)
        ax.xaxis.set_minor_locator(ticker.AutoMinorLocator())
        ax.yaxis.set_minor_locator(ticker.AutoMinorLocator())

        # y_max = 60 if r == 0 else 70
        # ax.set_ylim(top=75)
        # if max(datas[i].counts) >= 70:
        #     ax.set_ylim(top=90)
        # else:
        #     ax.set_ylim(top=75)
        # ax.set_xlim(0, 100)

        i += 1

# Label
axs[0, 0].set_xlim(-5, 20)

axs[1, 0].set_ylabel('Occurrences')
axs[1, 0].set_xlabel('Generations needed')

# ax.set_title('Generations needed for the crossed house puzzle')
plt.subplots_adjust(left=0.05, top=0.96, right=0.98, bottom=0.06, hspace=0.4, wspace=0.25)
# plt.tight_layout(pad=0.2, w_pad=0.1, h_pad=0.1, rect=(0.05, 0.05, 0.95, 0.95))
plt.show()
