import csv

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from src.drawing.node_colors import get_supernode_color
from src.embedding.embedding import Embedding

# Matplotlib
# https://stackoverflow.com/a/7389998
# https://stackoverflow.com/a/10944967
plt.ion()
plt.rcParams.update({
    'axes.titlesize': 22,
    'text.usetex': True,
    'font.family': 'Helvetica'
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


class DegreePercentageData():

    x_generation_data = []
    y_percentage_datas = []
    lines = []
    DP_FILE_PATH = './out/dp.csv'

    def __init__(self, max_nodes) -> None:
        self.max_nodes = max_nodes
        self._setup_plot()

    def save_current_degree_percentages(self, generation, embedding: Embedding):
        degree_percentages = embedding.get_supernode_degree_percentages()
        values = [round(value, 2) for value in degree_percentages.values()]
        self._update_plot(generation, values)

        # Append to file
        with open(self.DP_FILE_PATH, 'a', newline='') as f:
            writer = csv.writer(f)
            row = [generation] + values
            writer.writerow(row)

    def _setup_plot(self):
        self.figure, self.ax = plt.subplots()

        # Prepare 2D lines
        for i in range(self.max_nodes):
            self.y_percentage_datas.append([])

            # Style plot
            # https://stackoverflow.com/a/55762294/9655481
            #linewidth = 5 - 3 * (i / self.max_nodes)
            #linestyle = ['-', '--', '-.', ':'][i % 4]
            color = get_supernode_color(i)

            line, = self.ax.plot([], [], color=color, marker='o', markersize=5,
                                 label=i, linewidth=1.8)
            self.lines.append(line)

        plt.xlabel('Generation')
        plt.ylabel('Super vertices degree percentages')
        plt.legend()

        # Autoscale on unknown axis and known lims on the other
        self.ax.set_autoscaley_on(True)
        self.ax.set_ylim(0.0, 1.1)
        self.ax.xaxis.set_major_locator(ticker.MultipleLocator(1))
        # self.ax.set_xlim(0, 100)

        self.ax.grid()

        plt.tight_layout()
        plt.show(block=False)

    def _update_plot(self, generation, values):
        # https://stackoverflow.com/a/24272092

        # Set new data
        self.x_generation_data.append(generation)
        for i, v in enumerate(values):
            # Avoid overlapping lines by shifting them a tiny bit
            prior_values = values[:i]
            count_v = prior_values.count(v)
            if count_v:
                v += 0.010 * count_v
            self.y_percentage_datas[i].append(v)

        # Update data (with the new *and* the old points)
        for i, line in enumerate(self.lines):
            line.set_xdata(self.x_generation_data)
            line.set_ydata(self.y_percentage_datas[i])

        # Need both of these in order to rescale
        self.ax.relim()
        self.ax.autoscale_view()

        # We need to draw *and* flush
        self.figure.canvas.draw()
        self.figure.canvas.flush_events()

    def plot_blocking(self):
        plt.show(block=True)
