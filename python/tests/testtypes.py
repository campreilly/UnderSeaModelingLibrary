"""Test USML types module.
"""
import inspect
import os
import unittest

import matplotlib.pyplot as plt
import numpy as np


class TestTypes(unittest.TestCase):
    USML_DIR = os.path.join(os.path.dirname(os.path.abspath(os.path.join(__file__, os.pardir, os.pardir))),
                            "types", "test")

    def test_seq_rayfan(self):
        """Plot angle set of tangent spaced beams to illustrate features of USML's seq_rayfan class.
        """
        testname = inspect.stack()[0][3]
        print("=== " + testname + " ===")

        first = -90.0
        last = 90.0
        spread = 6.0
        center = 0.0
        num = 45
        radius = np.linspace(0, 12e3, 13)

        first_ang = first - center
        last_ang = last - center
        scale = (last_ang - first_ang) / (num - 1)
        n = np.linspace(0, num - 1, num)
        x = first_ang + scale * n
        angle = center + x

        angle = np.radians(angle)
        (aa, rr) = np.meshgrid(angle, radius)
        x = rr * np.cos(aa)
        y = -75.0 + rr * np.sin(aa)

        fig, (ax1, ax2) = plt.subplots(2)

        ax1.plot(x / 1e3, y, 'k-')
        ax1.set_title(f"{num} uniformly spaced rays")
        ax1.set_xlim(0, 10)
        ax1.set_ylim(-200, 0)
        ax1.set_xticklabels([])
        ax1.set_ylabel('Depth (m)')

        first_ang = np.arctan((first - center) / spread)
        last_ang = np.arctan((last - center) / spread)
        scale = (last_ang - first_ang) / (num - 1)
        x = first_ang + scale * n
        angle = center + np.tan(x) * spread

        angle = np.radians(angle)
        (aa, rr) = np.meshgrid(angle, radius)
        x = rr * np.cos(aa)
        y = -75.0 + rr * np.sin(aa)

        ax2.plot(x / 1e3, y, 'k-')
        ax2.set_title(f"{num} tangent spaced rays")
        ax2.set_xlim(0, 10)
        ax2.set_ylim(-200, 0)
        ax2.set_xlabel('Range (km)')
        ax2.set_ylabel('Depth (m)')

        output = os.path.join(self.USML_DIR, f"{testname}.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()

        fig, ax3 = plt.subplots(subplot_kw={'projection': 'polar'})
        ax3.plot(angle, np.ones(angle.shape), 'o')
        ax3.set_title("std = {:.3f} deg, min diff = {:.3f} deg".format(np.std(angle), np.min(np.diff(angle))))

        output = os.path.join(self.USML_DIR, f"{testname}_polar.png")
        print(f"saving {output}")
        plt.savefig(output)
        plt.close()


if __name__ == '__main__':
    unittest.main()
