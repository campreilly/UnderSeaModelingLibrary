import netCDF4
import numpy as np


class Bathymetry:
    """Loads bathymetry from netCDF file.

    The COARDS convention used by the ETOPO Global Relief Model and most other databases stores the data in arrays
    for longitude, latitude, and altitude. Longitude and latitude, are in degrees. Altitude is 2D height above mean
    sea level with positive in the upward direction. Extracts netCDF variable by their position in the file to
    support a wide variety of variable names.

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

    def __init__(self, filename: str):
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
    latitude: np.array(object=float, ndmin=1)
    longitude: np.array(object=float, ndmin=1)
    altitude: np.array(object=float, ndmin=1)
    time: np.array(object=float, ndmin=1)
    data: np.array(object=float, ndmin=4)

    def __init__(self, filename: str):
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


class Eigenrays:
    """List of USML eigenrays for a specific target

    Attributes:
        intensity           transmission loss at each frequency (dB)
        phase               path phase adjustment at each frequency (rad)
        travel_time         path travel time (secs).
        source_de           source D/E angle (degrees)
        source_az           source AZ angle (degrees_true)
        target_de           target D/E angle (degrees)
        target_az           target AZ angle (degrees_true)
        surface             number of surface reflections (count)
        bottom              number of bottom reflections (count)
        caustic             number of caustic encounters (count)
        upper               number of upper vertices (count)
        lower               number of lower vertices (count)
    """
    intensity: np.array(object=float, ndmin=2)
    phase: np.array(object=float, ndmin=2)
    travel_time: np.array(object=float, ndmin=1)
    source_de: np.array(object=float, ndmin=1)
    source_az: np.array(object=float, ndmin=1)
    target_de: np.array(object=float, ndmin=1)
    target_az: np.array(object=float, ndmin=1)
    surface: np.array(object=int, ndmin=1)
    bottom: np.array(object=int, ndmin=1)
    caustic: np.array(object=int, ndmin=1)
    upper: np.array(object=int, ndmin=1)
    lower: np.array(object=int, ndmin=1)


class EigenrayList:
    """Loads USML eigenrays for matrix of targets from netCDF file.

    USML computes eigenrays from a single source location to a 2D matrix of targets. Each target has its own ID number,
    latitude, longitude, and altitude. Each target has a 1D list of Eigenrays objects. Note that this implementation
    uses the new file format introduced in the USML 3.0 release.

    Attributes:
        sourceID            source identification number (0 if unknown)
        source_latitude     source latitudes (degrees_north)
        source_longitude    source longitudes (degrees_east)
        source_altitude     target altitudes (meters)
        targetID	        target identification number (0 if unknown)
        latitude            target latitudes (degrees_north)
        longitude           target longitudes (degrees_east)
        altitude            target altitudes (meters)
        initial_time        earliest eigenray arrival time (secs)
        frequency           propagation frequency (hertz)
        eigenrays           list of eigenrays for each target
    """
    sourceID: int
    source_latitude: float
    source_longitude: float
    source_altitude: float
    targetID: np.array(object=int, ndmin=2)
    latitude: np.array(object=float, ndmin=2)
    longitude: np.array(object=float, ndmin=2)
    altitude: np.array(object=float, ndmin=2)
    initial_time: np.array(object=float, ndmin=2)
    frequency: np.array(object=float, ndmin=1)
    eigenrays: list[list[Eigenrays]]

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
        self.frequency = nc.variables["frequency"][:]
        eigenray_index = nc.variables["eigenray_index"][:]
        eigenray_num = nc.variables["eigenray_num"][:]

        num_rows, num_cols = self.latitude.shape
        self.eigenrays = [[Eigenrays() for j in range(num_cols)] for i in range(num_rows)]

        # find eigenrays for each target
        for nrow in range(num_rows):
            for ncol in range(num_cols):
                ray = Eigenrays()
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


class EigenverbList:
    """Loads USML eigenverbs for a specific sourcefrom netCDF file.

    USML computes eigenrays from a single source location to a 2D matrix of targets. Each target has its own ID number,
    latitude, longitude, and altitude. Each target has a 1D list of Eigenrays objects. Note that this implementation
    uses the new file format introduced in the USML 3.0 release.

    Attributes:
        travel_time         path travel time (secs).
        frequency           propagation frequency (hertz)
        power               intensity integrated over all locations on the interface (dB)
        length              half length of the D/E projection of ray path onto interface (meters)
        width               half width of the AZ projection of ray path onto interface (meters)
        latitude            collision latitudes (degrees_north)
        longitude           collision longitudes (degrees_east)
        altitude            collision altitudes (meters)
        direction           compass heading for the "length" axis (degrees_true)
        grazing             angle to interface tangent plane at point of impact (degrees, positive is up)
        sound_speed         sound speed at point of impact (m/s)
        de_index            index number of the source DE angle (count)
        az_index            index number of the source AZ angle (count)
        source_de           source D/E angle (degrees)
        source_az           source AZ angle (degrees_true)
        surface             number of surface reflections (count)
        bottom              number of bottom reflections (count)
        caustic             number of caustic encounters (count)
        upper               number of upper vertices (count)
        lower               number of lower vertices (count)
    """
    travel_time: np.array(object=float, ndmin=1)
    frequency: np.array(object=float, ndmin=1)
    power: np.array(object=float, ndmin=1)
    length: np.array(object=float, ndmin=1)
    width: np.array(object=float, ndmin=1)
    latitude: np.array(object=float, ndmin=1)
    longitude: np.array(object=float, ndmin=1)
    altitude: np.array(object=float, ndmin=1)
    direction: np.array(object=float, ndmin=1)
    grazing: np.array(object=float, ndmin=1)
    sound_speed: np.array(object=float, ndmin=1)
    de_index: np.array(object=int, ndmin=1)
    az_index: np.array(object=int, ndmin=1)
    source_de: np.array(object=float, ndmin=1)
    source_az: np.array(object=float, ndmin=1)
    surface: np.array(object=int, ndmin=1)
    bottom: np.array(object=int, ndmin=1)
    caustic: np.array(object=int, ndmin=1)
    upper: np.array(object=int, ndmin=1)
    lower: np.array(object=int, ndmin=1)

    def __init__(self, filename: str):
        """Loads eigenverb list from netCDF file."""
        nc = netCDF4.Dataset(filename)
        for attr in nc.variables.keys():
            setattr(self, attr, nc.variables[attr][:])

        # unit conversions
        # self.direction = np.degrees(self.direction)
        # self.grazing = np.degrees(self.grazing)