import inspect
import os
import unittest

import usml.netcdf


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
        """Load source eigenverbs and check format."""
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

        # # draw projection of eigensrc_eigenverbs onto ocean bottom
        # fig, ax = plt.subplots(figsize=(8, 6))
        # plt.scatter(verbs.longitude, verbs.latitude, 20)
        # earth_radius = 6371e3
        # for n in range(verb_shape[0]):
        #     x = verbs.longitude[n]
        #     y = verbs.latitude[n]
        #     width = 2.0 * np.degrees(verbs.width[n] / earth_radius)
        #     height = 2.0 * np.degrees(verbs.length[n] / earth_radius)
        #     angle = verbs.direction[n]
        #     ellipse = ptch.Ellipse((x, y), width=width, height=height, angle=angle, facecolor="none", edgecolor="black")
        #     ax.add_patch(ellipse)
        # ax.axis("equal")
        # ax.grid("on")
        # ax.set_xlabel("Longitude (deg)")
        # ax.set_ylabel("Latitude (deg)")
        # ax.set_title(testname)
        # print("saving {0}.png".format(testname))
        # plt.savefig(testname)
        # plt.show()


if __name__ == '__main__':
    unittest.main()
