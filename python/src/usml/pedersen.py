"""Numerical studies for Pedersen N^2 linear ocean sound speed profile
"""

import numpy as np
import scipy.integrate as integ
import scipy.optimize as opt


class PedersenCartesian:
    """Analytic solution for the ray paths of the Pedersen n^2 linear profile in a Cartesian coordinate system.

    The Pedersen profile is an extreme test case for a downward refracting n^2 linear profile that is useful in
    illustrating the effects of caustics at the edge of a shadow zone.

        c(z) = c0 / sqrt( 1 + 2 g0 z / c0 )

    Snell's Law in Cartesian coordinates relates the depression/elevation angle at eaach point to the sound speed.

        m = cos(a) / c(z) = constant

    where z = distance down from ocean surface, a = depression/elevation angle, c(z) = sound speed, m = ray parameter.
    The vertex is defined as the minimum depth achieved by the ray path.  At the vertex, cos(a)=1 and
    m = 1 / c(z_vertex).

    References:
        - M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied to Underwater Acoustic conditions
          of Extreme Downward Refraction", J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
    """

    def __init__(self, surface_speed=1550.0, surface_grad=1.2):
        """Initialize ocean profile parameters

        :param surface_speed:   speed of sound at ocean surface (m/s)
        :param surface_grad:    sound speed gradient factor (m/s/m)
        """
        self.surface_speed = surface_speed
        self.surface_grad = surface_grad

    def sound_speed(self, depth):
        """Compute the Pedersen profile sound speed at a specific depth.

        :param depth:           depth expressed as distance down from ocean surface, can be a vector
        :return:                speed of sound at this depth, can be a vector
        """
        sound_speed = self.surface_speed / np.sqrt(
            1 + 2 * self.surface_grad / self.surface_speed * depth)
        return sound_speed

    def param_diff(self, depth, vertex_speed):
        """Sound speed difference from vertex speed.

        :param depth:           depth expressed as distance down from ocean surface, can be a vector
        :param vertex_speed:    speed of sound at vertex
        :return:                sound speed difference, can be a vector
        """
        sound_speed = self.sound_speed(depth)
        speed_diff = sound_speed - vertex_speed
        return speed_diff

    def cot_angle(self, depth, vertex_speed):
        """Snell's law calculation of the cot(angle) at a given depth.

        :param depth:           depth expressed as distance down from ocean surface
        :param vertex_speed:    speed of sound at vertex
        :return:                cotangent of depression/elevation angle
        """
        sound_speed = self.sound_speed(depth)
        cosine = sound_speed / vertex_speed
        cot = cosine / np.sqrt(1.0 - cosine * cosine)
        return cot

    def slowness(self, depth, vertex_speed):
        """Snell's law calculation of the travel time.

        :param depth:           depth expressed as distance down from ocean surface
        :param vertex_speed:    speed of sound at vertex
        :return:                cotangent of depression/elevation angle
        """
        sound_speed = self.sound_speed(depth)
        cosine = sound_speed / vertex_speed
        time = 1.0 / np.sqrt(1.0 - cosine * cosine) / sound_speed
        return time

    def analytic_cycle(self, source_depth, source_angles):
        """Compute analytic solution for range and travel time for one cycle of ray path.

        First, the scipy.optimize.bisect() function is used to search for the depth where the sound speed equals the
        vertex speed. At depths above the vertex, this value is positive. At depths below, it is negative. There is
        at most one vertex, and it is always above the source. If the sound speed difference does not change sign
        between the source depth and the ocean surface, then the vertex is truncated to the ocean surface.

        Next, the cotangent of the depression/elevation angle is integrated from the vertex to the source depth to
        estimate the horizontal range traversed by the ray path.
        
        Finally, the slowness along the ray path is integrated from the vertex to the source depth to estimate the
        travel time traversed by the ray path.

        :param source_depth:        source depth expressed as distance down from ocean surface
        :param source_angles:       array of launch angles from source, up is positive
        :return:                    cycle range and cycle time
        """
        # compute ray parameters
        launch_angles = np.radians(source_angles)
        length_angles = len(launch_angles)

        source_speed = self.sound_speed(source_depth)
        ray_param = np.cos(launch_angles) / source_speed

        vertex_speed = (1.0 / ray_param)
        limits = np.asarray((0, source_depth))

        vertex_depth = np.zeros_like(launch_angles)
        cycle_ranges = np.zeros_like(launch_angles)
        cycle_times = np.zeros_like(launch_angles)

        # compute vertex range for each source angle
        for n in range(length_angles):
            # compute the depth at which ray becomes horizontal
            dc = self.param_diff(limits, vertex_speed[n])
            if dc[0] * dc[1] < 0.0:
                vertex_depth[n] = opt.bisect(self.param_diff, limits[0], limits[1], args=vertex_speed[n])

            # use integral to compute the range at which vertex occurs
            result = integ.quad(self.cot_angle, vertex_depth[n], source_depth, args=vertex_speed[n], full_output=True)
            cycle_ranges[n] = 2.0 * result[0]

            # use integral to compute the travel time along the ray path
            result = integ.quad(self.slowness, vertex_depth[n], source_depth, args=vertex_speed[n], full_output=True)
            cycle_times[n] = 2.0 * result[0]

        return cycle_ranges, cycle_times


