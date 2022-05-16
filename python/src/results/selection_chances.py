import matplotlib.pyplot as plt
import numpy as np

with open('./chances.txt', 'r') as f:
    line1 = f.readline()
    line2 = f.readline()
    p1 = np.fromstring(line1[1:-1], sep=' ')
    p2 = np.fromstring(line2[1:-1], sep=' ')
    x = np.arange(len(p1))

# --- Plot setup
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Helvetica"
})

# https://stackoverflow.com/a/39566040/9655481
MEDIUM_SIZE = 8
MEDIUM_SIZE = 18
BIGGER_SIZE = 20

plt.rc('font', size=MEDIUM_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=MEDIUM_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=MEDIUM_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title


width = 0.35  # the width of the bars
fig, ax = plt.subplots(figsize=(14, 11))
color_blue = '#2981B3'
color_red = '#E53054'
rects1 = ax.bar(x - width/2, p1, width, label='without increase', color=color_blue)
rects2 = ax.bar(x + width/2, p2, width,
                label='with increase for small super vertices', color=color_red)

# ax.set_xticks(x, x)
ax.legend()
ax.set_ylim(0, 0.5)

ax.bar_label(rects1, padding=3)
ax.bar_label(rects2, padding=3)
# fig.tight_layout()
plt.xlabel('Super vertex')
plt.ylabel('Chance of selecting the super vertex')
# plt.title('Selection chances')
plt.tight_layout()
plt.show()

# plt.ylim(0, 0.2)
# plt.plot(x, p1, marker='o', linestyle='--', label='without increase')
# plt.plot(x, p2, marker='o', linestyle='--', label='with increase for small supernodes')
# plt.xlabel('Supernodes')
# plt.ylabel('Chance of selecting a supernode')
# plt.title('Selection chances')
# plt.legend()
# plt.show()
