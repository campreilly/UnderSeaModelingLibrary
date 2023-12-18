"""Use analytic solutions to test WaveQ3D results for Pedersen n^2 linear ocean sound speed profile
"""
import inspect
import os
import unittest
from typing import Tuple

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import scipy.interpolate as interp

import usml.netcdf
import usml.pedersen
import usml.plot


def test_analytic_cycles(self, testname: str, source_depth: float, source_angles: np.ndarray) -> None:
    """Compare cycle range/time analytic solutions in cartesian and spherical coordinate.

    Our tests use calculation of the cycle range to esimtate the travel time, source angle, and target angle for each
    eigenray. This test checks to see if the Carteisan version of the cycle range model matches the results in
    Spherical Coordinates with an earth flattening correction. Acts as a unit test for the cycle range analytic
    solutions. Test fails if the maximum difference between cycle ranges is greater than 0.5 m or if the maximum
    difference between cycle times is greater than 0.5 msec.

    :param self:                unit test that called this functopm
    :param testname:            name of the test to use for saving files.
    :param source_depth:        source depth expressed as distance down from ocean surface
    :param source_angles:       array of launch angles from source, up is positive
    """
    cartesian = usml.pedersen.PedersenCartesian()
    spherical = usml.pedersen.PedersenSpherical()
    cart_cycle_ranges, cart_cycle_times, _, _ = cartesian.analytic_cycle(source_depth, source_angles)
    sphr_cycle_ranges, sphr_cycle_times, _, _ = spherical.analytic_cycle(source_depth, source_angles)

    fig, (ax1, ax2) = plt.subplots(2)

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

    output = os.path.join(self.USML_DIR, testname + ".png")
    print(f"saving {output}")
    plt.savefig(output)
    plt.close()

    self.assertLess(np.abs(cart_cycle_ranges - sphr_cycle_ranges).max(), 0.5)
    self.assertLess(np.abs(cart_cycle_times - cart_cycle_times).max(), 0.5e-3)


def test_analytic_eigenrays(self, testname: str, source_depth: float, source_angles: np.ndarray, target_depth: float,
                            target_ranges: np.ndarray) -> None:
    """Compare eigenray analytic solutions in cartesian and spherical coordinate.

    This test checks to see if the Carteisan version of the eigneray analytic solution matches the results in Spherical
    Coordinates with an earth flattening correction. Acts as a unit test for the eigenray analytic solutions. Test
    fails if the maximum difference between travel times ranges is greater than 0.001 sec or if the maximum difference
    between angles is greater than 0.1 degrees.

    :param self:                unit test that called this functopm
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

    # compare calculations of direct path eigenrays in Cartesian and Spherical coordinates.
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2)
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

    output = os.path.join(self.USML_DIR, testname + "_direct.png")
    print(f"saving {output}")
    plt.savefig(output)
    plt.close()

    travel_time = interp.pchip_interpolate(shpr_direct.horz_range, shpr_direct.travel_time, cart_direct.horz_range)
    source_de = interp.pchip_interpolate(shpr_direct.horz_range, shpr_direct.source_de, cart_direct.horz_range)
    target_de = interp.pchip_interpolate(shpr_direct.horz_range, shpr_direct.target_de, cart_direct.horz_range)

    self.assertLess(np.abs(cart_direct.travel_time - travel_time).max(), 0.001)
    self.assertLess(np.abs(cart_direct.source_de - source_de).max(), 0.1)
    self.assertLess(np.abs(cart_direct.target_de - target_de).max(), 0.1)

    # compare calculations of non-direct path eigenrays in Cartesian and Spherical coordinates.
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2)
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

    output = os.path.join(self.USML_DIR, testname + "_folded.png")
    print(f"saving {output}")
    plt.savefig(output)
    plt.close()

    travel_time = interp.pchip_interpolate(sphr_folded.horz_range, sphr_folded.travel_time, cart_folded.horz_range)
    source_de = interp.pchip_interpolate(sphr_folded.horz_range, sphr_folded.source_de, cart_folded.horz_range)
    target_de = interp.pchip_interpolate(sphr_folded.horz_range, sphr_folded.target_de, cart_folded.horz_range)

    self.assertLess(np.abs(cart_folded.travel_time - travel_time).max(), 0.001)
    self.assertLess(np.abs(cart_folded.source_de - source_de).max(), 0.1)
    self.assertLess(np.abs(cart_folded.target_de - target_de).max(), 0.1)


def grab_eigenrays(filename: str, srf: int, btm: int, upr: int, lwr: int, phase: float = None) -> usml.netcdf.Eigenrays:
    """Load CASS/GRAB eigenrays for a single path type

    The GRAB eigenrays for these tests are cut-and-paste from the OUTPUT.DAT text file produced by CASS. The trailing 
    "i" on the imaginary eigenrays is removed so that the file can be decoded purely as matrix of numbers. This 
    routine uses a-priori knowledge of the CASS output files to decode these text files. The calling routine 
    specifies a specific combination of bounces and phase to select a single acoustic path from this data.
    
    :param filename:    Name of the text eigenray file to load
    :param srf:         Number of surface bounces to select
    :param btm:         Number of bottom bounces to select
    :param upr:         Number of upper vertices to select
    :param lwr:         Number of lower vertices to select
    :param phase:       Option to match path phase to distinguish paths
    :return:            Eigenrays data structure.
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


