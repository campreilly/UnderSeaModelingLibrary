"""Plot outputs from units tests for usml.waveq3d package.
"""
import inspect
import os
import unittest

import matplotlib as mpl
import matplotlib.patches as ptch
import matplotlib.pyplot as plt
import numpy as np
import pyproj
import scipy.interpolate as interp

import usml.eigenrays
import usml.netcdf
import usml.plot


def plot_wavefront_sphere(ax, wavefront, az: float = 0.0, de=None, time: float = 0.0, fmt="-"):
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


class TestWaveQ3D(unittest.TestCase):
    """Plot outputs from units tests for usml.waveq3d package.
    """
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "waveq3d", "test")

    @classmethod
    def setUpClass(cls):
        """Setup matplotlib defaults."""
        mpl.rcdefaults()
        plt.rcParams["figure.constrained_layout.use"] = True
        plt.rcParams["figure.figsize"] = [8, 6]

    @classmethod
    def tearDownClass(cls):
        """Reset matplotlib defaults."""
        mpl.rcdefaults()

    def test_concave_rays(self):
        """Plots iovelocity ray paths on a spherical earth to illustrate effects of curvature.

        ref: Section 4.2 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "eigenray_concave_wave.nc")
        wavefront = usml.netcdf.read(filename)

        # plot ray path on a flat earth
        fig, ax = plt.subplots()
        usml.plot.plot_raytrace_2d(ax, wavefront)
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 175.0)
        ax.set_ylim(-1200, 0)

        output = os.path.join(self.USML_DIR, testname + "_flat.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # re-extract rays along AZ=0 from wavefront
        src_latitude = wavefront.latitude[0, 0, 0]
        _, _, az_index = np.intersect1d(0.0, wavefront.source_az, return_indices=True)
        latitude = wavefront.latitude[:, :, az_index]
        altitude = wavefront.altitude[:, :, az_index]

        # compute ray paths on spherical earth
        earth_radius = 6378101.030201019  # earth radius at 45 deg north
        theta = np.radians(latitude - src_latitude)
        rho = earth_radius + altitude
        ray_x = rho * np.sin(theta) / 1000.0
        ray_y = rho * np.cos(theta) - earth_radius

        # compute location of ocean surface on spherical earth
        surf_t = np.radians(np.linspace(0.0, 2.0, num=200))
        surf_x = earth_radius * np.sin(surf_t) / 1000.0
        surf_y = earth_radius * np.cos(surf_t) - earth_radius

        # plot ray paths on spherical earth
        fig, ax = plt.subplots()
        ax.plot(surf_x, surf_y, 'k-')
        ax.plot(ray_x[:, :, 0], ray_y[:, :, 0])
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 160.0)
        ax.set_ylim(-2000, 0)

        output = os.path.join(self.USML_DIR, testname + "_sphere.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_tl_az(self):
        """Illustrate the variability of transmission loss as a function of azimuth.

        These oscillations are a side effect of the way that Guassians from multiple azimuths are added together to
        create the total transmission loss.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "eigenray_tl_az.nc")
        eigenrays = usml.eigenrays.wq3d_eigenrays(filename)

        # plot ray path on a flat earth
        fig, ax = plt.subplots()
        ax.plot(eigenrays.source_az, eigenrays.intensity - np.mean(eigenrays.intensity))
        ax.grid(True)
        ax.set_xlabel('Target Bearing (deg)')
        ax.set_ylabel('Intensity Difference (dB)')

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_reflect_flat(self):
        """Flat bottom reflection test results.

        ref: Section 3.1 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "reflect_flat_test.csv")
        results = np.loadtxt(filename, skiprows=1, delimiter=",")
        latitude = results[:, 1]
        altitude = results[:, 3]

        # plot ray path on a flat earth
        fig, ax = plt.subplots()
        ax.plot(latitude, altitude)
        ax.grid(True)
        ax.set_xlabel('Latitude (deg)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(45.0, 45.9)
        ax.set_ylim(-1000, 0)

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_reflect_slope(self):
        """Sloped bottom reflection test results.

        ref: Section 3.2 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "reflect_slope_test.csv")
        results = np.loadtxt(filename, skiprows=1, delimiter=",")
        latitude = results[:, 1]
        altitude = results[:, 3]

        # plot ray path on a flat earth
        fig, ax = plt.subplots()

        coord = np.array([[45.1, -1000], [45.4, -1000], [45.4, -425]])
        patch = ptch.Polygon(coord, facecolor=[0.7, 0.7, 0.7])
        ax.add_patch(patch)

        ax.plot(latitude, altitude)
        ax.grid(True)
        ax.set_xlabel('Latitude (deg)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(45.0, 45.4)
        ax.set_ylim(-1000, 0)

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_reflect_grid(self):
        """Sloped bottom reflection test results.

        Plots the bathymetry for the Malta escarpment and then plots the ray path from the *.csv file on top of it.
        The ray paths for both the reflect_grid_test.csv and reflect_grid_test are plotted to see if they line up on
        top of eac hother. We turn off the computed_zorder so that matplotlib doesn't default to plotting the surface
        on top of the line plots. Also demonstrate a method for upsampling the bathymetry grid plot.

        ref: Section 3.2 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load bathymetry data from disk and plot in 3D
        filename = os.path.join(self.USML_DIR, "../../data/bathymetry/ETOPO1_Ice_g_gmt4.grd")
        print(f"reading {filename}")
        bathymetry = usml.netcdf.Bathymetry(filename, lat_range=(35.6, 36.4), lng_range=(15.35, 16.05))

        spline = interp.RectBivariateSpline(bathymetry.latitude, bathymetry.longitude, bathymetry.altitude)
        bathymetry.latitude = np.linspace(bathymetry.latitude[1], bathymetry.latitude[-1], num=200)
        bathymetry.longitude = np.linspace(bathymetry.longitude[1], bathymetry.longitude[-1], num=200)
        bathymetry.altitude = spline(bathymetry.latitude, bathymetry.longitude)

        fig, ax = plt.subplots(subplot_kw={'projection': '3d', 'computed_zorder': False}, figsize=[16, 12])
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.set_proj_type('ortho')
        ax.view_init(70, -70)
        ax.set_xlim(bathymetry.longitude[1], bathymetry.longitude[-1])
        ax.set_ylim(bathymetry.latitude[1], bathymetry.latitude[-1])

        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")

        # load acoustic ray trace data from disk and plot in 3D

        filename = os.path.join(self.USML_DIR, "reflect_grid_test.csv")
        results = np.loadtxt(filename, skiprows=1, delimiter=",")
        latitude = results[:, 1]
        longitude = results[:, 2]
        altitude = results[:, 3]
        ax.plot(longitude, latitude, altitude, 'k-')

        # load acoustic ray trace data from disk and plot in 3D

        filename = os.path.join(self.USML_DIR, "reflect_fast_test.csv")
        results = np.loadtxt(filename, skiprows=1, delimiter=",")
        latitude = results[:, 1]
        longitude = results[:, 2]
        altitude = results[:, 3]
        ax.plot(longitude, latitude, altitude, 'r--')

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()


if __name__ == '__main__':
    unittest.main()
