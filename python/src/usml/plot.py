"""Plot USML data product
"""

import matplotlib
import matplotlib.cm as cm
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import numpy as np
import pyproj
import scipy


def ocean_colormap(num_colors=1024):
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

    # clip all "land" values to zero elevation
    index = np.where(z >= 0)
    z[index] = 0.0

    # display 3D triangulated surface
    surface = ax.plot_surface(x, y, z, rstride=1, cstride=1, cmap=ocean_colormap(), linewidth=0, antialiased=False)
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


def plot_beampattern_3d(ax, pattern):
    """Plot one output from bp_test.cc script.

    :param ax:          Axis on which to draw plot
    :param pattern:     Beampattern as function of D/E and AZ
    """
    pattern = 30 + 10.0 * np.log10(abs(pattern) + 1e-30);
    pattern = np.clip(pattern, a_min=0.0, a_max=None)

    # construct mesh of solid angles
    az = np.linspace(0.0, 2 * np.pi, len(pattern))
    de = np.linspace(-np.pi / 2, np.pi / 2, len(pattern[0]))
    de, az = np.meshgrid(de, az)

    # compute level at each solid angle
    x = pattern * np.cos(de) * np.cos(az)
    y = pattern * np.cos(de) * np.sin(az)
    z = pattern * np.sin(de)
    colors = cm.viridis(pattern / np.amax(pattern))

    # draw 3D surface plot
    ax.plot_surface(x, y, z, rstride=1, cstride=1, facecolors=colors, shade=False)
    ax.axis('equal')


def plot_raytrace_2d(ax, wavefront, az: float = 0.0, de=None, times=None, fmt="-"):
    """Plot 2D ray paths for a selected launch azimuth as a function of range and depth

    :param ax:              matplotlib axis to use for drawing
    :param wavefront:       USMl wavefront data structure
    :param az:              individual source azimuth angle to plot
    :param de:              list of source depression/elevation angles to plot
    :param times:           range of times to plot
    :param fmt:             format string used to draw lines
    :return:                lines drawn by matplotlib.
    """

    # extract source location from first point in first ray
    src_latitude = wavefront.latitude[0, 0, 0]
    src_longitude = wavefront.longitude[0, 0, 0]

    # extract ray positions for selected D/E and AZ
    az_list = np.asarray(az)
    if de is None:
        de = wavefront.source_de

    _, _, de_index = np.intersect1d(de, wavefront.source_de, return_indices=True)
    _, _, az_index = np.intersect1d(az_list, wavefront.source_az, return_indices=True)

    latitude = wavefront.latitude[:, de_index, az_index]
    longitude = wavefront.longitude[:, de_index, az_index]
    altitude = wavefront.altitude[:, de_index, az_index]

    # extract ray positions for selected times
    if times is not None:
        time_index = np.asarray(
            np.logical_and(wavefront.travel_time >= np.min(times), wavefront.travel_time <= np.max(times))).nonzero()
        latitude = latitude[time_index, :][0]
        longitude = longitude[time_index, :][0]
        altitude = altitude[time_index, :][0]

    # extract source location from first point in first ray

    # plot range and depth for each latitude and longitude along this bearing
    geodesic = pyproj.Geod(ellps='WGS84')
    src_latitude = np.full_like(latitude, fill_value=src_latitude)
    src_longitude = np.full_like(longitude, fill_value=src_longitude)
    _, _, ranges = geodesic.inv(src_longitude, src_latitude, longitude, latitude)
    rays = ax.plot(ranges / 1e3, altitude, fmt)
    return rays


