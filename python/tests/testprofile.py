"""Test ability to read ocean profiles files from disk.
"""
import inspect
import os
import unittest

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.plot


class TestProfile(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

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
        """Test the ability to load a  3D profile file downloaded from the HYCOM.org website.
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


if __name__ == '__main__':
    unittest.main()
