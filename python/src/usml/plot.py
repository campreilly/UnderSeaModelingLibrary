"""Plot USML data product
"""

import matplotlib
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import numpy as np
import pyproj
import scipy


def ocean_colormap(num_colors = 1024):
    """Create a special color map with blue water, tan shallows, green land."""
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
        ellipse = patches.Ellipse((x, y), width=width, height=height, angle=angle, facecolor="none", edgecolor="black")
        ax.add_patch(ellipse)


def plot_bathymetry_3d(ax, bathymetry):
    """Plot 3D surface for ocean bathymetry.

    Reshapes latitude, longitude, and altitude bathymetry fields to support format required by plot_trisurf(). Uses
    ocean_colormap() to create blue water, tan shallows, green land. Clips all "land" values to zero altitude.

    :param ax:              matplotlib axis to use for drawing
    :param bathymetry:      bathmetry data loaded into 1D latitude, 1D longitude, and 2D altitude
    :return:                surface drawn by matplotlib.
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


def plot_bathymetry_2d(ax, bathymetry, latitude: float, longitude: float, bearing: float, ranges):
    """Plot 2D slice of ocean bathymetry.

    Uses the pyproj library to estimate latitude/longitude for a list of ranges along a bearing of 45 degrees from a
    point. It then uses scipy.interpolate to estimate the depth at each of those latitude/longitude points. Depth is
    plotted as a function of range.

    :param ax:              matplotlib axis to use for drawing
    :param bathymetry:      bathmetry data loaded into 1D latitude, 1D longitude, and 2D altitude
    :param latitude:        latitude of plot origin (degrees_north)
    :param longitude:       longitude of plot origin (degrees_east)
    :param bearing:         true bearing of radial to plot (degrees)
    :param ranges:          list of ranges to plot (meters)
    :return:                line drawn by matplotlib.
    """
    # reshape x, y, z, variables to support format required by griddata
    x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
    z = bathymetry.altitude

    x = np.reshape(x, [x.size, ])
    y = np.reshape(y, [y.size, ])
    z = np.reshape(z, [z.size, ])

    # clip all "land" values to zero elevation
    index = np.where(z >= 0)
    z[index] = 0.0

    # find latitude and longitude for each range along this bearing
    geodesic = pyproj.Geod(ellps='WGS84')
    lat = np.full_like(ranges, fill_value=latitude)
    lng = np.full_like(ranges, fill_value=longitude)
    az = np.full_like(ranges, fill_value=bearing)
    lng, lat, baz = geodesic.fwd(lng, lat, az, ranges, return_back_azimuth=False)
    depths = scipy.interpolate.griddata((x, y), z, (lng, lat))

    # display 2D slice of depth vs. range
    line = ax.plot(ranges / 1e3, depths, color="black", linewidth=1.5)
    return line
