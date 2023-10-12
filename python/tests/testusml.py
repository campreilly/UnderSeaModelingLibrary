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
            fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
            usml.plot.plot_beampattern_3d(ax, pattern)
            ax.set_title(output)
            ax.set_xlabel('x')
            ax.set_ylabel('y')
            ax.set_zlabel('z')
            print("saving {0}.png".format(output))
            plt.savefig(output)

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

        fig, ax3 = plt.subplots(subplot_kw={'projection': 'polar'})
        ax3.plot(angle, np.ones(angle.shape), 'o')
        ax3.set_title("std = {:.3f} deg, min diff = {:.3f} deg".format(np.std(angle), np.min(np.diff(angle))))
        print("saving {0}.png".format(testname + "_polar"))
        plt.savefig(testname + "_polar")

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
        fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Hawaii")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)

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
        fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Malta Escarpment")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)

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
        fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
        surface = usml.plot.plot_bathymetry_3d(ax, bathymetry)
        ax.view_init(70, -100)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Hawaii")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)

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
        fig, ax = plt.subplots(figsize=(8, 6))
        line = usml.plot.plot_bathymetry_2d(ax, bathymetry, latitude=19, longitude=-159, bearing=45,
                                            ranges=np.linspace(start=0, stop=200e3, num=1001))
        ax.set_xlabel("Range (km)")
        ax.set_ylabel("Depth (m)")
        ax.set_ylim(top=0)
        ax.grid(visible=True)
        ax.set_title("ETOPO1 Bathymetry from 19N 159W at 45 degrees")
        print("saving {0}.png".format(testname))
        plt.savefig(testname)

    def test_profile_file(self):
        """Loads in-situ ocean profile in Florida Straits from netCDF file.

        Displays sea surface temperature as matplotlib surface plot in a top-down view. If the test runs correctly, then
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/flstrts_temperature.nc")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename)
        x, y = np.meshgrid(profile.longitude, profile.latitude)
        v = profile.data[0, 0, :, :]

        # draw surface plot
        fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
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
        fig, ax = plt.subplots(figsize=(8, 6))
        index = range(verb_shape[0])
        usml.plot.plot_eigenverbs_2d(ax, verbs, index)

        ax.axis("equal")
        ax.grid("on")
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title(testname)
        print("saving {0}.png".format(testname))
        plt.savefig(testname)

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
        fig, ax = plt.subplots(figsize=(8, 6))

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
