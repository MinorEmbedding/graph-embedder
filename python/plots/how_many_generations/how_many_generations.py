import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# --- Prepare data
with open('./how_many_generations_k7.txt', 'r') as f:
    data = f.read().splitlines()
    data = [int(value) for value in data]

unique, counts = np.unique(data, return_counts=True)


# --- Plot setup
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Helvetica"
})

# https://stackoverflow.com/a/39566040/9655481
MEDIUM_SIZE = 8
MEDIUM_SIZE = 10
BIGGER_SIZE = 12

plt.rc('font', size=MEDIUM_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=MEDIUM_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title


# --- Plot
fig, ax = plt.subplots(figsize=(10, 6))
ax.axhline(0, color='grey', linewidth=0.8)
ax.set_ylabel('Scores', fontsize=18)
ax.set_ylabel('Occurrences')
ax.set_xlabel('Generations needed', fontsize=18)
ax.xaxis.set_major_locator(ticker.MultipleLocator(5))
ax.xaxis.set_minor_locator(ticker.MultipleLocator(1))
ax.bar(unique, counts)

# ax.set_title('Generations needed for the crossed house puzzle')
plt.tight_layout()
plt.show()
