"""Compute solutions for n^2 linear environment
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

    fig, (ax1, ax2) = plt.subplots(2, figsize=(8, 6), layout="constrained")

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

    self.assertLess(np.abs(cart_cycle_ranges - sphr_cycle_ranges).max(), 0.5)
    self.assertLess(np.abs(cart_cycle_times - cart_cycle_times).max(), 0.5e-3)


def test_eigenrays(self, testname: str, source_depth: float, source_angles: np.ndarray, target_depth: float,
                   target_ranges: np.ndarray):
    """Compare eigenray analytic solutions in cartesian and spherical coordinate.

    :param testname:            name of the test to use for saving files.
    :param source_depth:        source depth expressed as distance down from ocean surface
    :param source_angles:       array of launch angles from source, up is positive
    :param target_depth:        target depth expressed as distance down from ocean surface (m)
    :param target_ranges:       target range from source (m)
    """
    cartesian = usml.pedersen.PedersenCartesian()
    spherical = usml.pedersen.PedersenSpherical()
    cart_direct, cart_folded = cartesian.eigenrays(source_depth, source_angles, target_depth, target_ranges)
    shpr_direct, sphr_folded = spherical.eigenrays(source_depth, source_angles, target_depth, target_ranges)

    # remove bulk travel time
    slope = np.mean(cart_direct.travel_time) / np.mean(cart_direct.horz_range)
    cart_direct.travel_time -= slope * cart_direct.horz_range
    shpr_direct.travel_time -= slope * shpr_direct.horz_range
    cart_folded.travel_time -= slope * cart_folded.horz_range
    sphr_folded.travel_time -= slope * sphr_folded.horz_range

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(8, 6), layout="constrained")
    ax1.axis("off")

    ax2.plot(cart_direct.horz_range / 1e3, cart_direct.travel_time * 1e3)
    ax2.plot(shpr_direct.horz_range / 1e3, shpr_direct.travel_time * 1e3, "--")
    ax2.grid(True)
    ax2.set_ylabel("Travel Time - Bulk (msec)")
    ax2.set_title(testname)

    ax3.plot(cart_direct.horz_range / 1e3, cart_direct.source_de)
    ax3.plot(shpr_direct.horz_range / 1e3, shpr_direct.source_de, "--")
    ax3.grid(True)
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(cart_direct.horz_range / 1e3, cart_direct.target_de)
    ax4.plot(shpr_direct.horz_range / 1e3, shpr_direct.target_de, "--")
    ax4.grid(True)
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["Cartesian", "Spherical"])

    output = testname + "_direct"
    print(f"saving {output}.png")
    plt.savefig(output)
    plt.close()

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(8, 6), layout="constrained")

    ax1.axis("off")

    ax2.plot(cart_folded.horz_range / 1e3, cart_folded.travel_time * 1e3)
    ax2.plot(sphr_folded.horz_range / 1e3, sphr_folded.travel_time * 1e3, "--")
    ax2.grid(True)
    ax2.set_ylabel("Travel Time - Bulk (msec)")
    ax2.set_title(testname)

    ax3.plot(cart_folded.horz_range / 1e3, cart_folded.source_de)
    ax3.plot(sphr_folded.horz_range / 1e3, sphr_folded.source_de, "--")
    ax3.grid(True)
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(cart_folded.horz_range / 1e3, cart_folded.target_de)
    ax4.plot(sphr_folded.horz_range / 1e3, sphr_folded.target_de, "--")
    ax4.grid(True)
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["Cartesian", "Spherical"])

    output = testname + "_folded"
    print(f"saving {output}.png")
    plt.savefig(output)
    plt.close()


def grab_eigenrays(filename: str, srf: int, btm: int, upr: int, lwr: int, phase: float = None):
    """Load CASS/GRAB eigenrays for a single path type

    :param filename:    Name of the text eigenray file to load
    :param srf:         Number of surface bounces to select
    :param btm:         Number of bottom bounces to select
    :param upr:         Number of upper vertices to select
    :param lwr:         Number of lower vertices to select
    :param phase:       Option to match path phase to distinguish paths
    :return:            Eigenrays data structue.
    """
    model = np.loadtxt(filename)
    model_phase = model[:, 5]
    model_srf = model[:, 6]
    model_btm = model[:, 7]
    model_upr = model[:, 8]
    model_lwr = model[:, 9]

    index = np.logical_and(model_srf == srf, model_btm == btm)
    index = np.logical_and(index, model_upr == upr)
    index = np.logical_and(index, model_lwr == lwr)
    if phase is not None:
        index = np.logical_and(index, model_phase == phase)

    rays = usml.netcdf.Eigenrays()
    rays.horz_range = model[index, 0] * 1e3
    rays.travel_time = model[index, 1]
    rays.source_de = -model[index, 2]
    rays.target_de = -model[index, 3]
    rays.intensity = model[index, 4]
    rays.phase = model[index, 5]
    return rays


def wq3d_eigenrays(filename: str, srf: int, btm: int, upr: int, lwr: int, phase: float = None):
    """Load USML/WaveQ3D eigenrays for a single path type

    :param filename:    Name of the netCDF eigenray file to load
    :param srf:         Number of surface bounces to select
    :param btm:         Number of bottom bounces to select
    :param upr:         Number of upper vertices to select
    :param lwr:         Number of lower vertices to select
    :param phase:       Option to match path phase to distinguish paths
    :return:            Eigenrays data structue.
    """
    model = usml.netcdf.EigenrayList(filename)

    earth_radius = 6366.71e3
    rays = usml.netcdf.Eigenrays()
    rays.horz_range = list()
    rays.travel_time = list()
    rays.source_de = list()
    rays.target_de = list()
    rays.intensity = list()
    rays.phase = list()

    for target_list in model.eigenrays:
        for target in target_list:
            for n in range(len(target.bottom)):
                model_phase = np.round(np.degrees(target.phase.item(n)))
                model_srf = target.surface.item(n)
                model_btm = target.bottom.item(n)
                model_upr = target.upper.item(n)
                model_lwr = target.lower.item(n)

                ok = model_srf == srf and model_btm == btm
                ok = ok and model_upr == upr and model_lwr == lwr
                if phase is not None:
                    ok = ok and model_phase == phase

                if ok:
                    rays.travel_time.append(target.travel_time.item(n))
                    rays.source_de.append(target.source_de.item(n))
                    rays.target_de.append(target.target_de.item(n))
                    rays.intensity.append(-target.intensity.item(n))
                    rays.phase.append(model_phase)

    rays.horz_range = np.radians(model.latitude[:, 0] - model.source_latitude) * earth_radius
    rays.travel_time = np.asarray(rays.travel_time)
    rays.source_de = np.asarray(rays.source_de)
    rays.target_de = np.asarray(rays.target_de)
    rays.intensity = np.asarray(rays.intensity)
    rays.phase = np.asarray(rays.phase)

    return rays


def plot_comparison(output: str, grab: usml.netcdf.Eigenrays, wq3d: usml.netcdf.Eigenrays,
                    analytic: usml.netcdf.Eigenrays):
    """Compare eigenrays computed by different models for individual paths

    :param output:      Filename for plot outputs
    :param grab:        Eigenrays for CASS/GRAB model
    :param wq3d:        Eigenrays for USML/WaveQ3D model
    :param analytic:    Eigenrays for analytic solution
    """
    # remove bulk travel time
    slope = np.mean(analytic.travel_time) / np.mean(analytic.horz_range)
    grab.travel_time -= slope * grab.horz_range
    wq3d.travel_time -= slope * wq3d.horz_range
    analytic.travel_time -= slope * analytic.horz_range

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(8, 6), layout="constrained")

    ax1.plot(grab.horz_range / 1e3, grab.intensity)
    ax1.plot(wq3d.horz_range / 1e3, wq3d.intensity, "--")
    ax1.grid(True)
    ax1.set_ylabel("Propagation Loss (dB)")

    ax2.plot(grab.horz_range / 1e3, grab.travel_time * 1e3)
    ax2.plot(wq3d.horz_range / 1e3, wq3d.travel_time * 1e3, "--")
    ax2.plot(analytic.horz_range / 1e3, analytic.travel_time * 1e3, ":", linewidth=3.0)
    ax2.grid(True)
    ax2.set_ylabel("Travel Time - Bulk (msec)")

    ax3.plot(grab.horz_range / 1e3, grab.source_de)
    ax3.plot(wq3d.horz_range / 1e3, wq3d.source_de, "--")
    ax3.plot(analytic.horz_range / 1e3, analytic.source_de, ":", linewidth=3.0)
    ax3.grid(True)
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(grab.horz_range / 1e3, grab.target_de)
    ax4.plot(wq3d.horz_range / 1e3, wq3d.target_de, "--")
    ax4.plot(analytic.horz_range / 1e3, analytic.target_de, ":", linewidth=3.0)
    ax4.grid(True)
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["CASS/GRAB", "USML/WaveQ3D", "Analytic"])

    print(f"saving {output}.png")
    plt.savefig(output)
    plt.close()


class TestPedersen(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    @classmethod
    def setUpClass(cls):
        """Setup matplotlib defaults."""
        mpl.rcdefaults()
        plt.rcParams['figure.constrained_layout.use'] = True
        plt.rcParams['figure.figsize'] = [8, 6]

    @classmethod
    def tearDownClass(cls):
        """Reset matplotlib defaults."""
        mpl.rcdefaults()

    def test_pedersen_cycles_shallow(self):
        """Compare cycle range/time analytic solutions for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 2.0)  # 0.1)
        test_cycles(self, testname, source_depth, source_angles)

    def test_pedersen_cycles_deep(self):
        """Compare cycle range/time analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        source_angles = np.arange(20.0, 60.0, 1.0)
        test_cycles(self, testname, source_depth, source_angles)

    def test_pedersen_eigenrays_shallow(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        target_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 0.1)
        target_ranges = np.arange(500.0, 1000.0, 1.0)
        test_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)

    def test_pedersen_eigenrays_deep(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        target_depth = 800.0
        source_angles = np.arange(20.0, 51.21, 0.1)
        target_ranges = np.arange(3000.0, 3100.0, 1.0)
        test_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)

    def test_pedersen_raytrace_shallow(self):
        """Draw ray trace for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "studies/pedersen/pedersen_shallow_raytrace.nc")
        wavefront = usml.netcdf.read(filename)
        de_list = wavefront.source_de[::50]
        de_direct = de_list[np.asarray(de_list < 18.82).nonzero()]
        de_reflect = de_list[np.asarray(de_list >= 18.82).nonzero()]
        time = 0.42

        fig, ax = plt.subplots(figsize=(8, 6), layout="constrained")
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_direct)
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_reflect, fmt="--")
        usml.plot.plot_wavefront_2d(ax, wavefront, time=time, fmt="k-")
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 1.2)
        ax.set_ylim(-500, 0)

        print(f"saving {testname}.png")
        plt.savefig(testname)
        plt.close()

    def test_pedersen_raytrace_deep(self):
        """Draw ray trace for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "studies/pedersen/pedersen_deep_raytrace.nc")
        wavefront = usml.netcdf.read(filename)

        de_list = wavefront.source_de[::10]
        de_direct = de_list[np.asarray(de_list < 51.21).nonzero()]
        de_reflect = de_list[np.asarray(de_list >= 51.21).nonzero()]
        time = 1.75

        fig, ax = plt.subplots(figsize=(8, 6), layout="constrained")
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_direct)
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_reflect, fmt="--")
        usml.plot.plot_wavefront_2d(ax, wavefront, time=time, fmt="k-")
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 3.5)
        ax.set_ylim(-1200, 0)

        print(f"saving {testname}.png")
        plt.savefig(testname)
        plt.close()

    def test_pedersen_compare_shallow(self):
        """Compare eigenray solutions for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        target_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 0.025)
        target_ranges = np.arange(500.0, 1000.0, 1.0)

        analytic = usml.pedersen.PedersenCartesian()
        analytic_direct, analytic_folded = analytic.eigenrays(source_depth, source_angles, target_depth, target_ranges)

        filename = os.path.join(self.USML_DIR, "studies/pedersen/grab_eignerays_shallow.txt")
        grab_direct = grab_eigenrays(filename, 0, 0, 1, 0)
        grab_folded = grab_eigenrays(filename, 1, 0, 0, 0)

        filename = os.path.join(self.USML_DIR, "studies/pedersen/pedersen_shallow_proploss.nc")
        wq3d_direct = wq3d_eigenrays(filename, 0, 0, 1, 0, 0.0)
        wq3d_folded = wq3d_eigenrays(filename, 1, 0, 0, 0)

        output = testname + "_direct"
        plot_comparison(output, grab_direct, wq3d_direct, analytic_direct)
        plt.suptitle("Direct Path")

        output = testname + "_folded"
        plot_comparison(output, grab_folded, wq3d_folded, analytic_folded)
        plt.suptitle("Surface Reflected Path")

    def test_pedersen_compare_deep(self):
        """Compare eigenray solutions for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        target_depth = 800.0
        source_angles = np.arange(20.0, 60.0, 0.025)
        target_ranges = np.arange(3000.0, 3120.0, 1.0)

        analytic = usml.pedersen.PedersenCartesian()
        analytic_direct, analytic_folded = analytic.eigenrays(source_depth, source_angles, target_depth, target_ranges)

        filename = os.path.join(self.USML_DIR, "studies/pedersen/grab_eignerays_deep.txt")
        grab_direct = grab_eigenrays(filename, 0, 0, 1, 0, 0.0)
        grab_folded = grab_eigenrays(filename, 0, 0, 1, 0, -90.0)

        filename = os.path.join(self.USML_DIR, "studies/pedersen/pedersen_deep_proploss.nc")
        wq3d_direct = wq3d_eigenrays(filename, 0, 0, 1, 0, 0.0)
        wq3d_folded = wq3d_eigenrays(filename, 0, 0, 1, 0, -90.0)

        output = testname + "_direct"
        plot_comparison(output, grab_direct, wq3d_direct, analytic_direct)
        plt.suptitle("Direct Path")

        output = testname + "_folded"
        plot_comparison(output, grab_folded, wq3d_folded, analytic_folded)
        plt.suptitle("Caustic Path")
