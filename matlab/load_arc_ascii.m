%
% load_arc_ascii.m - Loads ASCII Raster Grid File with an Arc Header
%
% Returns a bottom topology structure with the following fields:
%
%       latitude        bottom latitudes (degrees_north)
%       longitude       bottom longitudes (degrees_east)
%       altitude        bottom altitudes (meters)
%
function bathymetry = load_arc_ascii( filename )

% NCOLS  2401
% NROWS  1201
% XLLCENTER  -80.00000
% YLLCENTER  26.00000
% CELLSIZE 0.00083333
% NODATA_VALUE  999999

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
