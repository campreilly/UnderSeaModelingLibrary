import netCDF4
import numpy as np


class Bathymetry:
    """Loads bathymetry from netCDF file.

    The COARDS convention used by the ETOPO Global Relief Model and most other databases stores the data in arrays for
    longitude, latitude, and altitude. Longitude and latitude, are in degrees. Altitude is 2D height above mean sea
    level with positive in the upward direction.

    USML has an ability to write bathymetry data to a netCDF file from the data_grid class. But the USML file is in a
    spherical coordinates format. The first variable is the local earth radius for mean sea level, the second in
    distance from the center of curvature, the third is angle down from the North Pole, and the forth is angle around
    the equator from the Prime Meridian. The USML angles are in radians. This implementation searches the netCDF
    variables for one named "earth_radius", and if it is found, it converts from USML to COARDS conventions.

    https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
    https://www.ncei.noaa.gov/products/etopo-global-relief-model
    """
    latitude: np.array(object=float, ndmin=1)
    longitude: np.array(object=float, ndmin=1)
    altitude: np.array(object=float, ndmin=2)

    def __init__(self, filename: str) -> object:
        """Loads bathymetry from netCDF file."""
        nc = netCDF4.Dataset(filename)
        values = list(nc.variables.values())
        if "earth_radius" in nc.variables.keys():  # USML data_grid<2>
            radius = values[0][:]
            north = 90.0 - np.degrees(values[1][:])
            east = np.degrees(values[2][:])
            up = values[3][:] - radius
        else:  # COARDS
            east = values[0][:]
            north = values[1][:]
            up = values[2][:]

        up = np.reshape(up, [len(north), len(east)])

        self.longitude = east
        self.latitude = north
        self.altitude = up


class Profile:
    """Loads ocean profile from netCDF file.

    The COARDS convention used by the World Ocean Atlas and most other databases stores ocean profile data in arrays
    for longitude, latitude, altitude, time, and data. Longitude and latitude, are in degrees. Altitude is height
    above mean sea level with positive in the upward direction. Time is day of the year. Data is a 4D array where
    dimensions are time, depth, latitude, and longitude. Ocean profiles are used for temperature, salinity,
    sound speed, and other scalar field.

    USML has an ability to write profile data to a netCDF file from the data_grid class. But the USML file is in a
    spherical coordinates format. The first variable is the local earth radius for mean sea level, the second in
    distance from the center of curvature, the third is angle down from the North Pole, and the forth is angle around
    the equator from the Prime Meridian. Time is not provided. The USML angles are in radians. This implementation
    searches the netCDF variables for one named "earth_radius", and if it is found, it converts from USML to
    COARDS conventions.

    https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
    https://www.ncei.noaa.gov/products/world-ocean-atlas
    """
    latitude: np.array(object=float, ndmin=1)
    longitude: np.array(object=float, ndmin=1)
    altitude: np.array(object=float, ndmin=1)
    time: np.array(object=float, ndmin=1)
    data: np.array(object=float, ndmin=4)

    def __init__(self, filename: str) -> object:
        """Loads ocean profile from netCDF file."""
        nc = netCDF4.Dataset(filename)
        values = list(nc.variables.values())
        if "earth_radius" in nc.variables.keys():  # USML data_grid<2>
            radius = values[0][:]
            up = values[1][:] - radius
            north = 90.0 - np.degrees(values[2][:])
            east = np.degrees(values[3][:])
            data = values[4][:]
            time = None
        else:  # COARDS
            east = values[0][:]
            north = values[1][:]
            up = values[2][:]
            time = values[3][:]
            data = values[4][:]

        up = np.reshape(data, [len(time), len(up), len(north), len(east)])

        self.longitude = east
        self.latitude = north
        self.altitude = up
        self.time = time
        self.data = data