def wq3d_eigenrays(filename: str, srf: int, btm: int, upr: int, lwr: int, phase: float = None) -> usml.netcdf.Eigenrays:
    """Load USML/WaveQ3D eigenrays for a single path type

    The WaveQ3D eigenrays for these tests are written to a netCDF in USML format. The calling routine specifies a
    specific combination of bounces and phase to select a single acoustic path from this data.

    :param filename:    Name of the netCDF eigenray file to load
    :param srf:         Number of surface bounces to select
    :param btm:         Number of bottom bounces to select
    :param upr:         Number of upper vertices to select
    :param lwr:         Number of lower vertices to select
    :param phase:       Option to match path phase to distinguish paths
    :return:            Eigenrays data structue.
    """
    model = usml.netcdf.EigenrayList(filename)

    earth_radius = 6378101.030201019  # earth radius at 45 deg north
    rays = usml.netcdf.Eigenrays()
    rays.horz_range = list()
    rays.travel_time = list()
    rays.source_de = list()
    rays.source_az = list()
    rays.target_de = list()
    rays.target_az = list()
    rays.intensity = list()
    rays.phase = list()
    rays.bottom = list()
    rays.surface = list()
    rays.caustic = list()
    rays.upper = list()
    rays.lower = list()

    for target_list in model.eigenrays:
        for target in target_list:
            for n in range(len(target.bottom)):
                model_phase = np.round(np.degrees(target.phase.item(n)))
                model_srf = target.surface.item(n)
                model_btm = target.bottom.item(n)
                model_cst = target.caustic.item(n)
                model_upr = target.upper.item(n)
                model_lwr = target.lower.item(n)

                ok = model_srf == srf and model_btm == btm and model_upr == upr and model_lwr == lwr
                if phase is not None:
                    ok = ok and model_phase == phase

                if ok:
                    rays.travel_time.append(target.travel_time.item(n))
                    rays.source_de.append(target.source_de.item(n))
                    rays.source_az.append(target.source_az.item(n))
                    rays.target_de.append(target.target_de.item(n))
                    rays.target_az.append(target.target_az.item(n))
                    rays.intensity.append(-target.intensity.item(n))
                    rays.phase.append(model_phase)
                    rays.bottom.append(model_btm)
                    rays.surface.append(model_btm)
                    rays.caustic.append(model_btm)
                    rays.upper.append(model_btm)
                    rays.lower.append(model_btm)

    rays.horz_range = np.radians(model.latitude[:, 0] - model.source_latitude) * earth_radius
    rays.travel_time = np.asarray(rays.travel_time)
    rays.source_de = np.asarray(rays.source_de)
    rays.source_az = np.asarray(rays.source_az)
    rays.target_de = np.asarray(rays.target_de)
    rays.target_az = np.asarray(rays.target_az)
    rays.intensity = np.asarray(rays.intensity)
    rays.phase = np.asarray(rays.phase)
    rays.bottom = np.asarray(rays.bottom)
    rays.surface = np.asarray(rays.surface)
    rays.caustic = np.asarray(rays.caustic)
    rays.upper = np.asarray(rays.upper)
    rays.lower = np.asarray(rays.lower)

    return rays


