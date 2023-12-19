"""Use analytic solutions to test WaveQ3D results for Pedersen n^2 linear ocean sound speed profile
"""

import numpy as np

import usml.netcdf


def wq3d_eigenrays(filename: str, srf: int = 0, btm: int = 0, upr: int = None, lwr: int = None,
                   phase: float = None) -> usml.netcdf.Eigenrays:
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

                ok = model_srf == srf and model_btm == btm
                if upr is not None:
                    ok = ok and model_upr == upr

                if lwr is not None:
                    ok = ok and model_lwr == lwr

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
