"""Plot outputs from units tests for usml.waveq3d package.
"""
import inspect
import os
import unittest

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pyproj

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
        plt.show()
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


if __name__ == '__main__':
    unittest.main()