class PedersenSpherical:
    """Analytic solution for the ray paths of the Pedersen n^2 linear profile in a Spherical coordinate system.

    The Pedersen profile is an extreme test case for a downward refracting n^2 linear profile that is useful in
    illustrating the effects of caustics at the edge of a shadow zone. A "flat Earth" correction is applied to
    simplify comparisons to the Cartesian coordinate system.

        c(r) = c0 / sqrt( 1 + 2 g0 (R-r) / c0 ) r / R

    Snell's Law in Spherical coordinates has an extra scaling factor for radius.

        m = r cos(a) / c(r) = constant ray parameter

    where r = radial distance from earth center, a = depression/elevation angle, c(r) = sound speed, m = ray parameter.
    The vertex is defined as the maximum radius achieved by the ray path.  At the vertex, cos(a)=1 and
    m = 1 / c(r_vertex).

    References:
        - M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied to Underwater Acoustic conditions
          of Extreme Downward Refraction", J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
    """

    def __init__(self, surface_speed=1550.0, surface_grad=1.2,
                 earth_radius=6378101.030201019):
        """Initialize ocean profile parameters

        :param surface_speed:   speed of sound at ocean surface (m/s)
        :param surface_grad:    sound speed gradient factor (m/s/m)
        :param earth_radius:    radius of the earth's curvature (m)
        """
        self.surface_speed = surface_speed
        self.surface_grad = surface_grad
        self.earth_radius = earth_radius

    def sound_speed(self, radius):
        """Compute the Pedersen profile sound speed at a specific depth.

        :param radius:          depth expressed as distance up from center of curvature, can be a vector
        :return:                speed of sound at this depth, can be a vector
        """
        sound_speed = self.surface_speed / np.sqrt(
            1 + 2 * self.surface_grad / self.surface_speed * (self.earth_radius - radius))
        return sound_speed * radius / self.earth_radius

    def param_diff(self, radius, vertex_speed):
        """Sound speed difference from vertex speed.

        :param radius:          depth expressed as distance up from center of curvature, can be a vector
        :param vertex_speed:    speed of sound at vertex
        :return:                sound speed difference, can be a vector
        """
        sound_speed = self.sound_speed(radius) / radius
        speed_diff = sound_speed - vertex_speed
        return speed_diff

    def cot_angle(self, radius, vertex_speed):
        """Snell's law calculation of the cot(angle) at a given depth.

        :param radius:          depth expressed as distance up from center of curvature
        :param vertex_speed:    speed of sound at vertex
        :return:                cotangent of depression/elevation angle
        """
        sound_speed = self.sound_speed(radius)
        cosine = sound_speed / vertex_speed / radius
        cot = cosine / np.sqrt(1.0 - cosine * cosine) / radius
        return cot

    def slowness(self, radius, vertex_speed):
        """Snell's law calculation of the travel time.

        :param radius:          depth expressed as distance up from center of curvature
        :param vertex_speed:    speed of sound at vertex
        :return:                cotangent of depression/elevation angle
        """
        sound_speed = self.sound_speed(radius)
        cosine = sound_speed / vertex_speed / radius
        time = 1.0 / np.sqrt(1.0 - cosine * cosine) / sound_speed
        return time

    def analytic_cycle(self, source_depth, source_angles):
        """Compute analytic solution for range and travel time for one cycle of ray path.

        First, the scipy.optimize.bisect() function is used to search for the depth where the sound speed equals the
        vertex speed. At depths above the vertex, this value is positive. At depths below, it is negative. There is
        at most one vertex, and it is always above the source. If the sound speed difference does not change sign
        between the source depth and the ocean surface, then the vertex is truncated to the ocean surface.

        Next, the cotangent of the depression/elevation angle is integrated from the vertex to the source depth to
        estimate the horizontal range traversed by the ray path.

        Finally, the slowness along the ray path is integrated from the vertex to the source depth to estimate the
        travel time traversed by the ray path.

        Note that both the cot() and slowness() functions have a singularity at the vertex, the point at which cos(
        a)=1. Using full_output=True in the integration supresses warnings and gives us the best estimate.

        :param source_depth:        source depth expressed as distance down from ocean surface
        :param source_angles:       array of launch angles from source, up is positive
        :return:                    cycle range and cycle time
        """
        # compute ray parameters
        launch_angles = np.radians(source_angles)
        length_angles = len(launch_angles)
        cycle_ranges = np.zeros_like(launch_angles)
        cycle_times = np.zeros_like(launch_angles)

        source_radius = self.earth_radius - source_depth
        source_speed = self.sound_speed(source_radius)
        ray_param = source_radius * np.cos(launch_angles) / source_speed

        vertex_speed = 1.0 / ray_param
        limits = self.earth_radius - np.asarray((0, source_depth))
        vertex_radius = np.full_like(launch_angles, self.earth_radius)

        # compute vertex range for each source angle
        for n in range(length_angles):
            # compute the depth at which ray becomes horizontal
            dc = self.param_diff(limits, vertex_speed[n])
            if dc[0] * dc[1] < 0.0:
                vertex_radius[n] = opt.bisect(self.param_diff, limits[0], limits[1], args=vertex_speed[n])

            # use integral to compute the range at which vertex occurs
            result = integ.quad(self.cot_angle, source_radius, vertex_radius[n], args=vertex_speed[n], full_output=True)
            cycle_ranges[n] = 2.0 * result[0] * source_radius

            # use integral to compute the travel time along the ray path
            result = integ.quad(self.slowness, source_radius, vertex_radius[n], args=vertex_speed[n], full_output=True)
            cycle_times[n] = 2.0 * result[0]

        return cycle_ranges, cycle_times, vertex_speed, vertex_radius
