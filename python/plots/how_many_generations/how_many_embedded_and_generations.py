import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

names = [str(i) for i in range(2, 15)]
# names = np.linspace(0.0, 1.0, num=20)
# names = [str(round(v, 2)) for v in names]
# names.append('0.22')
# names.append('0.24')
# names.append('0.27')
# names.append('0.29')
# names.append('0.31')
# names.append('0.33')
# names.append('0.36')
# names.append('0.38')
# names.append('0.4')

embedded = []
average_generations = []
max_total = 300

# --- Prepare data
for name in names:
    # filename = f'./data_after_bug_fix/k8_popsize/howManyGenerations_5x5_10_600gen_k8popsize{name}.txt'
    # filename = f'./data_after_bug_fix/k8_probabilities_free/howManyGenerations_5x5_20_600gen_k80.0-0.2prob{name}.txt'
    # filename = f'./data_after_bug_fix/k_graphs/howManyGenerations_16x16_100_600gen_6popsize_k{name}.txt'
    # filename = f'./data_after_bug_fix/k8_probabilities_free/server-run/howManyGenerations_5x5_300_600gen_k8prob{name}.txt'
    filename = f'./data_after_bug_fix/k8_popsize/server-run/howManyGenerations_5x5_300_600gen_k8popsize{name}.txt'
    with open(filename, 'r') as f:
        data = f.read().splitlines()
        data = [int(value) for value in data]
        # unique, counts = np.unique(data, return_counts=True)
        embedded.append(max_total - data.count(-1))

        data_embedded = [value for value in data if value != -1]
        average_generations.append(np.average(data_embedded))


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
plt.rc('xtick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title


# --- Plot
fig, ax = plt.subplots(figsize=(11, 6))
ax.set_xlabel('Population size')
# ax.set_xlabel('Probability for "extend to free neighbors" heuristic')
# ax.set_xlabel('K graph')

# Average generations
color_blue = '#2981B3'
# color_blue = '#2EB4FF'
ax.set_ylabel('Average number of generations needed', color=color_blue)
# ax.plot(names, average_generations, color=color_blue, marker='o')
ax.bar(names, average_generations, color=color_blue, fill=False, edgecolor=color_blue)
ax.tick_params(axis='y', labelcolor=color_blue)


ax2 = ax.twinx()
# Number embedded
color_red = '#E53054'
# color_red = '#FF4A6D'
ax2.set_ylabel('Occurrences of valid embeddings', color=color_red)
ax2.set_ylim(0, max_total+5)
# ax.xaxis.set_major_locator(ticker.MultipleLocator(5))
# ax.xaxis.set_minor_locator(ticker.MultipleLocator(1))
ax2.plot(names, embedded, color=color_red, marker='o')
ax2.tick_params(axis='y', labelcolor=color_red)


# Write average number of generations needed next to data point
# https://queirozf.com/entries/add-labels-and-text-to-matplotlib-plots-annotation-examples
for i, embedded_count in enumerate(embedded):
    percent_embedded = round(embedded_count / max_total * 100, 2)
    ax2.annotate(f'{percent_embedded}\,\%',
                 (i, embedded_count),
                 textcoords='offset points',
                 xytext=(16, 10),
                 horizontalalignment='center',
                 color=color_red,
                 fontsize=14)
# for i, v in enumerate(average_generations):
#     ax.annotate(str(round(v, 0)),
#                 (i, embedded[i]),
#                 textcoords='offset points',
#                 xytext=(0, -22),
#                 horizontalalignment='center')

# ax.set_title('Generations needed for the crossed house puzzle')
plt.tight_layout()
plt.show()
