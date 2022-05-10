import csv

import matplotlib.pyplot as plt
import numpy as np
from src.embedding.embedding import Embedding

# Matplotlib
# https://stackoverflow.com/a/7389998
# https://stackoverflow.com/a/10944967
plt.ion()

supernode_colors = ['#F29E38', '#F23827', '#D748F5', '#39DBC8',
                    '#F5428A', '#3CDE73', '#11F0EB', '#E9B952',
                    '#7D2EFF', '#DBDE5D', '#3A2CE0', '#DE6E31',
                    '#E0165C']


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
            color = supernode_colors[i % len(supernode_colors)]
            line, = self.ax.plot([], [], color=color, marker='x', label=i)
            self.lines.append(line)
            self.y_percentage_datas.append([])

        plt.xlabel('Generation')
        plt.ylabel('Super vertices degree percentages')
        plt.legend()

        # Autoscale on unknown axis and known lims on the other
        self.ax.set_autoscaley_on(True)
        # self.ax.set_xlim(0, 100)

        self.ax.grid()

    def _update_plot(self, generation, values):
        # https://stackoverflow.com/a/24272092

        # Set new data
        self.x_generation_data.append(generation)
        for i, v in enumerate(values):
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
