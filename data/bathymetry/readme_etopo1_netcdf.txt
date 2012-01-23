ETOPO1 was built using GMT 4.3.1 (http://gmt.soest.hawaii.edu/), development
version/CVS.

GMT 4.3.1 creates grids in netCDF COARDS-compliant format. This is the format 
of the "gmt4" version in this directory, and is recommended for GMT 4 users. 

Some older versions of GDAL (http://www.gdal.org/) do not properly read the
netcdf grids created by GMT 4 (grids are inverted south to north).  If you are
using an older version of GDAL and encounter this problem you can work around
the problem a couple different ways.  If you have a version of GMT installed,
you can reformat the netcdf grid to a depreciated netcdf file that older GDAL
will read correctly, you can do this with the following command:
$ grdreformat Etopo1_gmt4.grd Etopo1_gdal.grd=cf

Another option would be to obtain the geotiff version of ETOPO1, found at the
following address:
http://www.ngdc.noaa.gov/mgg/global/global.html

Newer versions of GDAL should correctly read the gmt4 netcdf grids.

Please contact me if you have problems with ETOPO1.

Barry W. Eakins
barry.eakins@noaa.gov

ETOPO1 1 Arc-Minute Global Relief Model
http://www.ngdc.noaa.gov/mgg/global/global.html
