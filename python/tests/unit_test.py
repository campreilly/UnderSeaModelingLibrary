import unittest

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import cm

import usml.netcdf


class unit_test(unittest.TestCase):
    def test_bathymetry(self):
        """Loads bathymetry around Hawaii from netCDF file and displays it as matplotlib surface plot in
        a top-down view. If the test runs correctly, then
            - Latitudes will range from 18N to 23N along the y-axis
            - Longitudes will range from 160W to 154W along the x-axis
            - The big island of Hawaii will be in the south-east corner.
        """
        bathymetry = usml.netcdf.Bathymetry("etopo_cmp.nc")
        x, y = np.meshgrid(bathymetry.longitude, bathymetry.latitude)
        z = bathymetry.altitude
        fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
        surf = ax.plot_surface(x, y, z, linewidth=0,
                               cmap=cm.coolwarm, antialiased=False)
        ax.set_proj_type('ortho')
        ax.view_init(90, -90)
        ax.set_xlabel('Longitude (deg)')
        ax.set_ylabel('Latitude (deg)')
        ax.set_title('ETOPO1 Bathymetry Around Hawaii')
        plt.show()


if __name__ == '__main__':
    unittest.main()
