"""Compute solutions for n^2 linear environment
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np

import usml.pedersen


def compare_pedersen(self, testname: str, source_depth: float, source_angles: np.ndarray):
    """Compare pedersen solutions in cartesian and spherical coordinate, with earth flattening correction.

    Test fails if the maximum difference between cycle ranges is greater than 0.5 m or if the maximum difference
    between cycle times is greater than 0.5 msec.

    :param testname:            name of the test to use for saving files.
    :param source_depth:        source depth expressed as distance down from ocean surface
    :param source_angles:       array of launch angles from source, up is positive
    """
    cartesian = usml.pedersen.PedersenCartesian()
    spherical = usml.pedersen.PedersenSpherical()
    cart_cycle_ranges, cart_cycle_times = cartesian.analytic_cycle(source_depth, source_angles)
    sphr_cycle_ranges, sphr_cycle_times, sphr_vertex_speed, sphr_vertex_radius \
        = spherical.analytic_cycle(source_depth, source_angles)

    fig, (ax1, ax2) = plt.subplots(2, 1)

    diff = sphr_cycle_ranges - cart_cycle_ranges
    ax1.plot(source_angles, diff)
    ax1.grid(True)
    ax1.set_title(testname)
    ax1.set_ylabel("Cycle Range Error (m)")

    diff = sphr_cycle_times - cart_cycle_times
    ax2.plot(source_angles, diff*1e3)
    ax2.grid(True)
    ax2.set_xlabel("Source Angle (deg)")
    ax2.set_ylabel("Cycle Time Error (msec)")

    print("saving {0}.png".format(testname))
    plt.savefig(testname)
    plt.close()

    self.assertLess(np.abs(cart_cycle_ranges - sphr_cycle_ranges).max(), 0.5)
    self.assertLess(np.abs(cart_cycle_times - cart_cycle_times).max(), 0.5e-3)


class TestPedersen(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_analytic_deep(self):
        """Compare pedersen solutions in cartesian and spherical coordinate for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        source_angles = np.arange(20.0, 60.0, 1.0)
        compare_pedersen(self, testname, source_depth, source_angles)

    def test_analytic_shallow(self):
        """Compare pedersen solutions in cartesian and spherical coordinate for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 2.0) # 0.1)
        compare_pedersen(self, testname, source_depth, source_angles)
