import netCDF4
import numpy as np


class Bathymetry:
    """Loads bathymetry from netCDF4 file.
    The COARDS convention used by ETOPO and most other databases stores the data in arrays for longitude, latitude, and
    altitude. Longitude and latitude, are in degrees. Altitude is 2D height above mean sea level with positive in the
    upward direction.

    USML has an ability to write bathymetry data to a netCDF file from the data_grid class. But this file is in a
    a spherical coordinates  format. The first variable is the local earth radius for mean sea level, the second in
    distance from the center of curvature, the third is angle down from the North Pole, and the forth is angle around
    the equator from the Prime Meridian. The USML angles are in radians. This implementation searches the netCDF
    variables for one named "earth_radius", and if it is found, it converts from USML to COARDS conventions.

    see https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
    and https://www.ncei.noaa.gov/products/etopo-global-relief-model
    """
    latitude: np.array
    longitude: np.array
    altitude: np.array

    def __init__(self, filename: str) -> object:
        """Loads bathymetry from netCDF4 file."""
        nc = netCDF4.Dataset(filename)
        values = list(nc.variables.values())
        if "earth_radius" in nc.variables.keys():  # USML data_grid<2>
            R = values[0][:]
            Y = 90.0 - np.degrees(values[1][:])
            X = np.degrees(values[2][:])
            Z = values[3][:] - R
        else:  # COARDS
            R = None
            X = values[0][:]
            Y = values[1][:]
            Z = values[2][:]

        Z = np.reshape(Z, [len(Y), len(X)])

        self.longitude = X
        self.latitude = Y
        self.altitude = Z
