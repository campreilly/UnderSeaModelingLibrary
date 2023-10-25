"""Numerical studies for Pedersen N^2 linear ocean sound speed profile
"""

import numpy as np
import scipy.integrate as integ
import scipy.interpolate as interp
import scipy.optimize as opt


class Eigenray(object):
    pass


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
        1. Sean M. Reilly, Gopu R. Potty, "Verification Tests for Hybrid Gaussian Beams in Spherical/Time Coordinates",
        Department of Ocean Engineering, University of Rhode Island, 10 May 2012
    """

    def __init__(self, surface_speed=1550.0, surface_grad=1.2):
        """Initialize ocean profile parameters

        :param surface_speed:   speed of sound at ocean surface (m/s)
        :param surface_grad:    sound speed gradient factor (m/s/m)
        """
        self.surface_speed = surface_speed
        self.surface_grad = surface_grad

    def sound_speed(self, depth):
        """Compute the n^2 linear sound speed profile at a specific depth.

        :param depth:           depth expressed as distance down from ocean surface (m)
        :return:                speed of sound at this depth (m/s)
        """
        sound_speed = self.surface_speed / np.sqrt(
            1 + 2 * self.surface_grad / self.surface_speed * depth)
        return sound_speed

    def vertex_diff(self, depth, ray_param):
        """Function used in root finding algorithm to search for vertex depth.

        In an n^2 linear environment, there is at most 1 turning point in the ray path, and that point always occurs
        above the source depth. This implementation uses eqn. (12) in reference 1 to search for the depth where the
        cosine of the depression/elevation angle equals 1. The result is positive when the selected depth is above
        the vertex point and negative when it is below the vertex point.

        :param depth:           depth expressed as distance down from ocean surface (m)
        :param ray_param:       Snell's Law ray parameter for current path (s/m)
        :return:                cosine difference (unitless)
        """
        return 1.0 - ray_param * self.sound_speed(depth)

    def range_integrand(self, depth, ray_param):
        """Function used in integration algorithm to compute horizontal range.

        This implementation uses eqn. (14) in reference 1 to compute horzitonal range from a definite integral. 
        Snell's Law tells us that horizontal range is the integral of cot(a). Note that at any point where cos(a)=1, 
        this integrand is infinite.
        
        :param depth:           depth expressed as distance down from ocean surface (m)
        :param ray_param:       Snell's Law ray parameter for current path (s/m)
        :return:                horizontal range integrand (unitless)
        """
        cosine = ray_param * self.sound_speed(depth)
        return cosine / np.sqrt(1.0 - cosine * cosine)

    def time_integrand(self, depth, ray_param):
        """Function used in integration algorithm to compute travel time.

        This implementation uses eqn. (15) in reference 1 to compute travel time from a definite integral. Snell's 
        Law tells us that travel time is the integral of 1/[c*sin(a)]. Note that at any point where cos(a)=1, 
        this integrand is infinite.
        
        :param depth:           depth expressed as distance down from ocean surface (m)
        :param ray_param:       Snell's Law ray parameter for current path (s/m)
        :return:                travel time integrand (s/m)
        """
        sound_speed = self.sound_speed(depth)
        cosine = ray_param * sound_speed
        return 1.0 / (sound_speed * np.sqrt(1.0 - cosine * cosine))

    def analytic_cycle(self, source_depth: float, source_angles: np.ndarray):
        """Compute analytic solution for range and travel time for one cycle of ray path.

        First, the scipy.optimize.bisect() function is used to search for the depth where the sound speed equals the
        vertex speed. At depths above the vertex, this value is positive. At depths below, it is negative. There is
        at most one vertex, and it is always above the source. If the sound speed difference does not change sign
        between the source depth and the ocean surface, then the vertex is truncated to the ocean surface.

        Next, the cotangent of the depression/elevation angle is integrated from the vertex to the source depth to
        estimate the horizontal range traversed by the ray path.
        
        Finally, the time_integrand along the ray path is integrated from the vertex to the source depth to estimate the
        travel time traversed by the ray path.

        :param source_depth:        source depth expressed as distance down from ocean surface (m)
        :param source_angles:       array of launch angles from source, up is positive (deg)
        :return:                    cycle range and cycle time
        """
        limits = np.asarray((0, source_depth))
        vertex_depth = np.zeros_like(source_angles)
        cycle_ranges = np.zeros_like(source_angles)
        cycle_times = np.zeros_like(source_angles)
        ray_param = np.cos(np.radians(source_angles)) / self.sound_speed(source_depth)

        # compute vertex range for each source angle
        for n in range(len(source_angles)):
            # compute the depth at which ray becomes horizontal
            dc = self.vertex_diff(limits, ray_param[n])
            if dc[0] * dc[1] < 0.0:
                vertex_depth[n] = opt.bisect(self.vertex_diff, limits[0], limits[1], args=ray_param[n])

            # use integral to compute the range at which vertex occurs
            result = integ.quad(self.range_integrand, vertex_depth[n], source_depth, args=ray_param[n],
                                full_output=True)
            cycle_ranges[n] = 2.0 * result[0]

            # use integral to compute the travel time along the ray path
            result = integ.quad(self.time_integrand, vertex_depth[n], source_depth, args=ray_param[n], full_output=True)
            cycle_times[n] = 2.0 * result[0]

        return cycle_ranges, cycle_times, vertex_depth, ray_param

    def eigenrays(self, source_depth: float, source_angles: np.ndarray, target_depth: float, target_ranges: np.ndarray):
        """Analytic solution for eigenrays to series of target locations.
        
        :param source_depth:        source depth expressed as distance down from ocean surface (m)
        :param source_angles:       array of launch angles from source, up is positive (deg)
        :param target_depth:        target depth expressed as distance down from ocean surface (m)
        :param target_ranges:       target range from source (m)
        :return: 
        """

        # use Snell's Law to estimate target angles for each source ray
        source_speed = self.sound_speed(source_depth)
        target_speed = self.sound_speed(target_depth)
        target_angles = np.cos(np.radians(source_angles)) / source_speed * target_speed

        # clip source and target angles with no eigenrays
        index = target_angles < 1.0
        source_angles = source_angles[index]
        target_angles = target_angles[index]
        target_angles = -np.degrees(np.arccos(target_angles))

        # estimate cycle range and time for each source and target angle
        source_cycle_ranges, source_cycle_times, vertex_depth, ray_param = \
            self.analytic_cycle(source_depth, source_angles)
        target_cycle_ranges, target_cycle_times, _, _ = self.analytic_cycle(target_depth, target_angles)

        # glue source and target cycles together
        cycle_ranges = 0.5 * (source_cycle_ranges + target_cycle_ranges)
        cycle_times = 0.5 * (source_cycle_times + target_cycle_times)

        # find break point between direct path and folded paths
        max_index = cycle_ranges.argmax()
        valid = np.logical_and( cycle_ranges[0] <= target_ranges, target_ranges <= cycle_ranges[max_index])
        target_ranges = target_ranges[valid]

        # compute eigenray products as a function of target range for direct path
        index = range(max_index)
        direct = Eigenray()
        direct.range = target_ranges
        direct.travel_time = interp.pchip_interpolate(cycle_ranges[index], cycle_times[index], target_ranges)
        direct.source_de = interp.pchip_interpolate(cycle_ranges[index], source_angles[index], target_ranges)
        direct.target_de = interp.pchip_interpolate(cycle_ranges[index], target_angles[index], target_ranges)

        # compute eigenray products as a function of target range for folded path
        index = range(len(source_angles) - 1, max_index - 1, -1)
        folded = Eigenray()
        folded.range = target_ranges
        folded.travel_time = interp.pchip_interpolate(cycle_ranges[index], cycle_times[index], target_ranges)
        folded.source_de = interp.pchip_interpolate(cycle_ranges[index], source_angles[index], target_ranges)
        folded.target_de = interp.pchip_interpolate(cycle_ranges[index], target_angles[index], target_ranges)

        return direct, folded


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
    m = r_vertex / c(r_vertex).

    References:
        1. Sean M. Reilly, Gopu R. Potty, "Verification Tests for Hybrid Gaussian Beams in Spherical/Time Coordinates",
        Department of Ocean Engineering, University of Rhode Island, 10 May 2012
    """

    def __init__(self, surface_speed=1550.0, surface_grad=1.2, earth_radius=6366.71e3):
        """Initialize ocean profile parameters

        :param surface_speed:   speed of sound at ocean surface (m/s)
        :param surface_grad:    sound speed gradient factor (m/s/m)
        :param earth_radius:    radius of the earth's curvature (m)
        """
        self.surface_speed = surface_speed
        self.surface_grad = surface_grad
        self.earth_radius = earth_radius

    def sound_speed(self, radius):
        """Compute the n^2 linear sound speed profile at a specific depth.

        :param radius:          depth expressed as distance up from center of curvature (m)
        :return:                speed of sound at this depth (m/s)
        """
        sound_speed = self.surface_speed / np.sqrt(
            1 + 2 * self.surface_grad / self.surface_speed * (self.earth_radius - radius))
        return sound_speed * radius / self.earth_radius

    def vertex_diff(self, radius, ray_param):
        """Function used in root finding algorithm to search for vertex depth.

        In an n^2 linear environment, there is at most 1 turning point in the ray path, and that point always occurs
        above the source depth. This implementation uses eqn. (16) in reference 1 to search for the depth where the
        cosine of the depression/elevation angle equals 1. The result is positive when the selected depth is above
        the vertex point and negative when it is below the vertex point.

        :param radius:          depth expressed as distance up from center of curvature (m)
        :param ray_param:       Snell's Law ray parameter for current path (s)
        :return:                cosine difference (unitless)
        """
        return 1.0 - ray_param * self.sound_speed(radius) / radius

    def range_integrand(self, radius, ray_param):
        """Function used in integration algorithm to compute horizontal range.

        This implementation uses eqn. (18) in reference 1 to compute horzitonal range from a definite integral. 
        Snell's Law tells us that horizontal range is the integral of cot(a)/r. Note that at any point where cos(a)=1,
        this integrand is infinite.

        :param depth:           depth expressed as distance down from ocean surface (m)
        :param ray_param:       Snell's Law ray parameter for current path (s)
        :return:                horizontal range integrand (unitless)
        """
        cosine = ray_param * self.sound_speed(radius) / radius
        return cosine / np.sqrt(1.0 - cosine * cosine) / radius

    def time_integrand(self, radius, ray_param):
        """Function used in integration algorithm to compute travel time.

        This implementation uses eqn. (19) in reference 1 to compute travel time from a definite integral. Snell's
        law in Spherical coordinates tells us that travel time is the integral of 1/[c*sin(a)]. Note that at any point
        where cos(a)=1, this integrand is infinite.

        :param depth:           depth expressed as distance down from ocean surface (m)
        :param ray_param:       Snell's Law ray parameter for current path (s)
        :return:                travel time integrand (s/m)
        """
        sound_speed = self.sound_speed(radius)
        cosine = ray_param * sound_speed / radius
        return 1.0 / (sound_speed * np.sqrt(1.0 - cosine * cosine))

    def analytic_cycle(self, source_depth: float, source_angles: np.ndarray):
        """Compute analytic solution for range and travel time for one cycle of ray path.

        First, the scipy.optimize.bisect() function is used to search for the depth where the sound speed equals the
        vertex speed. At depths above the vertex, this value is positive. At depths below, it is negative. There is
        at most one vertex, and it is always above the source. If the sound speed difference does not change sign
        between the source depth and the ocean surface, then the vertex is truncated to the ocean surface.

        Next, the cotangent of the depression/elevation angle is integrated from the vertex to the source depth to
        estimate the horizontal range traversed by the ray path.

        Finally, the time_integrand along the ray path is integrated from the vertex to the source depth to estimate the
        travel time traversed by the ray path.

        Note that both the cot() and time_integrand() functions have a singularity at the vertex, the point at which cos(
        a)=1. Using full_output=True in the integration supresses warnings and gives us the best estimate.

        :param source_depth:        source depth expressed as distance down from ocean surface (m)
        :param source_angles:       array of launch angles from source, up is positive (deg)
        :return:                    cycle range (m) and cycle time (s)
        """
        limits = self.earth_radius - np.asarray((0, source_depth))
        source_radius = self.earth_radius - source_depth
        vertex_radius = np.full_like(source_angles, self.earth_radius)
        cycle_ranges = np.zeros_like(source_angles)
        cycle_times = np.zeros_like(source_angles)
        ray_param = source_radius * np.cos(np.radians(source_angles)) / self.sound_speed(source_radius)

        # compute vertex range for each source angle
        for n in range(len(source_angles)):
            # compute the depth at which ray becomes horizontal
            dc = self.vertex_diff(limits, ray_param[n])
            if dc[0] * dc[1] < 0.0:
                vertex_radius[n] = opt.bisect(self.vertex_diff, limits[0], limits[1], args=ray_param[n])

            # use integral to compute the range at which vertex occurs
            result = integ.quad(self.range_integrand, source_radius, vertex_radius[n], args=ray_param[n],
                                full_output=True)
            cycle_ranges[n] = 2.0 * result[0] * source_radius

            # use integral to compute the travel time along the ray path
            result = integ.quad(self.time_integrand, source_radius, vertex_radius[n], args=ray_param[n],
                                full_output=True)
            cycle_times[n] = 2.0 * result[0]

        return cycle_ranges, cycle_times, vertex_radius, ray_param

    def eigenrays(self, source_depth: float, source_angles: np.ndarray, target_depth: float, target_ranges: np.ndarray):
        """Analytic solution for eigenrays to series of target locations.

        :param source_depth:        source depth expressed as distance down from ocean surface (m)
        :param source_angles:       array of launch angles from source, up is positive (deg)
        :param target_depth:        target depth expressed as distance down from ocean surface (m)
        :param target_ranges:       target range from source (m)
        :return:
        """

        # use Snell's Law to estimate target angles for each source ray
        source_radius = self.earth_radius - source_depth
        target_radius = self.earth_radius - target_depth
        source_speed = self.sound_speed(source_radius)
        target_speed = self.sound_speed(target_radius)
        target_angles = np.cos(np.radians(source_angles)) / source_speed * target_speed * source_radius / target_radius

        # clip source and target angles with no eigenrays
        index = target_angles < 1.0
        source_angles = source_angles[index]
        target_angles = target_angles[index]
        target_angles = -np.degrees(np.arccos(target_angles))

        # estimate cycle range and time for each source and target angle
        source_cycle_ranges, source_cycle_times, vertex_radius, ray_param = \
            self.analytic_cycle(source_depth, source_angles)
        target_cycle_ranges, target_cycle_times, _, _ = self.analytic_cycle(target_depth, target_angles)

        # glue source and target cycles together
        cycle_ranges = 0.5 * (source_cycle_ranges + target_cycle_ranges)
        cycle_times = 0.5 * (source_cycle_times + target_cycle_times)

        # find break point between direct path and folded paths
        max_index = cycle_ranges.argmax()
        valid = np.logical_and( cycle_ranges[0] <= target_ranges, target_ranges <= cycle_ranges[max_index])
        target_ranges = target_ranges[valid]

        # compute eigenray products as a function of target range for direct path
        index = range(max_index)
        direct = Eigenray()
        direct.range = target_ranges
        direct.travel_time = interp.pchip_interpolate(cycle_ranges[index], cycle_times[index], target_ranges)
        direct.source_de = interp.pchip_interpolate(cycle_ranges[index], source_angles[index], target_ranges)
        direct.target_de = interp.pchip_interpolate(cycle_ranges[index], target_angles[index], target_ranges)

        # compute eigenray products as a function of target range for folded path
        index = range(len(source_angles) - 1, max_index - 1, -1)
        folded = Eigenray()
        folded.range = target_ranges
        folded.travel_time = interp.pchip_interpolate(cycle_ranges[index], cycle_times[index], target_ranges)
        folded.source_de = interp.pchip_interpolate(cycle_ranges[index], source_angles[index], target_ranges)
        folded.target_de = interp.pchip_interpolate(cycle_ranges[index], target_angles[index], target_ranges)

        return direct, folded
