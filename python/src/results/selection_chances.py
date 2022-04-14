import matplotlib.pyplot as plt
import numpy as np

with open('./chances.txt', 'r') as f:
    line1 = f.readline()
    line2 = f.readline()
    p1 = np.fromstring(line1[1:-1], sep=' ')
    p2 = np.fromstring(line2[1:-1], sep=' ')
    x = np.arange(len(p1))


width = 0.35  # the width of the bars
fig, ax = plt.subplots(figsize=(14, 11))
rects1 = ax.bar(x - width/2, p1, width, label='without increase')
rects2 = ax.bar(x + width/2, p2, width, label='with increase for small supernodes')

ax.set_xticks(x, x)
ax.legend()
ax.set_ylim(0, 0.2)

ax.bar_label(rects1, padding=3)
ax.bar_label(rects2, padding=3)
# fig.tight_layout()
plt.xlabel('Supernode')
plt.ylabel('Chance of selecting the supernode')
plt.title('Selection chances')
plt.show()

# plt.ylim(0, 0.2)
# plt.plot(x, p1, marker='o', linestyle='--', label='without increase')
# plt.plot(x, p2, marker='o', linestyle='--', label='with increase for small supernodes')
# plt.xlabel('Supernodes')
# plt.ylabel('Chance of selecting a supernode')
# plt.title('Selection chances')
# plt.legend()
# plt.show()
