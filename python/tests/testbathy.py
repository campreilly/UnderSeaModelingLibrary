"""Test ability to read bathymetry from disk.
"""
import inspect
import os
import unittest

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.plot


class TestBathy(unittest.TestCase):
    """Unit tests for bathymetry reading classes in usml.netcdf module. Reads data from and write plots to the USML
    netcdf/test directory.
    """
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

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
        print(f"reading {filename}")
        bathymetry = usml.netcdf.Bathymetry(filename)

        # test the latitude and longitude extents of the bathymetry
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

        output = os.path.join(self.USML_DIR, f"{testname}.png")
        print(f"saving {output}")
        plt.savefig(output)
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
        print(f"reading {filename}")
        bathymetry = usml.netcdf.Bathymetry(filename)

        # test the latitude and longitude extents of the bathymetry
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

        output = os.path.join(self.USML_DIR, f"netcdf/test/{testname}.png")
        print(f"saving {output}")
        plt.savefig(output)
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
        print(f"reading {filename}")
        bathymetry = usml.netcdf.Bathymetry(filename, lat_range=(18, 23), lng_range=(-160, -154))

        # test the latitude and longitude extents of the bathymetry
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

        output = os.path.join(self.USML_DIR, f"netcdf/test/{testname}.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_bathy_etopo_2d(self):
        """Loads 2D slice of bathymetry around Hawaii from ETOPO1 database.

        The plot_bathymetry_2d() function uses the pyproj library to estimate latitude/longitude for a list of ranges
        along a bearing of 45 degrees from 19N 159W. It uses scipy.interpolate() to estimate the depth at each of those
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
        print(f"reading {filename}")
        bathymetry = usml.netcdf.Bathymetry(filename, lat_range=(18, 23), lng_range=(-160, -154))

        # draw 2D depth vs. range plot
        fig, ax = plt.subplots()
        usml.plot.plot_bathymetry_2d(ax, bathymetry, latitude=19, longitude=-159, bearing=45,
                                     ranges=np.linspace(start=0, stop=200e3, num=1001))
        ax.set_xlabel("Range (km)")
        ax.set_ylabel("Depth (m)")
        ax.set_ylim(top=0)
        ax.grid(visible=True)
        ax.set_title("ETOPO1 Bathymetry from 19N 159W at 45 degrees")

        output = os.path.join(self.USML_DIR, f"netcdf/test/{testname}.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()


if __name__ == '__main__':
    unittest.main()
