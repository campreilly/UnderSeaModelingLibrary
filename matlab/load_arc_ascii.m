%
% load_arc_ascii.m - Loads ASCII Raster Grid File with an Arc Header
%
% Each row in the data represents the depth as a function of decreasing
% latitude, which is the Y coordinate. Each column represents the depth as
% a function of increasing longitude, which is the X coordinate. The first
% data point is therefore the depth at north-west corner. If the data is
% read in reverse latitude order, then the first point will become the
% south-west corner specified by longitude=XLLCENTER and
% latitude=YLLCENTER). Each depth represents the average value over the
% CELLSIZE x CELLSIZE area centered on the specified longitude and
% latitude.
%
% This format is one of the options used by the Geophysical Data System
% (GEODAS) Search and Data Retrieval web site. to distribute custom
% bathymetry grids.
%
% Returns a bottom topology structure with the following fields:
%
%       latitude        bottom latitudes (degrees_north)
%       longitude       bottom longitudes (degrees_east)
%       altitude        bottom altitudes (meters)
%
function bathymetry = load_arc_ascii( filename )

fi = fopen(filename) ;

% read the header from this text file

header = textscan(fi,'%s %f',6) ;
values = cell2mat( header(1,2) ) ;
ncols = values(1) ;
nrows = values(2) ;
xllcenter = values(3) ;
yllcenter = values(4) ;
cellsize = values(5) ;
nodata = values(6) ;

longitude = xllcenter + (0:(ncols-1))*cellsize ;
latitude = yllcenter + (0:(nrows-1))*cellsize ;

% read the data

a = fscanf( fi, '%f', nrows*ncols ) ;
altitude = flipud( reshape( a, ncols, nrows )' ) ;
 
bathymetry = struct( ...
    'latitude', double(latitude), ...
    'longitude', double(longitude), ...
    'altitude', double(altitude) ) ;

fclose(fi);
