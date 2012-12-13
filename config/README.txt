In order to compile and run USML without turning off netcdf-4 and dap
in the netcdf package, you will need to install, from source, the
following packages:

-HDF5
-zLib
-cURL

All of these packages can be found at the link provided below:

ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4/

Once you have downloaded and extracted the files, you will need to pass
specific configuration settings to each ./configure command, as 
provided below:

cURL: no special configuration settings
zLib: no special configuration settings
HDF5: --with-zlib=/path/to/zlib --disable-parallel --enable-shared

After running ./configure with the above settings for each package
USML will now compile and run properly.
