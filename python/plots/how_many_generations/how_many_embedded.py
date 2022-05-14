import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

names = [str(i) for i in range(1, 21)]
# names = ['0.0', '0.11']
embedded = []
average_generations = []

# --- Prepare data
for name in names:
    filename = f'./data/population_size_100_runs/how_many_generations_5x5_100_600_max_gen_k8_population_size_{name}.txt'
    # filename = f'./data/extend_to_free_prob_50_runs/how_many_generations_5x5_50_600_max_gen_k8_extend_to_free_prob{name}.txt'
    with open(filename, 'r') as f:
        data = f.read().splitlines()
        data = [int(value) for value in data]
        # unique, counts = np.unique(data, return_counts=True)
        embedded.append(100 - data.count(-1))

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
color = '#E53054'
fig, ax = plt.subplots(figsize=(10, 6))
ax.set_ylabel('Occurrences of valid embeddings')
ax.set_xlabel('Population size')
ax.set_ylim(0, 100)
# ax.xaxis.set_major_locator(ticker.MultipleLocator(5))
# ax.xaxis.set_minor_locator(ticker.MultipleLocator(1))
ax.plot(names, embedded, color, marker='o')

# Write average number of generations needed next to data point
# https://queirozf.com/entries/add-labels-and-text-to-matplotlib-plots-annotation-examples
for i, v in enumerate(average_generations):
    ax.annotate(str(round(v, 0)),
                (i, embedded[i]),
                textcoords='offset points',
                xytext=(0, -22),
                horizontalalignment='center')

# ax.set_title('Generations needed for the crossed house puzzle')
plt.tight_layout()
plt.show()
