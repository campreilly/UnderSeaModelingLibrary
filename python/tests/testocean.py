"""Display plots for outputs of usml.ocean.test unit tests.
"""
import inspect
import os
import unittest

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FormatStrFormatter, MultipleLocator


class TestOcean(unittest.TestCase):
    """Display plots for outputs of usml.ocean.test unit tests. Reads data from and write plots to the USML ocean/test
    directory.
    """
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "ocean", "test")

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

    def test_ambient(self):
        """Plots results of ambient_wenz_test case in ambient_test.cc unit test.

        Computes ambient noise levels for ambient_wenz model for a variety of sea states, shipping levels,
        and rain rates.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read ambient noise levels from disk
        fullfile = os.path.join(self.USML_DIR, "ambient_wenz_test.csv")
        print("reading {fullfile}")
        model = np.genfromtxt(fullfile, delimiter=',')
        frequency = model[0, 3:]
        sea_state = np.unique(model[1:, 0])
        ship_level = np.unique(model[1:, 1])
        rain_rate = np.unique(model[1:, 2])
        ambient = model[1:, 3:]
        ambient = ambient.reshape(ambient.size)
        ambient = ambient.reshape(len(rain_rate), len(ship_level), len(sea_state), len(frequency))

        # plot ambient noise levels as a function of frequency and sea state
        output = os.path.join(self.USML_DIR, testname + "_sea_state.png")
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, 0, :, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Sea State 0", "Sea State 1", "Sea State 2", "Sea State 3", "Sea State 4", "Sea State 5",
                      "Sea State 6"])
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = output = os.path.join(self.USML_DIR, testname + "_ship_level.png")
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[0, :, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["Shipping 0", "Shipping 1", "Shipping 2", "Shipping 3", "Shipping 4", "Shipping 5", "Shipping 6",
                      "Shipping 7"])
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # plot ambient noise levels as a function of frequency and sea state
        output = output = os.path.join(self.USML_DIR, testname + "_rain_rate.png")
        fig, ax = plt.subplots()
        h = ax.semilogx(frequency, ambient[:, 0, 0, :].transpose())
        ax.grid(True)
        ax.set_title("WENZ Ambient Noise Model")
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Noise Level (dB//Hz)")
        ax.legend(h, ["No Rain", "Light Rain", "Medium Rain", "Heavy Rain"])
        print(f"saving {output}")
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
        fullfile = os.path.join(self.USML_DIR, "profile_test.csv")
        print(f"reading {fullfile}")
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

        output = fullfile.replace(".csv", ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_profile_mackenzie(self):
        """Plot results of Mackenzie models written out to CSV by compute_mackenzie_test in test_profile.cc file.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "mackenzie_test.csv")
        print(f"reading {fullfile}")
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

        output = fullfile.replace(".csv", ".png")
        print(f"saving {output}")
        plt.savefig(output)
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
        fullfile = os.path.join(self.USML_DIR, "rayleigh_sediments.csv")
        print(f"reading {fullfile}")
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

        output = fullfile.replace(".csv", ".png")
        print(f"saving {output}")
        plt.savefig(output)
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
        fullfile = os.path.join(self.USML_DIR, "rayleigh_test_a.csv")
        output = fullfile.replace("_a.csv", ".png")
        print(f"reading {fullfile}")
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
        fullfile = os.path.join(self.USML_DIR, "rayleigh_test_b.csv")
        print(f"reading {fullfile}")
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
        fullfile = os.path.join(self.USML_DIR, "rayleigh_test_c.csv")
        print(f"reading {fullfile}")
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
        fullfile = os.path.join(self.USML_DIR, "rayleigh_test_d.csv")
        print(f"reading {fullfile}")
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

        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_wave_height_pierson(self):
        """Plot data output from Pierson and Moskowitz model for computing wave height from wind speed.

        Compare to significant wave height plot from http://www.wikiwaves.org/Ocean-Wave_Spectra.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read analytic ocean profiles from disk
        fullfile = os.path.join(self.USML_DIR, "wave_height_pierson_test.csv")
        print(f"reading {fullfile}")
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

        output = fullfile.replace(".csv", ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

    def test_scattering_chapman(self):
        """Plot data output from Chapman and Harris model for computing surface scattering strength from wind speed.

        The Chapman/Harris surface scattering strength model is an empirical fit to surface scattering strength
        measurements. The model developemnet process fit scattering strength vs. grazing angle on a log/log scale to
        straight lines. This test replicates Figures 1 and 3 from the original paper and shows how the values
        computed by USML fit the theory.

        Chapman R. P., Harris J. H., "Surface Backscattering Strengths Measured with Explosive Sound Sources,"
        J. Acoust. Soc. Am. 34, 1592–1597 (1962).
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        # read USML scattering strength from disk
        fullfile = os.path.join(self.USML_DIR, "scattering_chapman_test.csv")
        print(f"reading {fullfile}")
        model = np.genfromtxt(fullfile, delimiter=',', skip_header=1)
        wind_speed = np.unique(model[:, 0])
        grazing = np.unique(model[:, 1])
        titles = ["0.6 kHz", "1.2 kHz", "2.4 kHz", "4.8 kHz"]
        labels = ["5 kts", "10 kts", "15 kts", "20 kts", "25 kts", "30 kts"]
        freqs = [600.0, 1_200, 2_400, 4_800]

        # compute and plot scattering strength vs. grazing angle
        fig, ax = plt.subplots(2, 2, figsize=[10, 8], sharex=True, sharey=True)
        ax = np.reshape(ax, (1, 4))
        for speed in wind_speed:
            index = model[:, 0] == speed
            scattering = model[index, 2:]
            for nfreq in range(4):
                beta = 158.0 * (speed * freqs[nfreq] ** (1.0 / 3.0)) ** (-0.58)
                theory = 2.6 - 42.4 * np.log10(beta) + 3.3 * beta * np.log10(grazing / 30.0)
                sub_ax = ax[0, nfreq]
                sub_ax.semilogx(grazing, theory)

        for speed in wind_speed:
            index = model[:, 0] == speed
            scattering = model[index, 2:]
            for nfreq in range(4):
                sub_ax = ax[0, nfreq]
                sub_ax.semilogx(grazing, scattering[:, nfreq], "k--")
                sub_ax.grid(True, which="both")
                sub_ax.xaxis.set_major_locator(MultipleLocator(10))
                sub_ax.xaxis.set_major_formatter(FormatStrFormatter("%.0f"))
                sub_ax.set_xlim(2, 79.9)
                sub_ax.set_ylim(-70, 0)
                sub_ax.set_title(titles[nfreq])

        ax[0, 2].set_xlabel('Grazing Angle (deg)')
        ax[0, 3].set_xlabel("Grazing Angle (deg)")
        ax[0, 0].set_ylabel("Scattering Strength (dB)")
        ax[0, 2].set_ylabel("Scattering Strength (dB)")
        ax[0, 0].legend(labels)

        output = fullfile.replace(".csv", ".png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        # compute and plot scattering strength vs. beta
        fig, ax = plt.subplots()
        index = model[:, 1] == 30.0
        for nfreq in range(4):
            scattering = model[index, 2 + nfreq]
            beta = 158.0 * (wind_speed * freqs[nfreq] ** (1.0 / 3.0)) ** (-0.58)
            ax.semilogx(beta, scattering, "o")

        beta = np.linspace(2.0, 20.0, 19)
        theory = 2.6 - 42.4 * np.log10(beta)
        ax.semilogx(beta, theory, "k-")
        ax.xaxis.set_major_formatter(FormatStrFormatter("%.0f"))
        ax.xaxis.set_minor_formatter(FormatStrFormatter("%.0f"))
        ax.grid(True, which="both")
        ax.set_xlim(2, 20)
        ax.set_ylim(-70, 0)
        ax.set_xlabel('Beta Factor')
        ax.set_ylabel("Scattering Strength (dB)")
        ax.legend(titles)

        output = fullfile.replace(".csv", "_beta.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()


if __name__ == '__main__':
    unittest.main()
