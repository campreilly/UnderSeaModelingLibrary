"""Test ability to read netCDF files created by USML.
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.plot


class TestNetCDF(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

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
