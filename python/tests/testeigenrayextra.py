"""Unit tests for USML eigenray_extra study
"""
import inspect
import os
import unittest

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.pedersen
import usml.plot


class TestEigenrayExtra(unittest.TestCase):
    """Unit tests for USML eigenray_extra study"""
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "studies", "eigenray_extra")

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

    def test_lloyds_errors(self):
        """Illustrates the errors in the wavefront near the ocean surface.

        Compares modeled and analytic solutions for travel times and ray path angles, for the spherical equivalent of
        Lloyd’s mirror, at a variety of depths and ranges. This test used an isovelocity speed of sound of 1500 m/s,
        a frequency of 2000 Hz, a source depth of 200 m, target depths from 0 to 1000 m, 181 tangent spaced
        depression/elevation angles, 1 deg spaced azimuth angles from -4 to +4 deg, and a time step of 100 ms. The
        maximum range was limited to ∆θ = 0.8 deg to ensure that only a single surface reflected path was produced at
        each target location.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "eigenray_lloyds.nc")
        model = usml.netcdf.EigenrayList(filename)
        filename = os.path.join(self.USML_DIR, "eigenray_lloyds_analytic.nc")
        analytic = usml.netcdf.EigenrayList(filename)
        latitude = analytic.latitude[:, 1] - 45.0
        altitude = analytic.altitude[1, :]
        window = latitude > 0.1
        path_label = ["direct", "reflected"]

        # plot errors in time and angles for this path
        for path in range(2):
            print("*** " + path_label[path] + " path")
            travel_time = np.zeros_like(analytic.latitude)
            source_de = np.zeros_like(analytic.latitude)
            target_de = np.zeros_like(analytic.latitude)
            for nrow in range(len(latitude)):
                for ncol in range(len(altitude)):
                    mray = model.eigenrays[nrow][ncol]
                    aray = analytic.eigenrays[nrow][ncol]
                    travel_time[nrow, ncol] = np.abs(mray.travel_time[path] - aray.travel_time[path])
                    source_de[nrow, ncol] = np.abs(mray.source_de[path] - aray.source_de[path])
                    target_de[nrow, ncol] = np.abs(mray.target_de[path] - aray.target_de[path])

            fig, (ax1, ax2, ax3) = plt.subplots(3)
            ax1.semilogy(latitude, travel_time)
            ax1.grid(True)
            ax1.set_ylim(1e-12, 1e-4)
            ax1.set_ylabel("Travel Time Errors (s)")
            print("Max Travel Time Errors = {:.5f} (msec)".format(np.max(travel_time[window, :]) * 1e3))

            ax2.semilogy(latitude, source_de)
            ax2.grid(True)
            ax2.set_ylim(1e-10, 1e-2)
            ax2.set_ylabel("Source D/E Errors (deg)")
            print("Max Source D/E Errors = {:.4f} (deg)".format(np.max(source_de[window, :]) * 1e3))

            ax3.semilogy(latitude, target_de)
            ax3.grid(True)
            ax2.set_ylim(1e-10, 1e-2)
            ax3.set_ylabel("Target D/E Errors (deg)")
            ax3.set_xlabel("Latitude Change (deg)")
            ax3.legend(["0 m", "10 m", "100 m", "1000 m"], fontsize="8")
            print("Max Target D/E Errors = {:.4f} (deg)".format(np.max(target_de[window, :]) * 1e3))

            output = testname + "_" + path_label[path]
            plt.suptitle(output)
            filename = os.path.join(self.USML_DIR, output + ".png")
            print(f"saving {filename}")
            plt.savefig(filename)
            plt.close()

    def test_lloyds_zoom(self):
        """Illustrates the fold in the wavefront near the ocean surface.

        The plots in this test show how the distance from a wavefront edge to the surface can get large at short
        ranges. This causes inaccuracies in the model by forcing an increased level of extrapolation outside of the
        wavefront. At longer ranges, this source of inaccuracy is automatically reduced by the fact that the edges of
        the ray families get closer to the surface as the rays become more horizontal.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load wavefront data from disk
        filename = os.path.join(self.USML_DIR, "eigenray_lloyds_wave.nc")
        wavefront = usml.netcdf.read(filename)
        src_latitude = wavefront.latitude[0, 0, 0]

        # extract ray position data for AZ=0
        az_list = np.asarray(0.0)
        _, _, az_index = np.intersect1d(az_list, wavefront.source_az, return_indices=True)
        latitude = np.squeeze(wavefront.latitude[:, :, az_index], axis=2) - src_latitude
        altitude = np.squeeze(wavefront.altitude[:, :, az_index], axis=2)
        surface = np.squeeze(wavefront.surface[:, :, az_index], axis=2)
        ray_count = surface.shape[1]

        # plot direct paths, surface reflected, and discontinuous parts of path
        fig, ax = plt.subplots()
        for ray_num in range(ray_count - 1):
            lat = latitude[:, ray_num]
            alt = altitude[:, ray_num]
            srf = surface[:, ray_num]
            n = np.where(srf == 0)
            m = np.where(srf == 1)
            k = np.argmax(np.diff(srf, append=srf[-1]))
            k = range(k, k + 2)
            ax.plot(lat[n], alt[n], 'b')
            ax.plot(lat[m], alt[m], 'r')
            ax.plot(lat[k], alt[k], 'g--')
        ax.grid(True)
        ax.set_xlabel('Latitude Change (deg)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 0.025)
        ax.set_ylim(-100, 0)
        ax.legend(["Direct Path", "Surface Reflected", "Discontinuity"], loc='lower right', framealpha=1.0)

        filename = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {filename}")
        plt.savefig(filename)
        plt.close()