def plot_raytrace_3d(ax, wavefront, az=None, de=None, times=None, fmt="-"):
    """Plot 3D ray paths for a list of D/E and azimuth source angles

    :param ax:              matplotlib axis to use for drawing
    :param wavefront:       USMl wavefront data structure
    :param az:              list of source azimuth angles to plot
    :param de:              list of source depression/elevation angles to plot
    :param times:           range of times to plot
    :param fmt:             format string used to draw lines
    :return:                lines drawn by matplotlib.
    """

    # extract ray positions for selected D/E and AZ
    if az is None:
        az = wavefront.source_az
    if de is None:
        de = wavefront.source_de

    _, _, de_index = np.intersect1d(de, wavefront.source_de, return_indices=True)
    _, _, az_index = np.intersect1d(az, wavefront.source_az, return_indices=True)
    de_index, az_index = np.meshgrid(de_index, az_index)
    de_index = de_index.reshape(de_index.size)
    az_index = az_index.reshape(az_index.size)
    latitude = wavefront.latitude[:, de_index, az_index]
    longitude = wavefront.longitude[:, de_index, az_index]
    altitude = wavefront.altitude[:, de_index, az_index]

    # extract ray positions for selected times
    if times is not None:
        time_index = np.asarray(
            np.logical_and(wavefront.travel_time >= np.min(times), wavefront.travel_time <= np.max(times))).nonzero()
        latitude = latitude[time_index, :][0]
        longitude = longitude[time_index, :][0]
        altitude = altitude[time_index, :][0]

    rays = list()
    for n in range(len(latitude[0])):
        h = ax.plot(longitude[:, n], latitude[:, n], altitude[:, n], fmt)
        rays.append(h)
    return rays


def plot_wavefront_2d(ax, wavefront, az: float = 0.0, de=None, time: float = 0.0, fmt="-"):
    """Plot 2D wavefront for a selected launch azimuth as a function of range and depth

    :param ax:              matplotlib axis to use for drawing
    :param wavefront:       USMl wavefront data structure
    :param az:              individual source azimuth angle to plot
    :param de:              list of source depression/elevation angles to plot
    :param time:            individual travel time times to plot
    :param fmt:             format string used to draw lines
    :return:                lines drawn by matplotlib.
    """

    # extract source location from first point in first ray
    src_latitude = wavefront.latitude[0, 0, 0]
    src_longitude = wavefront.longitude[0, 0, 0]

    # extract ray positions for selected D/E and AZ
    az_list = np.asarray(az)
    if de is None:
        de = wavefront.source_de

    _, _, de_index = np.intersect1d(de, wavefront.source_de, return_indices=True)
    _, _, az_index = np.intersect1d(az_list, wavefront.source_az, return_indices=True)

    latitude = wavefront.latitude[:, de_index, az_index]
    longitude = wavefront.longitude[:, de_index, az_index]
    altitude = wavefront.altitude[:, de_index, az_index]

    # extract ray positions for selected time
    time_index = np.absolute(wavefront.travel_time - time).argmin()
    latitude = latitude[time_index, :]
    longitude = longitude[time_index, :]
    altitude = altitude[time_index, :]

    # plot range and depth for each latitude and longitude along this bearing
    geodesic = pyproj.Geod(ellps='WGS84')
    src_latitude = np.full_like(latitude, fill_value=src_latitude)
    src_longitude = np.full_like(longitude, fill_value=src_longitude)
    az12, az21, ranges = geodesic.inv(src_longitude, src_latitude, longitude, latitude)
    wave = ax.plot(ranges / 1e3, altitude, fmt)
    return wave


def plot_wavefront_3d(ax, wavefront, az=None, de=None, time: float = 0.0, **kwargs):
    """Plot 3D ray paths for a list of D/E and azimuth source angles

    :param ax:              matplotlib axis to use for drawing
    :param wavefront:       USMl wavefront data structure
    :param az:              list of source azimuth angles to plot
    :param de:              list of source depression/elevation angles to plot
    :param time:            individual travel time times to plot
    :param fmt:             format string used to draw lines
    :return:                lines drawn by matplotlib.
    """

    # extract ray positions for selected times
    time_index = np.absolute(wavefront.travel_time - time).argmin()
    latitude = wavefront.latitude[time_index, :, :]
    longitude = wavefront.longitude[time_index, :, :]
    altitude = wavefront.altitude[time_index, :, :]

    # extract ray positions for selected D/E and AZ
    if az is None:
        az = wavefront.source_az
    if de is None:
        de = wavefront.source_de

    _, _, de_index = np.intersect1d(de, wavefront.source_de, return_indices=True)
    _, _, az_index = np.intersect1d(az, wavefront.source_az, return_indices=True)
    de_index, az_index = np.meshgrid(de_index, az_index)
    latitude = latitude[de_index, az_index]
    longitude = longitude[de_index, az_index]
    altitude = altitude[de_index, az_index]

    surface = ax.plot_surface(longitude, latitude, altitude, **kwargs)
    return surface
