"""Display plots for outputs of usml.ocean.test unit tests.
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np


class TestOcean(unittest.TestCase):
    USML_DIR = os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir)))

    def test_ambient(self):
        """Plots results of ambient_wenz_test case in ambient_test.cc unit test.

        Computes ambient noise levels for ambient_wenz model for a variety of sea states, shipping levels,
        and rain rates.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read ambient noise levels from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/ambient_wenz_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        frequency = model[0, 3:]
        sea_state = np.unique(model[1:, 0])
        ship_level = np.unique(model[1:, 1])
        rain_rate = np.unique(model[1:, 2])
        ambient = model[1:, 3:]
        ambient = ambient.reshape(ambient.size)
        # ambient = ambient.reshape(len(frequency),len(sea_state),len(ship_level),len(rain_rate))
        ambient = ambient.reshape(len(rain_rate), len(ship_level), len(sea_state), len(frequency))

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_sea_state"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, 0, :, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Sea State 0", "Sea State 1", "Sea State 2", "Sea State 3", "Sea State 4", "Sea State 5",
                      "Sea State 6"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_ship_level"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, :, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Shipping 0", "Shipping 1", "Shipping 2", "Shipping 3", "Shipping 4", "Shipping 5", "Shipping 6",
                      "Shipping 7"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = testname + "_rain_rate"
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[:, 0, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["No Rain", "Light Rain", "Medium Rain", "Heavy Rain"])
        print("saving {0}.png".format(output))
        plt.savefig(output)
        plt.close()

    def test_profile_analytic(self):
        """Plot  results of analytic test models written out to CSV by plot_profile_test in test_profile.cc file.
        The plotted models include:
            - Constant Profile
            - Linear Profile
            - Bi-Linear Profile
            - Munk Profile
            - N^2 Linear Profile
            - Catenary Profile
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/profile_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        labels = ["FlatEarth", "Linear", "BiLinear", "Munk", "N2 Linear", "Catenary"]
        depth = model[1:, 0]
        speed = model[1:, 1::2]
        grad = model[1:, 2::2]

        fig, (ax1, ax2) = plt.subplots(ncols=2)
        h = ax1.plot(speed, depth)
        ax1.grid(True)
        ax1.set_xlabel('Speed (m/s)')
        ax1.set_ylabel('Depth (m)')
        ax1.legend(h, labels)

        ax2.plot(grad, depth)
        ax2.grid(True)
        ax2.set_xlabel('Grad (m/s/m)')
        ax2.set_yticklabels([])

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_profile_mackenzie(self):
        """Plot results of Mackenzie models written out to CSV by compute_mackenzie_test in test_profile.cc file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/mackenzie_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        depth = -model[1:, 0]
        temp = model[1:, 1]
        salinity = model[1:, 2]
        speed = model[1:, 3]
        grad = model[1:, 4]

        fig, (ax1, ax2, ax3, ax4) = plt.subplots(ncols=4)
        ax1.plot(temp, depth)
        ax1.grid(True)
        ax1.set_xlabel('Temp (degC)')
        ax1.set_ylabel('Depth (m)')

        ax2.plot(salinity, depth)
        ax2.grid(True)
        ax2.set_xlabel('Salinity (ppt)')
        ax2.set_yticklabels([])

        ax3.plot(speed, depth)
        ax3.grid(True)
        ax3.set_xlabel('Speed (m/s)')
        ax3.set_yticklabels([])

        ax4.plot(grad, depth)
        ax4.grid(True)
        ax4.set_xlabel('Grad (m/s/m)')
        ax4.set_yticklabels([])

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_rayleigh_sediments(self):
        """Tests the version of the reflect_loss_rayleigh() constructor that takes a sediment type as its argument.

        The parameters for each sediment type are defined using table 1.3 from F.B. Jensen, W.A. Kuperman,
        M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 41. Note that reflect_loss_rayleigh() does not
        implement the weak, depth dependent, shear in slit, sand, and gravel.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_sediments.csv")
        print("reading {0}".format(fullfile))
        labels = ["clay", "silt", "sand", "gravel", "moraine", "chalk", "limestone", "basalt"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        fig, ax = plt.subplots()
        h = ax.plot(angle, reflect_loss)
        ax.grid(True)
        ax.set_xlabel('Grazing Angle (deg)')
        ax.set_ylabel('Reflection Loss (dB)')
        ax.set_ylim(0, 20)
        ax.legend(h, labels, ncol=4)

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_rayleigh_changes(self):
        """Tests reflect_loss_rayleigh() constructor that takes compression and shear wave parameters as inputs.

        Illustrates how the relection loss changes as a function of compressional sound speed, density, attenuation,
        and shear speed. Implements the four test cases (a, b, c,and d) defined in  F.B. Jensen, W.A. Kuperman,
        M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 35-49.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2)
        max_db = 30

        # reflection loss for changes in compressional sound speed
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_a.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$c_p$=1550", r"$c_p$=1600", r"$c_p$=1800"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax1.plot(angle, reflect_loss)
        ax1.grid(True)
        ax1.set_xticklabels([])
        ax1.set_ylabel('Reflection Loss (dB)')
        ax1.set_ylim(0, max_db)
        ax1.set_title(r"$\rho$=2 $\alpha$=0.5")
        ax1.legend(h, labels)

        # reflection loss for changes in compressional attenuation
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_b.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$\alpha$=1.0", r"$\alpha$=0.5", r"$\alpha$=0.0"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax2.plot(angle, reflect_loss)
        ax2.grid(True)
        ax2.set_xticklabels([])
        ax2.set_yticklabels([])
        ax2.set_ylim(0, max_db)
        ax2.set_title(r"$c_p$=1600 $\rho$=2")
        ax2.legend(h, labels)

        # reflection loss for changes in density
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_c.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$\rho$=1.5", r"$\rho$=2.0", r"$\rho$=2.5"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax3.plot(angle, reflect_loss)
        ax3.grid(True)
        ax3.set_xlabel('Grazing Angle (deg)')
        ax3.set_ylabel('Reflection Loss (dB)')
        ax3.set_ylim(0, max_db)
        ax3.set_title(r"$c_p$=1600 $\alpha$=0.5")
        ax3.legend(h, labels)

        # reflection loss for changes in shear speed
        fullfile = os.path.join(self.USML_DIR, "ocean/test/rayleigh_test_d.csv")
        print("reading {0}".format(fullfile))
        labels = [r"$c_s$=0.0", r"$c_s$=200", r"$c_s$=400", r"$c_s$=600"]
        model = np.genfromtxt(fullfile, delimiter=',')
        angle = model[1:, 0]
        reflect_loss = model[1:, 1:]

        h = ax4.plot(angle, reflect_loss)
        ax4.grid(True)
        ax4.set_xlabel('Grazing Angle (deg)')
        ax4.set_ylabel('Reflection Loss (dB)')
        ax4.set_ylim(0, max_db)
        ax4.set_title(r"$c_p$=1600 $\rho$=2 $\alpha$=0.5")
        ax4.legend(h, labels)

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()

    def test_wave_height_pierson(self):
        """Plot data output from Pierson and Moskowitz model for computing wave height from wind speed.

        Compare to significant wave height plot from http://www.wikiwaves.org/Ocean-Wave_Spectra.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "ocean/test/wave_height_pierson_test.csv")
        print("reading {0}".format(fullfile))
        model = np.genfromtxt(fullfile, delimiter=',')
        wind_speed = model[1:, 0]
        wave_height = model[1:, 1]

        fig, ax = plt.subplots()
        ax.plot(wind_speed, wave_height)
        ax.grid(True)
        ax.set_xlim(0, 25.0)
        ax.set_ylim(0, 20.0)
        ax.set_xlabel('Wind Speed (m/s)')
        ax.set_ylabel('Significant Wave Height (m)')

        print("saving {0}.png".format(testname))
        plt.savefig(testname)
        plt.close()


if __name__ == '__main__':
    unittest.main()