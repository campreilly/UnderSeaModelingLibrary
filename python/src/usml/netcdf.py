"""Read netCDF files created by USML.
"""
import netCDF4
import numpy as np


class Struct:
    """Dynamically defined data structure for eading netCDF variable arrays."""
    pass


def read(filename: str):
    """Read variable arrays from netCDF file.

    Many of the netCDF files created by USML can be read as a data structure where each field is a np.array object.
    These include wavefront, eigenverb, and bistatic eigenverb files.
    """
    nc = netCDF4.Dataset(filename)
    obj = Struct()
    for attr in nc.variables.keys():
        setattr(obj, attr, nc.variables[attr][:])
    return obj


class Bathymetry:
    """Loads bathymetry from netCDF file.

    The COARDS convention used by the ETOPO Global Relief Model and most other databases stores the data in arrays
    for longitude, latitude, and altitude. Longitude and latitude, are in degrees. Altitude is 2D height above mean
    sea level with positive in the upward direction.

    USML has an ability to write bathymetry data to a netCDF file from the data_grid class. But the USML file is in a
    spherical coordinates format. The first variable is the local earth radius for mean sea level, the second in
    distance from the center of curvature, the third is angle down from the North Pole, and the forth is angle around
    the equator from the Prime Meridian. The USML angles are in radians. This implementation searches the netCDF
    variables for one named "earth_radius", and if it is found, it converts from USML to COARDS conventions.

    https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
    https://www.ncei.noaa.gov/products/etopo-global-relief-model
    """

    def __init__(self, filename: str, lat_range=None, lng_range=None):
        """Loads bathymetry from netCDF file.

         The variables to be loaded are deduced by their dimensionality. The first variable to have 2 dimensions is
         assumed to be depth. The first dimension of the depth is assumed to be latitude. The second dimension of the
         depth is assumed to be longitude. This may seem like a pretty loose specification, but this looseness is
         very helpful in automating the reading NetCDF files from a variety of sources.

         This implementation does not yet automatically unwrap differences between the [0,360) and the [-180,
         180) longitude range.

        :param filename:    Name of file to load from disk
        :param lat_range:   Range of latitude to load (degrees_north)
        :param lng_range:   Range of longitude to load (degrees_east)
        """
        ds = netCDF4.Dataset(filename)
        variables = ds.variables

        # search for depth data
        depth = None
        for values in variables.values():
            if len(values.dimensions) == 2:
                depth = values
                break

        # search for latitude and longitude data
        latitude = variables[depth.dimensions[0]][:]
        longitude = variables[depth.dimensions[1]][:]

        # convert USML data_grid files from rho/theta/phi to lat/long/alt
        if "earth_radius" in variables.keys():
            earth_radius = variables["earth_radius"][:]
            latitude = 90.0 - np.degrees(latitude)
            longitude = np.degrees(longitude)
        else:
            earth_radius = 0.0

        # find slice of latitude to use
        if lat_range is None:
            lat_index = range(len(latitude))
        else:
            lat_index = np.asarray((latitude >= lat_range[0]) & (latitude <= lat_range[-1] + 1e-6)).nonzero()
            lat_index = lat_index[0]  # extract list from tuple
        self.latitude = latitude[lat_index]

        # find slice of longitude to use
        if lng_range is None:
            lng_index = range(len(longitude))
        else:
            lng_index = np.asarray((longitude >= lng_range[0]) & (longitude <= lng_range[-1] + 1e-6)).nonzero()
            lng_index = lng_index[0]  # extract list from tuple
        self.longitude = longitude[lng_index]

        # extract depth for only these latitudes and longitudes
        self.altitude = depth[lat_index, lng_index] - earth_radius


