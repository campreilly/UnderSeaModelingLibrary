"""Test ability to plot USML beam patterns
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np

import usml.netcdf
import usml.plot


class TestBeams(unittest.TestCase):
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "beampatterns", "test")

    def test_beampatterns(self):
        """Plot all outputs from bp_test.cc script.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        filenames = (
            "bp_omni.csv",
            "bp_cos.csv",
            "bp_sin.csv",
            "bp_vla.csv",
            "bp_hla.csv",
            "bp_planar.csv",
            "bp_piston.csv",
            "bp_multi.csv",
            "bp_cardioid.csv",
            "bp_arb.csv",
            "bp_arb_weight.csv",
            "bp_solid.csv")
        for file in filenames:
            # read beam pattern from disk
            fullfile = os.path.join(self.USML_DIR, file)
            print(f"reading {fullfile}")
            pattern = np.genfromtxt(fullfile, delimiter=',')

            # plot beam pattern
            fig, ax = plt.subplots(subplot_kw={'projection': '3d'})
            usml.plot.plot_beampattern_3d(ax, pattern)
            ax.set_title(file.replace(".csv", ""))
            ax.set_xlabel('x')
            ax.set_ylabel('y')
            ax.set_zlabel('z')
            output = fullfile.replace(".csv", ".png")
            print(f"saving {output}")
            plt.savefig(output)
            plt.close()


if __name__ == '__main__':
    unittest.main()