def wq3d_proploss(filename: str) -> usml.netcdf.Eigenrays:
    """Load USML/WaveQ3D total propation loss for each target

    The WaveQ3D eigenrays for these tests are written to a netCDF in USML format. One of the eigenrays for each target
    represents the total propation loss for each target. This summation can be coherent or incoherent.

    :param filename:    Name of the netCDF eigenray file to load
    :return:            Eigenrays data structue.
    """
    model = usml.netcdf.EigenrayList(filename, proploss=True)

    earth_radius = 6378101.030201019  # earth radius at 45 deg north
    rays = usml.netcdf.Eigenrays()
    rays.horz_range = list()
    rays.travel_time = list()
    rays.source_de = list()
    rays.target_de = list()
    rays.intensity = list()
    rays.phase = list()

    for target_list in model.eigenrays:
        for target in target_list:
            n = 0
            rays.travel_time.append(target.travel_time.item(n))
            rays.source_de.append(target.source_de.item(n))
            rays.target_de.append(target.target_de.item(n))
            rays.intensity.append(-target.intensity.item(n))
            rays.phase.append(np.round(np.degrees(target.phase.item(n))))

    rays.horz_range = np.radians(model.latitude[:, 0] - model.source_latitude) * earth_radius
    rays.travel_time = np.asarray(rays.travel_time)
    rays.source_de = np.asarray(rays.source_de)
    rays.target_de = np.asarray(rays.target_de)
    rays.intensity = np.asarray(rays.intensity)
    rays.phase = np.asarray(rays.phase)

    return rays


def plot_ray_differences(filename: str) -> None:
    """Compare modeled ray trace to analytic results.

    The analyze_raytrace unit test in pedersen_test.cc compares the modeled results for travel time, range,
    and depression/elevation angle to their analytic solution, and it writes the results to a *.csv file. The unit
    test analyzes this data for the maximum difference. But this utility plots the differences to provide the tester
    with an overview of the difference trendes.

    :param filename:    CSV file to load
    """
    results = np.loadtxt(filename, skiprows=1, delimiter=",")
    de_launch = results[::10, 0]
    time_model = results[::10, 1]
    time_theory = results[::10, 2]
    range_model = results[::10, 3]
    range_theory = results[::10, 4]
    de_model = results[::10, 5]
    de_theory = results[::10, 6]

    fig, (ax1, ax2, ax3) = plt.subplots(3)
    ax1.plot(range_theory / 1e3, (time_model - time_theory) * 1e3, ".")
    ax1.grid(True)
    ax1.set_ylabel('Time Diff (msec)')

    ax2.plot(range_theory / 1e3, (range_model - range_theory), ".")
    ax2.grid(True)
    ax2.set_ylabel('Range Diff (m)')

    ax3.plot(range_theory / 1e3, (de_model - de_theory), ".")
    ax3.grid(True)
    ax3.set_ylabel('D/E Diff (deg)')
    ax3.set_xlabel('Range (km)')


