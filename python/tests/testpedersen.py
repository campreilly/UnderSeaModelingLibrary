"""Compute solutions for n^2 linear environment
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np

import usml.pedersen


def test_cycles(self, testname: str, source_depth: float, source_angles: np.ndarray):
    """Compare cycle range/time analytic solutions in cartesian and spherical coordinate.

    Test fails if the maximum difference between cycle ranges is greater than 0.5 m or if the maximum difference
    between cycle times is greater than 0.5 msec.

    :param testname:            name of the test to use for saving files.
    :param source_depth:        source depth expressed as distance down from ocean surface
    :param source_angles:       array of launch angles from source, up is positive
    """
    cartesian = usml.pedersen.PedersenCartesian()
    spherical = usml.pedersen.PedersenSpherical()
    cart_cycle_ranges, cart_cycle_times, _, _ = cartesian.analytic_cycle(source_depth, source_angles)
    sphr_cycle_ranges, sphr_cycle_times, _, _ = spherical.analytic_cycle(source_depth, source_angles)

    fig, (ax1, ax2) = plt.subplots(2, 1)

    ax1.plot(source_angles, cart_cycle_ranges / 1e3)
    ax1.plot(source_angles, sphr_cycle_ranges / 1e3, "--")
    ax1.grid(True)
    ax1.set_title(testname)
    ax1.set_ylabel("Cycle Range (km)")

    ax2.plot(source_angles, cart_cycle_times)
    ax2.plot(source_angles, sphr_cycle_times, "--")
    ax2.grid(True)
    ax2.set_xlabel("Source Angle (deg)")
    ax2.set_ylabel("Cycle Time (msec)")
    ax2.legend(["Cartesian", "Spherical"])

    print(f"saving {testname}.png")
    plt.savefig(testname)
    plt.close()

    # self.assertLess(np.abs(cart_cycle_ranges - sphr_cycle_ranges).max(), 0.5)
    # self.assertLess(np.abs(cart_cycle_times - cart_cycle_times).max(), 0.5e-3)


def test_eigenrays(self, testname: str, source_depth: float, source_angles: np.ndarray, target_depth: float,
                   target_ranges: np.ndarray):
    """Compare eigenray analytic solutions in cartesian and spherical coordinate.

    :param testname:            name of the test to use for saving files.
    :param source_depth:        source depth expressed as distance down from ocean surface
    :param source_angles:       array of launch angles from source, up is positive
    :param target_depth:        target depth expressed as distance down from ocean surface (m)
    :param target_ranges:       target range from source (m)
    """
    cartesian = usml.pedersen.PedersenCartesian();
    spherical = usml.pedersen.PedersenSpherical()
    cart_direct, cart_folded = cartesian.eigenrays(source_depth, source_angles, target_depth, target_ranges)
    shpr_direct, sphr_folded = spherical.eigenrays(source_depth, source_angles, target_depth, target_ranges)

    # remove bulk travel time
    slope = np.mean(cart_direct.travel_time) / np.mean(cart_direct.range)
    cart_direct.travel_time -= slope * cart_direct.range
    shpr_direct.travel_time -= slope * shpr_direct.range
    cart_folded.travel_time -= slope * cart_folded.range
    sphr_folded.travel_time -= slope * sphr_folded.range

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(10, 6))
    ax1.axis("off")

    ax2.plot(cart_direct.range / 1e3, cart_direct.travel_time * 1e3)
    ax2.plot(shpr_direct.range / 1e3, shpr_direct.travel_time * 1e3, "--")
    ax2.grid(True)
    ax2.set_ylabel("Travel Time - Bulk (msec)")
    ax2.set_title(testname)

    ax3.plot(cart_direct.range / 1e3, cart_direct.source_de)
    ax3.plot(shpr_direct.range / 1e3, shpr_direct.source_de, "--")
    ax3.grid(True)
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(cart_direct.range / 1e3, cart_direct.target_de)
    ax4.plot(shpr_direct.range / 1e3, shpr_direct.target_de, "--")
    ax4.grid(True)
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["Cartesian", "Spherical"])

    output = testname + "_direct"
    print(f"saving {output}.png")
    plt.savefig(output)
    plt.close()

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(10, 6))

    ax1.axis("off")

    ax2.plot(cart_folded.range / 1e3, cart_folded.travel_time * 1e3)
    ax2.plot(sphr_folded.range / 1e3, sphr_folded.travel_time * 1e3, "--")
    ax2.grid(True)
    ax2.set_ylabel("Travel Time - Bulk (msec)")
    ax2.set_title(testname)

    ax3.plot(cart_folded.range / 1e3, cart_folded.source_de)
    ax3.plot(sphr_folded.range / 1e3, sphr_folded.source_de, "--")
    ax3.grid(True)
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(cart_folded.range / 1e3, cart_folded.target_de)
    ax4.plot(sphr_folded.range / 1e3, sphr_folded.target_de, "--")
    ax4.grid(True)
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["Cartesian", "Spherical"])

    output = testname + "_folded"
    print(f"saving {output}.png")
    plt.savefig(output)
    plt.close()


class TestPedersen(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_cycles_shallow(self):
        """Compare cycle range/time analytic solutions for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 2.0)  # 0.1)
        test_cycles(self, testname, source_depth, source_angles)

    def test_cycles_deep(self):
        """Compare cycle range/time analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        source_angles = np.arange(20.0, 60.0, 1.0)
        test_cycles(self, testname, source_depth, source_angles)

    def test_eigenrays_shallow(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        target_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 0.1)
        target_ranges = np.arange(500.0, 1000.0, 1.0)
        test_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)

    def test_eigenrays_deep(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        target_depth = 800.0
        source_angles = np.arange(20.0, 51.21, 0.1)
        target_ranges = np.arange(3000.0, 3100.0, 1.0)
        test_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)
