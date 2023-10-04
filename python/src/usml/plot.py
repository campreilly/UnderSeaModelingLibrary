"""Plot USML data product
"""

import matplotlib
import matplotlib.patches as ptch
import matplotlib.pyplot as plt
import numpy as np


def ocean_colormap():
    # Create a special color map with blue water, tan shallows, green land
    num_colors = 50
    grey = matplotlib.colormaps['grey']
    newcolors = grey(np.linspace(0, 1, num_colors))
    newcolors[:, 2] = 0.75
    green = np.array([0.0, 0.5, 0.0, 1.0])
    newcolors[num_colors - 1, :] = green
    newcmp = matplotlib.colors.ListedColormap(newcolors)
    return newcmp


def plot_eigenverbs_2d(ax: plt.Axes, verbs, index):
    """Draw 2D plot of eigenverbs projected onto ocean bottom.

    :param ax:              matplotlib axis to use for drawing
    :param verbs:           list of eigenverbs to plot
    :param index:           subset of indices to plot
    """
    plt.scatter(verbs.longitude[index], verbs.latitude[index], 20)
    for n in index:  # range(verb_shape[0]):
        x = verbs.longitude[n]
        y = verbs.latitude[n]
        scale = 6371e3 * np.cos(np.radians(y))
        height = 2.0 * np.degrees(verbs.width[n] / scale)
        width = 2.0 * np.degrees(verbs.length[n] / scale)
        angle = 90 - verbs.direction[n]
        ellipse = ptch.Ellipse((x, y), width=width, height=height, angle=angle, facecolor="none", edgecolor="black")
        ax.add_patch(ellipse)


def plot_bathymetry_3d(ax, bathymetry):
    """Create 3D surface plot of ocean bathymetry

    Reshapes latitude, longitude, and altitude bathymetry fields to support format required by plot_trisurf(). Uses
    ocean_colormap() to create a special color map with blue water, tan shallows, green land. Clips all land values
    to zero altitude.

    :param ax:              matplotlib axis to use for drawing
    :param bathymetry:      bathmetry data loaded into 1D latitude, 1D longitude, and 2D altitude
    """
    # reshape x, y, z, variables to support format required by plot_trisurf()
    x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
    z = bathymetry.altitude

    x = np.reshape(x, [x.size, ])
    y = np.reshape(y, [y.size, ])
    z = np.reshape(z, [z.size, ])

    # clip all "land" values to zero elevation
    index = np.where(z >= 0)
    z[index] = 0.0

    # display 3D triangulated surface
    surface = ax.plot_trisurf(x, y, z, cmap=ocean_colormap(), linewidth=0, antialiased=False)
    return surface
