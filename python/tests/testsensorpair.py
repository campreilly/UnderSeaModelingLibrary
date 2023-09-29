import inspect
import os
import unittest

import matplotlib.patches as ptch
import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf


def plot_verbs(testname: str, verbs: usml.netcdf.EigenverbList, index):
    """Draw projection of eigenverbs onto ocean bottom.

    Arguments:
        testname            name of the test
        verbs               list of eigenverbs to plot
        index               subset of indices to plot
    """
    fig, ax = plt.subplots(figsize=(8, 6))
    plt.scatter(verbs.longitude[index], verbs.latitude[index], 20)
    for n in index:  # range(verb_shape[0]):
        x = verbs.longitude[n]
        y = verbs.latitude[n]
        scale = 6371e3*np.cos(np.radians(y))
        height = 2.0 * np.degrees(verbs.width[n] / scale)
        width = 2.0 * np.degrees(verbs.length[n] / scale)
        angle = 90 - verbs.direction[n]
        ellipse = ptch.Ellipse((x, y), width=width, height=height, angle=angle, facecolor="none", edgecolor="black")
        ax.add_patch(ellipse)
    ax.axis("equal")
    ax.grid("on")
    ax.set_xlabel("Longitude (deg)")
    ax.set_ylabel("Latitude (deg)")
    ax.set_title(testname)
    print("saving {0}.png".format(testname))
    plt.savefig(testname)
    plt.show()


class TestBistaticPair(unittest.TestCase):
    """Tests production of sensor_pair propgation data

     The bistatic_test/update_wavefront_data unit test in the sensor package tests the ability to control the
     production of sensor_pair objects with the multistatic(), is_source(), is_receiver(), min_range(),
     and compute_reverb() methods of the sensor_model. Uses a simple isovelocity ocean with a 2000m depth and the
     following sensors:

     - sensor #1 = south side, monostatic, pairs are 1_1
     - sensor #2 = center, pairs are 2_2, 2_4, 2_5
     - sensor #3 = north side, source only, pairs are 3_2, 3_4, 3_5,
     - sensor #4 = below #3, receiver only
     - sensor #5 = east side, min range 1m, pairs are 5_2, 5_4

     The USMl unit test uses a wavefront_generator to automatically to compute the bistatic direct path eigenrays (
     fathometers) and bistatic eigenverbs between these sensors, and writes those results to netCDF files
    """
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_src_eigenverbs(self):
        """Load source eigenverbs and plots projection on ocean bottom.

        Displays direct path eigenverbs as matplotlib Ellipse patches in a top-down view.
        If the test runs correctly, then

        - Blue eigenverb location dots are plotted for multiple ranges and bearings from 0 to 360 degrees.
        - Eigenverb locations eminate from the sensor location at 36N 16E.
        - Black eigenverb ellipses are drawn for each location.
        - Each ellipse touches its neighbors in both range and bearing directions.
        - The eigenverbs at long range have a width greater than their length.
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "sensors/test/src_eigenverbs_2_4.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.EigenverbList(filename)
        verb_shape = verbs.travel_time.shape
        freq_shape = verbs.frequency.shape

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

        index = np.where(verbs.surface == 0)
        plot_verbs(testname, verbs, index[0])

    def test_rcv_eigenverbs(self):
        """Load receiver eigenverbs and plots projection on ocean bottom.

        Displays direct path eigenverbs as matplotlib Ellipse patches in a top-down view.
        If the test runs correctly, then

        - Blue eigenverb location dots are plotted for multiple ranges and bearings from 0 to 360 degrees.
        - Eigenverb locations eminate from the sensor location at 36N 16E.
        - Black eigenverb ellipses are drawn for each location.
        - Each ellipse touches its neighbors in both range and bearing directions.
        - The eigenverbs at long range have a width greater than their length.
        - No assertions fail.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # load data from disk
        filename = os.path.join(self.USML_DIR, "sensors/test/rcv_eigenverbs_2_4.nc")
        print("reading {0}".format(filename))
        verbs = usml.netcdf.EigenverbList(filename)
        verb_shape = verbs.travel_time.shape
        freq_shape = verbs.frequency.shape

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

        index = np.where(verbs.surface == 0)
        plot_verbs(testname, verbs, index[0])

if __name__ == '__main__':
    unittest.main()
