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

    def test_refraction_great_circle(self):
        """Verify the model's ability to follow great circle routes along the earth's surface.

        ref: Section 2.4 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "refraction_great_circle.csv")
        results = np.loadtxt(filename, skiprows=1, delimiter=",")
        latitude = results[:, 1:16:4]
        longitude = results[:, 2:16:4]
        fig, ax = plt.subplots()
        ax.plot(longitude, latitude)

        rng = np.linspace(0.0, 18.0)  # distance in degrees of latitude
        brg = np.radians(np.linspace(30.0, 90.0, num=3))
        brg, rng = np.meshgrid(brg, rng)
        x = rng * np.sin(brg) - 45.0
        y = rng * np.cos(brg) + 45.0
        ax.plot(x, y, 'k--')

        ax.grid(True)
        ax.set_xlabel('Longitude (deg)')
        ax.set_ylabel('Latitude (deg)')
        ax.set_aspect('equal', 'box')
        plt.show()

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_refraction_linear(self):
        """Evaluates ray refraction accuracy using a comparison to the analytic solution for a linear profile.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "refraction_linear.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")

        dt = data[1, 0] - data[0, 0]
        R = data[0, 1] + 1000
        range = data[:, 5] / 1e3
        zmodel = data[:, 1] - R
        ztheory = data[:, 4]

        fig, ax = plt.subplots()
        ax.plot(range, zmodel)
        ax.plot(range, ztheory, '--')
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.legend(["Model", "Theory"])
        plt.show()

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_refraction_n2_linear(self):
        """Evaluates ray refraction accuracy using a comparison to the analytic solution for a n^2 linear profile.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "refraction_n2_linear.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")

        dt = data[1, 0] - data[0, 0]
        R = data[0, 1] + 1000
        range = data[:, 5] / 1e3
        zmodel = data[:, 1] - R
        ztheory = data[:, 4]

        fig, ax = plt.subplots()
        ax.plot(range, zmodel)
        ax.plot(range, ztheory, '--')
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.legend(["Model", "Theory"])
        plt.show()

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_refraction_munk_range(self):
        """Verify cycle range errors for Munk profile

        ref: Section 2.2 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load Munk wavefront and plot in range/depth coordinates

        filename = os.path.join(self.USML_DIR, "refraction_munk_range.nc")
        wavefront = usml.netcdf.read(filename)

        fig, (ax1, ax2) = plt.subplots(1, 2, width_ratios=[1, 4], sharey=True)

        d = np.linspace(0.0, 5000.0, num=200)
        z = 2.0 / 1300.0 * (d - 1300.0)
        c = 1500.0 * (1 + 7.37e-3 * (z - 1.0 + np.exp(-z)));
        ax1.plot(c, -d)
        ax1.grid(True)
        ax1.set_ylim(-5000, 0)
        ax1.set_xlabel('Speed (m/s)')
        ax1.set_ylabel('Depth (m)')

        usml.plot.plot_raytrace_2d(ax2, wavefront)
        ax2.grid(True)
        ax2.set_xlabel('Range (km)')
        ax2.set_xlim(0, 140.0)
        ax2.set_ylim(-5000, 0)

        output = os.path.join(self.USML_DIR, testname + "refraction_munk_range.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # load Munk error spreadsheet and plot errors

        filename = os.path.join(self.USML_DIR, "refraction_munk_range.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")
        de = data[:, 1]
        range = data[:, 3] / 1e3
        diff = data[:, 4]

        n = de > 0  # launched up
        m = de <= 0  # launched down

        fig, ax = plt.subplots()
        ax.plot(range[n], diff[n], 'kx')
        ax.plot(range[m], diff[m], 'k+')
        ax.grid(True)
        ax.set_xlim(0, 140.0)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Range Error (m)')
        ax.legend(["Launched Up", "Launched Down"])

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_refraction_pedersen_range(self):
        """Verify cycle range errors for Pedersen profile

        ref: Section 2.3 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load Munk wavefront and plot in range/depth coordinates

        filename = os.path.join(self.USML_DIR, "refraction_pedersen_range.nc")
        wavefront = usml.netcdf.read(filename)

        fig, (ax1, ax2) = plt.subplots(1, 2, width_ratios=[1, 4], sharey=True)

        d = np.linspace(0.0, 1200.0, num=200)
        c0 = 1550
        g0 = 1.2
        c = c0 / np.sqrt(1.0 + (2.0 * g0 / c0) * d)
        ax1.plot(c, -d)
        ax1.grid(True)
        ax2.set_ylim(-1200.0, 0)
        ax1.set_xlabel('Speed (m/s)')
        ax1.set_ylabel('Depth (m)')

        usml.plot.plot_raytrace_2d(ax2, wavefront)
        ax2.grid(True)
        ax2.set_xlabel('Range (km)')
        ax2.set_xlim(0, 3.5)
        ax2.set_ylim(-1200.0, 0)

        output = os.path.join(self.USML_DIR, testname + "refraction_munk_range.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # load Pedersen error spreadsheet and plot errors

        filename = os.path.join(self.USML_DIR, "refraction_pedersen_range.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")
        range = data[:, 3] / 1e3
        diff = data[:, 4]

        fig, ax = plt.subplots()
        ax.plot(range, diff, 'kx')
        ax.grid(True)
        ax.set_xlim(2.0, 3.5)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Range Error (m)')

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_proploss_lloyds_range(self):
        """Verify modeled propagation loss as a function of range for Lloyd's mirror,

        Compares total transmission loss and individual paths to the analytic solution for Lloyd's mirror surface
        reflection in an isovelocity ocean. In this test, we expect to see the errors between the model and theory
        grow at short ranges. The discreet time step creates a discontinuity between the direct and reflected paths
        near the ocean surface, and the size of this discontinuity increases at higher D/E angles. The model must
        extrapolate the transmission loss across this fold in the wavefront, and we beleive this extrapolation causes
        the observed errors. Decreasing the size of the time step decreases these errors.

        ref: Section 4.4 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "proploss_lloyds_range.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")

        freq_test = 500.0
        freq = data[:, 0]
        data = data[freq == freq_test, :]

        range = data[:, 1] / 1e3
        model = data[:, 2]
        theory = data[:, 3]
        m1surf = data[:,4]
        m1btm = data[:,5]
        m1amp = data[:, 6]
        m1time = data[:, 7] * 1e3
        t1amp = data[:, 8]
        t1time = data[:, 9] * 1e3
        m1surf = data[:,10]
        m1btm = data[:,11]
        m2amp = data[:, 12]
        m2time = data[:, 13] * 1e3
        t2amp = data[:, 14]
        t2time = data[:, 15] * 1e3

        # plot total transmission loss errors as a function of range
        fig, ax = plt.subplots()
        ax.plot(range, model)
        ax.plot(range, theory)
        ax.grid(True)
        ax.set_xlim(0.0, range[-1])
        ax.set_ylim(-90, -40)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Transmission Loss (dB)')
        ax.legend(["Model", "Theory"])
        ax.set_title(f"Frequency = {freq_test:.0f} Hz")

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # zoom into short ranges
        fig, ax = plt.subplots()
        ax.plot(range, model)
        ax.plot(range, theory)
        ax.grid(True)
        ax.set_xlim(0.0, 1.0)
        ax.set_ylim(-90, -40)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Transmission Loss (dB)')
        ax.legend(["Model", "Theory"])
        fig.suptitle(f"Frequency = {freq_test:.0f} Hz")

        output = os.path.join(self.USML_DIR, testname + "_zoom.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # plot transmission loss and travel time errors as a function of range
        fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
        ax1.plot(range, m1amp - t1amp)
        ax1.plot(range, m2amp - t2amp)
        ax1.grid(True)
        ax1.set_xlim(0.0, range[-1])
        ax1.set_ylabel('Transmission Loss Error (dB)')
        fig.suptitle(f"Frequency = {freq_test:.0f} Hz")

        ax2.plot(range, m1time - t1time)
        ax2.plot(range, m2time - t2time)
        ax2.grid(True)
        ax2.set_ylabel('Travel Time Error (ms)')
        ax2.set_xlabel('Range (km)')
        ax2.legend(["Direct", "Surface"])

        output = os.path.join(self.USML_DIR, testname + "_error.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_proploss_lloyds_depth(self):
        """Verify modeled propagation loss as a function of depth for Lloyd's mirror,

        Compares total transmission loss and individual paths to the analytic solution for Lloyd's mirror surface
        reflection in an isovelocity ocean. Targets near the surface must be extrapolated from the wavefronts below
        them.  Because the Gaussian profile rolls off at edge of a ray family, it is important to test the impact of
        that phenomena on propagation loss at the interface.

        ref: Section 4.4 from Sean M. Reilly, Gopu Potty, "Veriffcation Tests for Hybrid Gaussian Beams in
        Spherical/Time Coordinates," May 2012.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "proploss_lloyds_depth.csv")
        data = np.loadtxt(filename, skiprows=1, delimiter=",")

        freq_test = 500.0
        freq = data[:, 0]
        data = data[freq == freq_test, :]

        depth = data[:, 1]
        model = data[:, 2]
        theory = data[:, 3]
        m1surf = data[:,4]
        m1btm = data[:,5]
        m1amp = data[:, 6]
        m1time = data[:, 7] * 1e3
        t1amp = data[:, 8]
        t1time = data[:, 9] * 1e3
        m1surf = data[:,10]
        m1btm = data[:,11]
        m2amp = data[:, 12]
        m2time = data[:, 13] * 1e3
        t2amp = data[:, 14]
        t2time = data[:, 15] * 1e3

        # plot total transmission loss errors as a function of depth
        fig, ax = plt.subplots()
        ax.plot(model,depth)
        ax.plot(theory, depth)
        ax.grid(True)
        ax.set_xlim(-120, -75)
        ax.set_ylim(depth[-1], 0.0)
        ax.set_xlabel('Transmission Loss (dB)')
        ax.set_ylabel('Depth (m)')
        ax.legend(["Model", "Theory"])
        fig.suptitle(f"Frequency = {freq_test:.0f} Hz")

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # plot transmission loss and travel time errors as a function of depth
        fig, (ax1, ax2) = plt.subplots(1, 2, sharey=True)
        ax1.plot(m1amp - t1amp, depth)
        ax1.plot(m2amp - t2amp, depth)
        ax1.grid(True)
        ax1.set_ylim(depth[-1], 0.0)
        ax1.set_xlabel('Transmission Loss Error (dB)')
        ax1.set_ylabel('Depth (m)')
        fig.suptitle(f"Frequency = {freq_test:.0f} Hz")

        ax2.plot(m1time - t1time, depth)
        ax2.plot(m2time - t2time, depth)
        ax2.grid(True)
        ax2.set_xlabel('Travel Time Error (ms)')
        ax2.legend(["Direct", "Surface"])

        output = os.path.join(self.USML_DIR, testname + "_error.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()


if __name__ == '__main__':
    unittest.main()