def compare_eigenray_models(self, output: str, grab: usml.netcdf.Eigenrays, wq3d: usml.netcdf.Eigenrays,
                            analytic: usml.netcdf.Eigenrays, tl_lim: Tuple, ang_lim: Tuple, time_lim: Tuple) -> None:
    """Compare eigenrays computed by different models for individual paths

    :param self:        unit test that called this function
    :param output:      filename for plot outputs
    :param grab:        eigenrays from CASS/GRAB model
    :param wq3d:        eigenrays from USML/WaveQ3D model
    :param analytic:    eigenrays from analytic solution
    """
    # remove bulk travel time
    slope = np.mean(analytic.travel_time) / np.mean(analytic.horz_range)
    grab.travel_time -= slope * grab.horz_range
    wq3d.travel_time -= slope * wq3d.horz_range
    analytic.travel_time -= slope * analytic.horz_range

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2)

    ax1.plot(grab.horz_range / 1e3, grab.intensity, ":", linewidth=3.0)
    ax1.plot(wq3d.horz_range / 1e3, wq3d.intensity, "--")
    ax1.grid(True)
    ax1.set_ylim(tl_lim[0], tl_lim[1])
    ax1.set_ylabel("Propagation Loss (dB)")

    ax2.plot(grab.horz_range / 1e3, grab.travel_time * 1e3, ":", linewidth=3.0)
    ax2.plot(wq3d.horz_range / 1e3, wq3d.travel_time * 1e3, "--")
    ax2.plot(analytic.horz_range / 1e3, analytic.travel_time * 1e3)
    ax2.grid(True)
    ax2.set_ylim(time_lim[0], time_lim[1])
    ax2.set_ylabel("Travel Time - Bulk (msec)")

    ax3.plot(grab.horz_range / 1e3, grab.source_de, ":", linewidth=3.0)
    ax3.plot(wq3d.horz_range / 1e3, wq3d.source_de, "--")
    ax3.plot(analytic.horz_range / 1e3, analytic.source_de)
    ax3.grid(True)
    ax3.set_ylim(ang_lim[0], ang_lim[1])
    ax3.set_xlabel("Target Range (km)")
    ax3.set_ylabel("Source D/E (deg)")

    ax4.plot(grab.horz_range / 1e3, grab.target_de, ":", linewidth=3.0)
    ax4.plot(wq3d.horz_range / 1e3, wq3d.target_de, "--")
    ax4.plot(analytic.horz_range / 1e3, analytic.target_de)
    ax4.grid(True)
    ax4.set_ylim(-ang_lim[1], -ang_lim[0])
    ax4.set_xlabel("Target Range (km)")
    ax4.set_ylabel("Target D/E (deg)")
    ax4.legend(["CASS/GRAB", "USML/WaveQ3D", "Analytic"])

    print(f"saving {output}")
    plt.savefig(output)
    plt.close()


def compare_proploss_totals(self, output: str, grab: np.ndarray, wq3d: usml.netcdf.Eigenrays,
                            analytic: np.ndarray) -> None:
    """Compare total propagation loss computed by different models

    :param self:        unit test that called this function
    :param output:      filename for plot outputs
    :param grab:        eigenrays from CASS/GRAB model
    :param wq3d:        eigenrays from USML/WaveQ3D model
    :param analytic:    eigenrays from analytic solution
    """
    # remove bulk travel time

    fig, ax = plt.subplots()

    ax.plot(grab[:, 0], grab[:, 1])
    ax.plot(wq3d.horz_range / 1e3, wq3d.intensity)
    ax.plot(analytic[:, 0] / 1e3, analytic[:, 1])
    ax.grid(True)
    ax.set_xlabel("Target Range (km)")
    ax.set_ylabel("Propagation Loss (dB)")
    ax.legend(["CASS/GRAB", "USML/WaveQ3D", "Analytic"])
    ax.set_ylim(-80, -40)

    print(f"saving {output}")
    plt.savefig(output)
    plt.close()


