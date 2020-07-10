#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
plt.rcParams["font.family"] = "Times New Roman"

def plot_spectral_points():
    if len(sys.argv) < 2:
        print("Usage:", sys.argv[0], "input_spectral_txt")
        return

    print("Reading from file: ", sys.argv[1])
    spectral_points = np.loadtxt(sys.argv[1])

    print("Plotting")

    # Get rid of tiny magnitudes
    spectral_points = spectral_points[spectral_points[:,2] > 0.02 * np.max(spectral_points[:,2])]

    # Turn radians into hertz
    spectral_points[:,1] /= 2 * np.pi

    # Magnitude is the alpha channel
    value = spectral_points[:,2]
    value /= max(value)
    value *= 12.
    value = np.clip(value,0.,1.)
    m = cm.ScalarMappable(cmap="inferno")
    colors = m.to_rgba(value)
    colors[:,3] = value # set alpha

    xmin = np.min(spectral_points[:,0])
    xmax = np.max(spectral_points[:,0])
    ymin = np.min(spectral_points[:,1])
    ymax = np.max(spectral_points[:,1])
    # xmin, xmax = 1, 4.3
    # xmin, xmax = 0, 5.275
    # ymin, ymax = 430, 560
    # ymin, ymax = 0, 1600
    # ymin, ymax = 0, 22000 
    aspect = 0.5625

    # Plot scatter points
    fig = plt.figure()
    a = fig.add_subplot(1, 1, 1)
    a.scatter(spectral_points[:,0], spectral_points[:,1], marker='o', c=colors, s=0.1, rasterized=True)
    a.set_xlim([xmin,xmax])
    a.set_ylim([ymin,ymax])
    a.set_aspect(aspect * (xmax - xmin)/(ymax - ymin))
    a.set_xticks([])
    a.set_yticks([])
    a.set_facecolor('black')
    a.tick_params(direction='in')
    plt.savefig(sys.argv[1] + ".png", bbox_inches='tight', pad_inches=0, dpi=600)
    plt.show()

if __name__ == "__main__":
    plot_spectral_points()