class Profile:
    """Loads ocean profile from netCDF file.

    The COARDS convention used by the World Ocean Atlas and most other databases stores ocean profile data in arrays
    for longitude, latitude, altitude, time, and data. Longitude and latitude, are in degrees. Altitude is height
    above mean sea level with positive in the upward direction. Time is day of the year. Data is a 4D array where
    dimensions are time, depth, latitude, and longitude. Ocean profiles are used for temperature, salinity,
    sound speed, and other scalar fields. Extracts netCDF variable by their position in the file to support a wide
    variety of variable names.

    USML has an ability to write profile data to a netCDF file from the data_grid class. But the USML file is in a
    spherical coordinates format. The first variable is the local earth radius for mean sea level, the second in
    distance from the center of curvature, the third is angle down from the North Pole, and the forth is angle around
    the equator from the Prime Meridian. Time is not provided. The USML angles are in radians. This implementation
    searches the netCDF variables for one named "earth_radius", and if it is found, it converts from USML to
    COARDS conventions.

    https://ferret.pmel.noaa.gov/Ferret/documentation/coards-netcdf-conventions
    https://www.ncei.noaa.gov/products/world-ocean-atlas
    """

    def __init__(self, filename: str, lat_range=None, lng_range=None, profile_name: str = None):
        """Loads ocean profile from netCDF file.

        If the profile_name is specified, then it extracts that variable name from the netCDF file and treats that as
        the profile data. This allows us to read HYCOM files where both the temperature and salinity are stored in
        the same file. If the profile name is not specified, it looks for the first variable with 3 or more
        dimensions and treats that as the profile. The last three dimensions of the profile are assumed to be depth,
        latitude, and longitude. If there are more than 3 dimensions, the 4th from the end is assumed to be time.

        If the file includes a variable called "earth_radius", we assume that it was read from the USML data_grid class
        and that the coordinate system must be converted from rho/theta/phi to depth/lat/long.

        If the profile has 4 dimensions, but the time variable has a size of 1, then only the last three dimensions
        stored.

        This implementation does not yet automatically unwrap differences between the [0,360) and the [-180,
        180) longitude range.

        :param filename:        Name of file to load from disk
        :param lat_range:       Range of latitude to load (degrees_north)
        :param lng_range:       Range of longitude to load (degrees_east)
        :param profile_name:    Profile name to search for (string)
        """
        ds = netCDF4.Dataset(filename)
        variables = ds.variables

        # search for profile data
        profile = None
        if profile_name is None:
            for values in variables.values():
                if len(values.dimensions) >= 3:
                    profile = values
                    break
        else:
            profile = variables[profile_name]

        # search for depth, latitude, and longitude data, backwards from the end
        depth = variables[profile.dimensions[-3]][:]
        latitude = variables[profile.dimensions[-2]][:]
        longitude = variables[profile.dimensions[-1]][:]

        # convert USML data_grid files from rho/theta/phi to lat/long/alt
        if "earth_radius" in variables.keys():
            earth_radius = variables["earth_radius"][:]
            latitude = 90.0 - np.degrees(latitude)
            longitude = np.degrees(longitude)
        else:
            earth_radius = 0.0
        self.altitude = depth - earth_radius

        # find time variable if it exists
        if len(profile.dimensions) > 3:
            self.time = variables[profile.dimensions[-4]][:]
        else:
            self.time = np.asarray([0.0])

        # find slice of latitude to use
        if lat_range is None:
            lat_index = range(len(latitude))
        else:
            lat_index = np.asarray((latitude >= lat_range[0]) & (latitude <= lat_range[-1] + 1e-6)).nonzero()
            lat_index = lat_index[0]  # extract list from tuple
        self.latitude = latitude[lat_index]

        # find slice of longitude to use
        if lng_range is None:
            lng_index = range(len(longitude))
        else:
            lng_index = np.asarray((longitude >= lng_range[0]) & (longitude <= lng_range[-1] + 1e-6)).nonzero()
            lng_index = lng_index[0]  # extract list from tuple
        self.longitude = longitude[lng_index]

        # extract profile for only these latitudes and longitudes
        if "earth_radius" in variables.keys():
            self.data = profile[:, lat_index, lng_index]
        else:
            self.data = profile[:, :, lat_index, lng_index]
            if self.data.ndim == 4 and self.time.size == 1:
                self.data = self.data[0, :, :, :]


class EigenrayList:
    """Loads USML eigenrays for matrix of targets from netCDF file.

    USML computes eigenrays from a single source location to a 2D matrix of targets. Each target has its own ID number,
    latitude, longitude, and altitude. Each target has a 1D list of Eigenrays objects. Note that this implementation
    uses the new file format introduced in the USML 3.0 release.
    """

    def __init__(self, filename: str):
        """Loads eigenray list from netCDF file."""
        nc = netCDF4.Dataset(filename)

        # load file header variables
        self.sourceID = int(nc.variables["sourceID"][0])
        self.source_latitude = float(nc.variables["source_latitude"][0])
        self.source_longitude = float(nc.variables["source_longitude"][0])
        self.source_altitude = float(nc.variables["source_altitude"][0])
        self.targetID = nc.variables["targetID"][:]
        self.latitude = nc.variables["latitude"][:]
        self.longitude = nc.variables["longitude"][:]
        self.altitude = nc.variables["altitude"][:]
        self.initial_time = nc.variables["initial_time"][:]
        self.frequencies = nc.variables["frequencies"][:]
        eigenray_index = nc.variables["eigenray_index"][:]
        eigenray_num = nc.variables["eigenray_num"][:]

        num_rows, num_cols = self.latitude.shape
        self.eigenrays = [[Struct() for j in range(num_cols)] for i in range(num_rows)]

        # find eigenrays for each target
        for nrow in range(num_rows):
            for ncol in range(num_cols):
                ray = Struct()
                offset = eigenray_index[nrow][ncol]
                index = range(offset, offset + eigenray_num[nrow][ncol])
                ray.intensity = nc.variables["intensity"][index][:]
                ray.phase = nc.variables["phase"][index][:]
                ray.travel_time = nc.variables["travel_time"][index]
                ray.source_de = nc.variables["source_de"][index]
                ray.source_az = nc.variables["source_az"][index]
                ray.target_de = nc.variables["target_de"][index]
                ray.target_az = nc.variables["target_az"][index]
                ray.surface = nc.variables["surface"][index]
                ray.bottom = nc.variables["bottom"][index]
                ray.caustic = nc.variables["caustic"][index]
                ray.upper = nc.variables["upper"][index]
                ray.lower = nc.variables["lower"][index]
                self.eigenrays[nrow][ncol] = ray
