#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt
plt.rcParams["font.family"] = "Times New Roman"

def plot_groups():
    if len(sys.argv) < 2:
        print("Usage:", sys.argv[0], "input_file")
        return

    print("Reading from file: ", sys.argv[1])
    groups = np.loadtxt(sys.argv[1])

    print("Plotting")

    aspect = 0.5

    # Make a figure
    fig = plt.figure()

    xmin, xmax = 400, 920
    ymin, ymax = -100, 100
    lw = 0.8

    # Construct the first
    a = fig.add_subplot(2, 1, 2)
    a.set_xlabel('Frequency (hz)')
    a.set_ylabel('Frequency (hz)')
    a.set_xlim([xmin,xmax])
    a.set_ylim([xmin,xmax])
    a.set_aspect(aspect)

    # Convert to hz and db
    groups[:,0] /= 2 * np.pi
    groups[:,1] /= 2 * np.pi
    groups[:,2] = 20 * np.log(groups[:,2])/np.log(10)

    a.plot(groups[:,0], groups[:,0], color="black", linestyle=":", linewidth=lw)
    a.plot(groups[:,0], groups[:,1], color="black", linewidth=lw)
    for i in range(groups.shape[0]):
        if groups[i,3]:
            plt.axvline(x=groups[i,0], linewidth=lw)
        if groups[i,4]:
            plt.scatter(groups[i,0],groups[i,1], marker=".", color="xkcd:red", linewidth=lw)


    # Construct the second
    a = fig.add_subplot(2, 1, 1)
    a.set_xticks([])
    a.set_ylabel('Amplitude (dB)')
    a.set_xlim([xmin,xmax])
    a.set_ylim([ymin,ymax])
    a.set_aspect(aspect * (xmax - xmin)/(ymax - ymin))

    a.plot(groups[:,0], groups[:,2], color="black", linewidth=lw)
    for i in range(groups.shape[0]):
        if groups[i,3]:
            plt.axvline(x=groups[i,0], linewidth=lw)
        if groups[i,4]:
            plt.scatter(groups[i,0],groups[i,2], marker=".", color="xkcd:red", linewidth=lw)

    fig.subplots_adjust(wspace=0, hspace=0.1)
    plt.savefig("groups.pdf", bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()

if __name__ == "__main__":
    plot_groups()
