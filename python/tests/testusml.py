"""Test ability to read netCDF files created by USML.
"""
import inspect
import os
import unittest

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.plot


class TestUSML(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_beampatterns(self):
        """Plot all outputs from bp_test.cc script.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filenames = (
            "bp_omni.csv",
            "bp_cos.csv",
            "bp_sin.csv",
            "bp_vla.csv",
            "bp_hla.csv",
            "bp_planar.csv",
            "bp_piston.csv",
            "bp_multi.csv",
            "bp_cardioid.csv",
            "bp_arb.csv",
            "bp_arb_weight.csv",
            "bp_solid.csv")
        for file in filenames:
            # read beam pattern from disk
            fullfile = os.path.join(self.USML_DIR, "beampatterns/test", file)
            output = file.replace(".csv", "")
            print("reading {0}".format(fullfile))
            pattern = np.genfromtxt(fullfile, delimiter=',')

            # plot beam pattern
            fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
            usml.plot.plot_beampattern_3d(ax, pattern)
            ax.set_title(output)
            ax.set_xlabel('x')
            ax.set_ylabel('y')
            ax.set_zlabel('z')
            print("saving {0}.png".format(output))
            plt.savefig(output)
            plt.close()

    def test_seq_rayfan(self):
        """Plot angle set of tangent spaced beams to illustrate features of USML's seq_rayfan class.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        first = -90.0
        last = 90.0
        spread = 6.0
        center = 0.0
        num = 45
        radius = np.linspace(0, 12e3, 13)

        first_ang = first - center
        last_ang = last - center
        scale = (last_ang - first_ang) / (num - 1)
        n = np.linspace(0, num - 1, num)
        x = first_ang + scale * n
        angle = center + x

        angle = np.radians(angle)
        (aa, rr) = np.meshgrid(angle, radius)
        x = rr * np.cos(aa)
        y = -75.0 + rr * np.sin(aa)

        fig, (ax1, ax2) = plt.subplots(2, 1)

        ax1.plot(x / 1e3, y, 'k-')
        ax1.set_title("{} uniformly spaced rays".format(num))
        ax1.set_xlim(0, 10)
        ax1.set_ylim(-200, 0)
        ax1.set_xticklabels([])
        ax1.set_ylabel('Depth (m)')

        first_ang = np.arctan((first - center) / spread)
        last_ang = np.arctan((last - center) / spread)
        scale = (last_ang - first_ang) / (num - 1)
        x = first_ang + scale * n
        angle = center + np.tan(x) * spread

        angle = np.radians(angle)
        (aa, rr) = np.meshgrid(angle, radius)
        x = rr * np.cos(aa)
        y = -75.0 + rr * np.sin(aa)

        ax2.plot(x / 1e3, y, 'k-')
        ax2.set_title("{} tangent spaced rays".format(num))
        ax2.set_xlim(0, 10)
        ax2.set_ylim(-200, 0)
        ax2.set_xlabel('Range (km)')
        ax2.set_ylabel('Depth (m)')

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

        fig, ax3 = plt.subplots(subplot_kw={'projection': 'polar'})
        ax3.plot(angle, np.ones(angle.shape), 'o')
        ax3.set_title("std = {:.3f} deg, min diff = {:.3f} deg".format(np.std(angle), np.min(np.diff(angle))))
        print("saving {0}.png".format(testname + "_polar"))
        plt.savefig(testname + "_polar")
        plt.close()

    def test_ambient(self):
        """Plots results of ambient_wenz_test case in ambient_test.cc unit test.

        Computes ambient noise levels for ambient_wenz model for a variety of sea states, shipping levels, and rain rates.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read ambient noise levels from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/ambient_wenz_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        frequency = model[0, 3:]
        sea_state = np.unique(model[1:, 0])
        ship_level = np.unique(model[1:, 1])
        rain_rate = np.unique(model[1:, 2])
        ambient = model[1:, 3:]
        ambient = ambient.reshape(ambient.size)
        # ambient = ambient.reshape(len(frequency),len(sea_state),len(ship_level),len(rain_rate))
        ambient = ambient.reshape(len(rain_rate), len(ship_level), len(sea_state), len(frequency))

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_sea_state"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, 0, :, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Sea State 0", "Sea State 1", "Sea State 2", "Sea State 3", "Sea State 4", "Sea State 5",
                      "Sea State 6"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_ship_level"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, :, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Shipping 0", "Shipping 1", "Shipping 2", "Shipping 3", "Shipping 4", "Shipping 5", "Shipping 6",
                      "Shipping 7"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_rain_rate"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[:, 0, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["No Rain", "Light Rain", "Medium Rain", "Heavy Rain"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

    def test_profile_analytic(self):
        """Plot  results of analytic test models written out to CSV by plot_profile_test in test_profile.cc file.
        The plotted models include:
            - Constant Profile
            - Linear Profile
            - Bi-Linear Profile
            - Munk Profile
            - N^2 Linear Profile
            - Catenary Profile
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/profile_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        labels = ["FlatEarth", "Linear", "BiLinear", "Munk", "N2 Linear", "Catenary"]
        depth = model[1:, 0]
        speed = model[1:, 1::2]
        grad = model[1:, 2::2]

        fig, (ax1, ax2) = plt.subplots(1, 2)
        h = ax1.plot(speed, depth)
        ax1.grid(True)
        ax1.set_xlabel('Speed (m/s)')
        ax1.set_ylabel('Depth (m)')
        ax1.legend(h, labels)

        h = ax2.plot(grad, depth)
        ax2.grid(True)
        ax2.set_xlabel('Grad (m/s/m)')
        ax2.set_yticklabels([])

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_mackenzie(self):
        """Plot results of Mackenzie models written out to CSV by compute_mackenzie_test in test_profile.cc file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/mackenzie_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        depth = -model[1:, 0]
        temp = model[1:, 1]
        salinity = model[1:, 2]
        speed = model[1:, 3]
        grad = model[1:, 4]

        fig, (ax1, ax2, ax3, ax4) = plt.subplots(1, 4)
        h = ax1.plot(temp, depth)
        ax1.grid(True)
        ax1.set_xlabel('Temp (degC)')
        ax1.set_ylabel('Depth (m)')

        h = ax2.plot(salinity, depth)
        ax2.grid(True)
        ax2.set_xlabel('Salinity (ppt)')
        ax2.set_yticklabels([])

        h = ax3.plot(speed, depth)
        ax3.grid(True)
        ax3.set_xlabel('Speed (m/s)')
        ax3.set_yticklabels([])

        h = ax4.plot(grad, depth)
        ax4.grid(True)
        ax4.set_xlabel('Grad (m/s/m)')
        ax4.set_yticklabels([])

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_rayleigh_sediments(self):
        """Tests the version of the reflect_loss_rayleigh() constructor that takes a sediment type as its argument.

        The parameters for each sediment type are defined using table 1.3 from F.B. Jensen, W.A. Kuperman,
        M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 41. Note that reflect_loss_rayleigh() does not
        implement the weak, depth dependent, shear in slit, sand, and gravel.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_sediments.csv")
        print("reading {0}".format(fullfile))
        labels = ["clay", "silt", "sand", "gravel", "moraine", "chalk", "limestone", "basalt"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        fig, ax = plt.subplots()
        h = ax.plot(angle, reflect_loss)
        ax.grid(True)
        ax.set_xlabel('Grazing Angle (deg)')
        ax.set_ylabel('Reflection Loss (dB)')
        ax.set_ylim(0, 20)
        ax.legend(h, labels, ncol=4)

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_rayleigh_changes(self):
        """Tests reflect_loss_rayleigh() constructor that takes compression and shear wave parameters as inputs.

        Illustrates how the relection loss changes as a function of compressional sound speed, density, attenuation,
        and shear speed. Implements the four test cases (a, b, c,and d) defined in  F.B. Jensen, W.A. Kuperman,
        M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 35-49.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2)
        maxDb = 30

        # reflection loss for changes in compressional sound speed
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_a.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$c_p$=1550", r"$c_p$=1600", r"$c_p$=1800"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax1.plot(angle, reflect_loss)
        ax1.grid(True)
        ax1.set_xticklabels([])
        ax1.set_ylabel('Reflection Loss (dB)')
        ax1.set_ylim(0, maxDb)
        ax1.set_title(r"$\rho$=2 $\alpha$=0.5")
        ax1.legend(h, labels)

        # reflection loss for changes in compressional attenuation
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_b.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$\alpha$=1.0", r"$\alpha$=0.5", r"$\alpha$=0.0"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax2.plot(angle, reflect_loss)
        ax2.grid(True)
        ax2.set_xticklabels([])
        ax2.set_yticklabels([])
        ax2.set_ylim(0, maxDb)
        ax2.set_title(r"$c_p$=1600 $\rho$=2")
        ax2.legend(h, labels)

        # reflection loss for changes in density
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_c.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$\rho$=1.5", r"$\rho$=2.0", r"$\rho$=2.5"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax3.plot(angle, reflect_loss)
        ax3.grid(True)
        ax3.set_xlabel('Grazing Angle (deg)')
        ax3.set_ylabel('Reflection Loss (dB)')
        ax3.set_ylim(0, maxDb)
        ax3.set_title(r"$c_p$=1600 $\alpha$=0.5")
        ax3.legend(h, labels)

        # reflection loss for changes in shear speed
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_d.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$c_s$=0.0", r"$c_s$=200", r"$c_s$=400", r"$c_s$=600"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax4.plot(angle, reflect_loss)
        ax4.grid(True)
        ax4.set_xlabel('Grazing Angle (deg)')
        ax4.set_ylabel('Reflection Loss (dB)')
        ax4.set_ylim(0, maxDb)
        ax4.set_title(r"$c_p$=1600 $\rho$=2 $\alpha$=0.5")
        ax4.legend(h, labels)

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_wave_height_pierson(self):
        """Plot data output from Pierson and Moskowitz model for computing wave height from wind speed.

        Compare to significant wave height plot from http://www.wikiwaves.org/Ocean-Wave_Spectra.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/wave_height_pierson_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        wind_speed = model[1:, 0]
        wave_height = model[1:, 1]

        fig, ax = plt.subplots()
        h = ax.plot(wind_speed, wave_height)
        ax.grid(True)
        ax.set_xlim(0, 25.0)
        ax.set_ylim(0, 20.0)
        ax.set_xlabel('Wind Speed (m/s)')
        ax.set_ylabel('Significant Wave Height (m)')

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_bathy_ncks_3d(self):
        """Draw 3D map of bathymetry around Hawaii from netCDF file extracted by ncks.

        Tests the abilities to:

        - Ready bathymetry from file extracted from ETOPO1 using ncks.
        - Read whole bathymetry file without specifying lat_range or lng_range.
        - Create 3D bathymetry surface plot

        Displays bathymetry as matplotlib surface plot. If the test runs correctly, then

        - The batymetry loads from netCDF file without errors.
        - Latitudes extend from 18N to 23N along the y-axis.
        - Longitudes extend from 160W to 154W along the x-axis.
        - Depths extend from 0 to -5000 meters.
        - The big island of Hawaii is displayed in the south-east corner.
        - Plot uses special color map with blue water, tan shallows, green land.
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/etopo_cmp.nc")
        print("reading {0}".format(filename))
        bathymetry = usml.netcdf.Bathymetry(filename)
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude

        # test the latitude and longtitude extents of the bathymetry
        # using a-priori knowledge of this file's contents taken from ncdump
        self.assertAlmostEqual(bathymetry.latitude[0], 18.0)
        self.assertAlmostEqual(bathymetry.latitude[-1], 23.0)
        self.assertAlmostEqual(bathymetry.longitude[0], -160.0)
        self.assertAlmostEqual(bathymetry.longitude[-1], -154.0)
        self.assertEqual(len(bathymetry.altitude), len(bathymetry.latitude))
        self.assertEqual(len(bathymetry.altitude[0]), len(bathymetry.longitude))

        # draw 3D surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Hawaii")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_bathy_grid_3d(self):
        """Draw 3D map of bathymetry around Malta Escarpment from netCDF file written by USML's data_grid class.

        Tests the abilities to:

        - Ready bathymetry from file extracted from ETOPO1 using ncks.
        - Read whole bathymetry file without specifying lat_range or lng_range.
        - Create 3D bathymetry surface plot

        Displays bathymetry as matplotlib surface plot. If the test runs correctly, then

        - The batymetry loads from netCDF file without errors.
        - Latitudes extend from 35.5N to 36.6N along the y-axis.
        - Longitudes extend from 15.25E to 16.25 along the x-axis.
        - Depths extend from 0 to -3500 meters.
        - Plot uses special color map with blue water, tan shallows, green land.
        - The west edge of the plot is shallow water, but it doesn't show up as green "land"
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/grid_2d_test.nc")
        print("reading {0}".format(filename))
        bathymetry = usml.netcdf.Bathymetry(filename)
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude

        # test the latitude and longtitude extents of the bathymetry
        # using a-priori knowledge of this file's contents taken from ncdump
        self.assertAlmostEqual(bathymetry.latitude[0], 35.5)
        self.assertAlmostEqual(bathymetry.latitude[-1], 36.5)
        self.assertAlmostEqual(bathymetry.longitude[0], 15.25)
        self.assertAlmostEqual(bathymetry.longitude[-1], 16.25)
        self.assertEqual(len(bathymetry.altitude), len(bathymetry.latitude))
        self.assertEqual(len(bathymetry.altitude[0]), len(bathymetry.longitude))

        # draw 3D surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Malta Escarpment")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_bathy_etopo_3d(self):
        """Draw 3D map of bathymetry around Hawaii from ETOPO1 database.

        Tests the abilities to

        - Read bathymetry from whole world ETOPO1 database.
        - Limit area by specifying lat_range or lng_range.
        - Create 3D bathymetry surface plot

        Displays bathymetry as matplotlib surface plot. If the test runs correctly, then

        - The batymetry loads from ETOPO1 database without errors.
        - The batymetry file loads correctly.
        - Plot looks identical to output of test_bathymetry_3d().
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "data/bathymetry/ETOPO1_Ice_g_gmt4.grd")
        print("reading {0}".format(filename))
        bathymetry = usml.netcdf.Bathymetry(filename, lat_range=(18, 23), lng_range=(-160, -154))
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude

        # test the latitude and longtitude extents of the bathymetry
        # using a-priori knowledge of this file's contents taken from ncdump
        self.assertAlmostEqual(bathymetry.latitude[0], 18.0)
        self.assertAlmostEqual(bathymetry.latitude[-1], 23.0)
        self.assertAlmostEqual(bathymetry.longitude[0], -160.0)
        self.assertAlmostEqual(bathymetry.longitude[-1], -154.0)
        self.assertEqual(len(bathymetry.altitude), len(bathymetry.latitude))
        self.assertEqual(len(bathymetry.altitude[0]), len(bathymetry.longitude))

        # draw 3D surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Hawaii")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_bathy_etopo_2d(self):
        """Loads 2D slice of bathymetry around Hawaii from ETOPO1 database.

        The plot_bathymetry_2d() function uses the pyproj library to estimate latitude/longitude for a list of ranges
        along a bearing of 45 degrees from 19N 159W. It uses scipy.interpolate to estimate the depth at each of those
        latitude/longitude points. Depth is plotted as a function of range. If the test runs correctly, then

        - Read bathymetry from whole world ETOPO1 database.
        - Ranges extend from 0 to 200 km along the x-axis.
        - Depths extend from 160W to 154W along the x-axis.
        - A minimum depth of about 3768 meters occurs at a range of 153.8 km.

        Note that there are no automatic assertions in this test.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "data/bathymetry/ETOPO1_Ice_g_gmt4.grd")
        print("reading {0}".format(filename))
        bathymetry = usml.netcdf.Bathymetry(filename, lat_range=(18, 23), lng_range=(-160, -154))
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude

        # draw 2D depth vs. range plot
        fig, ax = plt.subplots()
        line = usml.plot.plot_bathymetry_2d(ax, bathymetry, latitude=19, longitude=-159, bearing=45,
                                            ranges=np.linspace(start=0, stop=200e3, num=1001))
        ax.set_xlabel("Range (km)")
        ax.set_ylabel("Depth (m)")
        ax.set_ylim(top=0)
        ax.grid(visible=True)
        ax.set_title("ETOPO1 Bathymetry from 19N 159W at 45 degrees")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_file(self):
        """Loads in-situ ocean profile in Florida Straits from netCDF file.

        Displays sea surface temperature as matplotlib surface plot in a top-down view.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/flstrts_temperature.nc")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename)
        x, y = np.meshgrid(profile.longitude, profile.latitude)
        v = profile.data[0, :, :]

        # draw surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = ax.plot_surface(x, y, v, linewidth=0, cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("WOA09 Sea Surface Temperature in Florida Straits")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Temp (C)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_woa(self):
        """Loads in-situ ocean profile in Florida Straits from World Ocean Atlas database.

        Displays sea surface temperature as matplotlib surface plot in a top-down view.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "data/woa09/temperature_monthly_1deg.nc")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename, lat_range=(20, 30), lng_range=(279, 289))
        x, y = np.meshgrid(profile.longitude, profile.latitude)
        v = profile.data[6, 0, :, :]

        # draw surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = ax.plot_surface(x, y, v, linewidth=0, cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("WOA09 Sea Surface Temperature in Florida Straits")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Temp (C)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_hycom(self):
        """Test the ability to load a  3D profile file downloaded from the HYCOM.org web site.
            - site = https://ncss.hycom.org/thredds/ncss/grid/GLBv0.08/expt_93.0/ts3z/dataset.html
            - type = NetcdfSubset
            - latitudes 25.9-27.1
            - longitudes 279.9-281.1
            - time 2019-10-18T10:00:00Z
        These files have a few differences from the other NetCDF files we have worked with in the past.
            - Both the water_temp and salinity are contained in the same file, so the variables must be looked up by
              name.
            - The water_temp and salinity data are stored as scaled short integers, where all other file types to date
              have used floating point values.
        Displays sea surface temperature as matplotlib surface plot in a top-down view.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/hycom_ts3z.nc4")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename, profile_name="water_temp")
        x, y = np.meshgrid(profile.longitude - 360.0, profile.latitude)
        v = profile.data[0, :, :]

        # draw surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = ax.plot_surface(x, y, v, linewidth=0, cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("WOA09 Sea Surface Temperature in Florida Straits")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Temp (C)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_grid(self):
        """Test the ability to load a 3D profile file stored by USML's data_grid class.

        Displays sea surface temperature as matplotlib surface plot in a top-down view.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/hycom_grid.nc")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename)
        x, y = np.meshgrid(profile.longitude, profile.latitude)
        v = profile.data[0, :, :]

        # draw surface plot
        fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
        surface = ax.plot_surface(x, y, v, linewidth=0, cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("WOA09 Sea Surface Temperature in Florida Straits")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Temp (C)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_eigenrays(self):
        """Loads USML eigenrays from netCDF file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "eigenrays/test/create_eigenray.nc")
        print("reading {0}".format(filename))
        eigenray_list = usml.netcdf.EigenrayList(filename)

        # check that header arrays are the right size
        self.assertEqual(eigenray_list.frequencies.shape, (1,))
        id_shape = eigenray_list.targetID.shape
        self.assertEqual(id_shape, (1, 1))
        self.assertEqual(eigenray_list.latitude.shape, id_shape)
        self.assertEqual(eigenray_list.latitude.shape, id_shape)
        self.assertEqual(eigenray_list.longitude.shape, id_shape)
        self.assertEqual(eigenray_list.altitude.shape, id_shape)
        self.assertEqual(len(eigenray_list.eigenrays), 1)
        self.assertEqual(len(eigenray_list.eigenrays[0]), 1)

        # check that eigenray arrays are the right size
        ray = eigenray_list.eigenrays[0][0]
        num_rays = 3
        self.assertEqual(ray.intensity.shape, (num_rays, 1))
        self.assertEqual(ray.phase.shape, (num_rays, 1))
        self.assertEqual(ray.travel_time.shape, (num_rays,))
        self.assertEqual(ray.source_de.shape, (num_rays,))
        self.assertEqual(ray.source_az.shape, (num_rays,))
        self.assertEqual(ray.target_de.shape, (num_rays,))
        self.assertEqual(ray.target_az.shape, (num_rays,))
        self.assertEqual(ray.surface.shape, (num_rays,))
        self.assertEqual(ray.bottom.shape, (num_rays,))
        self.assertEqual(ray.caustic.shape, (num_rays,))
        self.assertEqual(ray.upper.shape, (num_rays,))
        self.assertEqual(ray.lower.shape, (num_rays,))

    def test_eigenverbs(self):
        """Loads hard-coded USML eigenverbs from netCDF file and plots projection on ocean bottom.

        Displays eigenverbs as matplotlib Ellipse patches in a top-down view.
        If the test runs correctly, then

        - Blue eigenverb location dots are plotted for 8 ranges and 10 beargins from 0 to 90 degrees.
        - Eigenverb locations eminate from the sensor location at 36N 16E.
        - Black eigenverb ellipses are drawn for each location.
        - The long axis of each ellipse is parrallel to the bearing from the sensor location.
        - Each ellipse touches its neighbors in both range and bearing directions.
        - No assertions fail.

        Note that a change of coordinate system is required to correctly plot eigenverbs using matplotlib. The
        matplotlib's sense of height, width, and angle are reverved from those in USML.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "eigenverbs/test/create_eigenverbs.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.read(filename)
        verb_shape = verbs.travel_time.shape
        freq_shape = verbs.frequencies.shape

        # check that eigenverb arrays are the right size
        self.assertEqual(verbs.power.shape, (verb_shape[0], freq_shape[0]))
        self.assertEqual(verbs.length.shape, verb_shape)
        self.assertEqual(verbs.width.shape, verb_shape)
        self.assertEqual(verbs.latitude.shape, verb_shape)
        self.assertEqual(verbs.longitude.shape, verb_shape)
        self.assertEqual(verbs.altitude.shape, verb_shape)
        self.assertEqual(verbs.direction.shape, verb_shape)
        self.assertEqual(verbs.grazing.shape, verb_shape)
        self.assertEqual(verbs.sound_speed.shape, verb_shape)
        self.assertEqual(verbs.de_index.shape, verb_shape)
        self.assertEqual(verbs.az_index.shape, verb_shape)
        self.assertEqual(verbs.source_de.shape, verb_shape)
        self.assertEqual(verbs.source_az.shape, verb_shape)
        self.assertEqual(verbs.surface.shape, verb_shape)
        self.assertEqual(verbs.bottom.shape, verb_shape)
        self.assertEqual(verbs.caustic.shape, verb_shape)
        self.assertEqual(verbs.upper.shape, verb_shape)
        self.assertEqual(verbs.lower.shape, verb_shape)

        self.assertEqual(verbs.source_az.all(), verbs.direction.all())

        # draw projection of eigenverbs onto ocean bottom.
        fig, ax = plt.subplots()
        index = range(verb_shape[0])
        usml.plot.plot_eigenverbs_2d(ax, verbs, index)

        ax.axis("equal")
        ax.grid("on")
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title(testname)
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_eigenverb_pair(self):
        """Plots source and receiver eigenverbs on ocean bottom.

        Displays direct path eigenverbs as matplotlib Ellipse patches in a top-down view.
        If the test runs correctly, then

        - Source and receiver eigenverb location dots are plotted for multiple ranges and bearings from 0 to 360 deg.
        - Eigenverb locations eminate from the sensor locations at 36N 16E and 36.1N 16E.
        - Black eigenverb ellipses are drawn for each location.
        - Each ellipse touches its neighbors in both range and bearing directions.
        - The eigenverbs at long range have a width greater than their length.
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")
        fig, ax = plt.subplots()

        # load source eigenverb data from disk and plot it
        filename = os.path.join(self.USML_DIR, "sensors/test/src_eigenverbs_2_4.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.read(filename)
        index = np.where(verbs.surface == 0)
        usml.plot.plot_eigenverbs_2d(ax, verbs, index[0])

        # load source eigenverb data from disk and plot it
        filename = os.path.join(self.USML_DIR, "sensors/test/rcv_eigenverbs_2_4.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.read(filename)
        index = np.where(verbs.surface == 0)
        usml.plot.plot_eigenverbs_2d(ax, verbs, index[0])

        # record results
        ax.axis("equal")
        ax.grid("on")
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title(testname)
        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_fathometers(self):
        """Loads USML bistatic fathometers from netCDF file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "sensors/test/dirpaths_2_4.nc")
        print("reading {0}".format(filename))
        eigenray_list = usml.netcdf.EigenrayList(filename)

        # check that header arrays are the right size
        self.assertEqual(eigenray_list.frequencies.shape, (11,))
        num_freqs = eigenray_list.frequencies.shape[0]
        id_shape = eigenray_list.targetID.shape
        self.assertEqual(id_shape, (1, 1))
        self.assertEqual(eigenray_list.latitude.shape, id_shape)
        self.assertEqual(eigenray_list.latitude.shape, id_shape)
        self.assertEqual(eigenray_list.longitude.shape, id_shape)
        self.assertEqual(eigenray_list.altitude.shape, id_shape)
        self.assertEqual(len(eigenray_list.eigenrays), 1)
        self.assertEqual(len(eigenray_list.eigenrays[0]), 1)

        # check that eigenray arrays are the right size
        ray_list = eigenray_list.eigenrays[0][0]  # first target
        num_rays = ray_list.lower.shape[0]
        self.assertEqual(ray_list.intensity.shape, (num_rays, num_freqs))
        self.assertEqual(ray_list.phase.shape, (num_rays, num_freqs))
        self.assertEqual(ray_list.travel_time.shape, (num_rays,))
        self.assertEqual(ray_list.source_de.shape, (num_rays,))
        self.assertEqual(ray_list.source_az.shape, (num_rays,))
        self.assertEqual(ray_list.target_de.shape, (num_rays,))
        self.assertEqual(ray_list.target_az.shape, (num_rays,))
        self.assertEqual(ray_list.surface.shape, (num_rays,))
        self.assertEqual(ray_list.bottom.shape, (num_rays,))
        self.assertEqual(ray_list.caustic.shape, (num_rays,))
        self.assertEqual(ray_list.upper.shape, (num_rays,))
        self.assertEqual(ray_list.lower.shape, (num_rays,))

    def test_biverbs(self):
        """Loads USML bistatic eigenverbs from netCDF file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "sensors/test/biverbs_2_4.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.read(filename)
        verb_shape = verbs.travel_time.shape
        freq_shape = verbs.frequencies.shape

        # check that eigenverb arrays are the right size
        self.assertEqual(verbs.power.shape, (verb_shape[0], freq_shape[0]))
        self.assertEqual(verbs.duration.shape, verb_shape)
        self.assertEqual(verbs.de_index.shape, verb_shape)
        self.assertEqual(verbs.az_index.shape, verb_shape)
        self.assertEqual(verbs.source_de.shape, verb_shape)
        self.assertEqual(verbs.source_az.shape, verb_shape)
        self.assertEqual(verbs.source_surface.shape, verb_shape)
        self.assertEqual(verbs.source_bottom.shape, verb_shape)
        self.assertEqual(verbs.source_caustic.shape, verb_shape)
        self.assertEqual(verbs.source_upper.shape, verb_shape)
        self.assertEqual(verbs.source_lower.shape, verb_shape)
        self.assertEqual(verbs.receiver_surface.shape, verb_shape)
        self.assertEqual(verbs.receiver_bottom.shape, verb_shape)
        self.assertEqual(verbs.receiver_caustic.shape, verb_shape)
        self.assertEqual(verbs.receiver_upper.shape, verb_shape)
        self.assertEqual(verbs.receiver_lower.shape, verb_shape)

    def test_wavefront(self):
        """Loads USML wavefronts from netCDF file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "waveq3d/test/eigenverb_demo_wave.nc")
        print("reading {0}".format(filename))
        wavefront = usml.netcdf.read(filename)
        time_shape = wavefront.travel_time.shape
        de_shape = wavefront.source_de.shape
        az_shape = wavefront.source_az.shape
        grid_shape = (time_shape[0], de_shape[0], az_shape[0])
        self.assertEqual(wavefront.latitude.shape, grid_shape)
        self.assertEqual(wavefront.longitude.shape, grid_shape)
        self.assertEqual(wavefront.altitude.shape, grid_shape)
        self.assertEqual(wavefront.surface.shape, grid_shape)
        self.assertEqual(wavefront.bottom.shape, grid_shape)
        self.assertEqual(wavefront.caustic.shape, grid_shape)
        self.assertEqual(wavefront.upper.shape, grid_shape)
        self.assertEqual(wavefront.lower.shape, grid_shape)
        self.assertEqual(wavefront.on_edge.shape, grid_shape)


if __name__ == '__main__':
    unittest.main()