class TestPedersen(unittest.TestCase):
    """Unit tests for USML Pedersen study"""
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "studies", "pedersen")

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

    def test_pedersen_cycles_shallow(self):
        """Compare cycle range/time analytic solutions for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 2.0)  # 0.1)
        test_analytic_cycles(self, testname, source_depth, source_angles)

    def test_pedersen_cycles_deep(self):
        """Compare cycle range/time analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        source_angles = np.arange(20.0, 60.0, 1.0)
        test_analytic_cycles(self, testname, source_depth, source_angles)

    def test_pedersen_eigenrays_shallow(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 75.0
        target_depth = 75.0
        source_angles = np.arange(1.0, 25.0, 0.1)
        target_ranges = np.arange(500.0, 1000.0, 1.0)
        test_analytic_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)

    def test_pedersen_eigenrays_deep(self):
        """Compare eigenray analytic solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        target_depth = 800.0
        source_angles = np.arange(20.0, 51.21, 0.1)
        target_ranges = np.arange(3000.0, 3100.0, 1.0)
        test_analytic_eigenrays(self, testname, source_depth, source_angles, target_depth, target_ranges)

    def test_pedersen_raytrace_shallow(self):
        """Draw ray trace for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "pedersen_shallow_raytrace.nc")
        wavefront = usml.netcdf.read(filename)
        de_list = wavefront.source_de
        de_direct = de_list[np.asarray(de_list < 18.82).nonzero()]
        de_reflect = de_list[np.asarray(de_list >= 18.82).nonzero()]
        time = 0.42
        target_ranges = np.arange(500.0, 1000.0, 1.0)
        target_depths = -75.0 * np.ones_like(target_ranges)

        fig, ax = plt.subplots()
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_direct)
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_reflect, fmt="--")
        usml.plot.plot_wavefront_2d(ax, wavefront, time=time, fmt="k-")
        ax.plot(target_ranges / 1e3, target_depths, 'k.')
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 1.2)
        ax.set_ylim(-200, 0)
        ax.set_title(f"Wavefront at {time:.3f} secs")

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        filename = os.path.join(self.USML_DIR, "pedersen_shallow_raytrace.csv")
        plot_ray_differences(filename)

        output = os.path.join(self.USML_DIR, testname + "_compare.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_pedersen_raytrace_deep(self):
        """Draw ray trace for shallow source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "pedersen_deep_raytrace.nc")
        wavefront = usml.netcdf.read(filename)

        de_list = wavefront.source_de
        de_direct = de_list[np.asarray(de_list < 51.21).nonzero()]
        de_reflect = de_list[np.asarray(de_list >= 51.21).nonzero()]
        time = 1.75
        target_ranges = np.arange(3000.0, 3100.0, 1.0)
        target_depths = -800.0 * np.ones_like(target_ranges)

        fig, ax = plt.subplots()
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_direct)
        usml.plot.plot_raytrace_2d(ax, wavefront, de=de_reflect, fmt="--")
        usml.plot.plot_wavefront_2d(ax, wavefront, time=time, fmt="k-")
        ax.plot(target_ranges / 1e3, target_depths, 'k.')
        ax.grid(True)
        ax.set_xlabel('Range (km)')
        ax.set_ylabel('Depth (m)')
        ax.set_xlim(0, 3.5)
        ax.set_ylim(-1200, 0)
        ax.set_title(f"Wavefront at {time:.3f} secs")

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        filename = os.path.join(self.USML_DIR, "pedersen_deep_raytrace.csv")
        plot_ray_differences(filename)

        output = os.path.join(self.USML_DIR, testname + "_compare.png")
        print(f"saving {output}")
        plt.savefig(output)
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
        filename = os.path.join(self.USML_DIR, "ffp_n2shallow.csv")
        analytic_total = np.loadtxt(filename, delimiter=",")

        filename = os.path.join(self.USML_DIR, "grab_eigenrays_shallow.txt")
        grab_direct = grab_eigenrays(filename, 0, 0, 1, 0)
        grab_folded = grab_eigenrays(filename, 1, 0, 0, 0)
        filename = os.path.join(self.USML_DIR, "grab_pressure_shallow.csv")
        grab_total = np.loadtxt(filename, delimiter=",")

        filename = os.path.join(self.USML_DIR, "pedersen_shallow_proploss.nc")
        wq3d_direct = wq3d_eigenrays(filename, 0, 0, 1, 0)
        wq3d_folded = wq3d_eigenrays(filename, 1, 0, 0, 0)
        wq3d_total = wq3d_proploss(filename)

        output = os.path.join(self.USML_DIR, testname + "_direct.png")
        compare_eigenray_models(self, output, grab_direct, wq3d_direct, analytic_direct, tl_lim=(-90, -50),
                                ang_lim=(8, 24),
                                time_lim=(-20, 15))

        output = os.path.join(self.USML_DIR, testname + "_folded.png")
        compare_eigenray_models(self, output, grab_folded, wq3d_folded, analytic_folded, tl_lim=(-90, -50),
                                ang_lim=(8, 24),
                                time_lim=(-20, 15))

        output = testname + "_total"
        output = os.path.join(self.USML_DIR, testname + "_total.png")
        compare_proploss_totals(self, output, grab_total, wq3d_total, analytic_total)

    def test_pedersen_compare_deep(self):
        """Compare eigenray solutions for deep source. """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        source_depth = 1000.0
        target_depth = 800.0
        source_angles = np.arange(20.0, 60.0, 0.025)
        target_ranges = np.arange(3000.0, 3120.0, 1.0)

        analytic = usml.pedersen.PedersenCartesian()
        analytic_direct, analytic_folded = analytic.eigenrays(source_depth, source_angles, target_depth, target_ranges)
        filename = os.path.join(self.USML_DIR, "ffp_n2deep.csv")
        analytic_total = np.loadtxt(filename, delimiter=",")

        filename = os.path.join(self.USML_DIR, "grab_eigenrays_deep.txt")
        grab_direct = grab_eigenrays(filename, 0, 0, 1, 0, 0.0)
        grab_folded = grab_eigenrays(filename, 0, 0, 1, 0, -90.0)
        filename = os.path.join(self.USML_DIR, "grab_pressure_deep.csv")
        grab_total = np.loadtxt(filename, delimiter=",")

        filename = os.path.join(self.USML_DIR, "pedersen_deep_proploss.nc")
        wq3d_direct = wq3d_eigenrays(filename, 0, 0, 1, 0, 0.0)
        wq3d_folded = wq3d_eigenrays(filename, 0, 0, 1, 0, -90.0)
        wq3d_total = wq3d_proploss(filename)

        output = os.path.join(self.USML_DIR, testname + "_direct.png")
        compare_eigenray_models(self, output, grab_direct, wq3d_direct, analytic_direct, tl_lim=(-90, -50),
                                ang_lim=(36, 52),
                                time_lim=(-20, 15))

        output = os.path.join(self.USML_DIR, testname + "_folded.png")
        compare_eigenray_models(self, output, grab_folded, wq3d_folded, analytic_folded, tl_lim=(-90, -50),
                                ang_lim=(36, 52),
                                time_lim=(-20, 15))

        output = os.path.join(self.USML_DIR, testname + "_total.png")
        compare_proploss_totals(self, output, grab_total, wq3d_total, analytic_total)

    def test_pedersen_sensitivity(self):
        """Compare eigenray solutions for different ray spacing options.

        TODO Compute statistics of differences from analystic model.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filename = os.path.join(self.USML_DIR, "ffp_n2deep.csv")
        analytic = np.loadtxt(filename, delimiter=",")

        filename = os.path.join(self.USML_DIR, "pedersen_deep_sensitivity_0125.nc")
        total_0125 = wq3d_proploss(filename)

        filename = os.path.join(self.USML_DIR, "pedersen_deep_sensitivity_0250.nc")
        total_0250 = wq3d_proploss(filename)

        filename = os.path.join(self.USML_DIR, "pedersen_deep_sensitivity_0500.nc")
        total_0500 = wq3d_proploss(filename)

        filename = os.path.join(self.USML_DIR, "pedersen_deep_sensitivity_1000.nc")
        total_1000 = wq3d_proploss(filename)

        filename = os.path.join(self.USML_DIR, "pedersen_deep_sensitivity_tan.nc")
        total_tan = wq3d_proploss(filename)

        fig, ax = plt.subplots()

        ax.plot(analytic[:, 0] / 1e3, analytic[:, 1])
        ax.plot(total_0125.horz_range / 1e3, total_0125.intensity)
        ax.plot(total_0250.horz_range / 1e3, total_0250.intensity)
        ax.plot(total_0500.horz_range / 1e3, total_0500.intensity)
        ax.plot(total_1000.horz_range / 1e3, total_1000.intensity)
        ax.plot(total_tan.horz_range / 1e3, total_tan.intensity)
        ax.grid(True)
        ax.set_xlabel("Target Range (km)")
        ax.set_ylabel("Propagation Loss (dB)")
        ax.legend(["Analytic", "$1/80^o$ spacing", "$1/40^o$ spacing", "$1/20^o$ spacing", "$1/10^o$ spacing", "tan spacing"])
        ax.set_ylim(-80, -40)

        output = os.path.join(self.USML_DIR, testname + ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()
