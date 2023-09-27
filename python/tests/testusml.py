import inspect
import os
import unittest

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np
import unitreport

import usml.netcdf


class UsmlTest(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_bathymetry(self):
        """Loads bathymetry around Hawaii from netCDF file.

        Displays it as matplotlib surface plot in a top-down view. If the test runs correctly, then

            - Latitudes will range from 18N to 23N along the y-axis
            - Longitudes will range from 160W to 154W along the x-axis
            - The big island of Hawaii will be in the south-east corner.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/etopo_cmp.nc")
        print("reading {0}".format(filename))
        bathymetry = usml.netcdf.Bathymetry(filename)
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude

        # draw surface plot
        fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={'projection': '3d'})
        surface = ax.plot_surface(x, y, z, linewidth=0, cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel("Longitude (deg)")
        ax.set_ylabel("Latitude (deg)")
        ax.set_title("ETOPO1 Bathymetry Around Hawaii")
        cbar = fig.colorbar(surface)
        cbar.ax.set_title("Depth (m)")
        plt.savefig(testname)

    def test_profile(self):
        """Loads ocean profile in Florida Straits from netCDF file.

        Displays sea surface temperature as matplotlib surface plot in a top-down view. If the test runs correctly, then

            - Latitudes will range from 18N to 23N along the y-axis
            - Longitudes will range from 160W to 154W along the x-axis
            - The big island of Hawaii will be in the south-east corner.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "netcdf/test/flstrts_temperature.nc")
        print("reading {0}".format(filename))
        profile = usml.netcdf.Profile(filename)
        x, y = np.meshgrid(profile.longitude, profile.latitude)
        v = profile.altitude[0, 0, :, :]

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
        plt.savefig(testname)


if __name__ == '__main__':
    unitreport.main()
