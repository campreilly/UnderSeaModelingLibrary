The module is a temporary kludge to provide the old 4.2 netcdf_c++ interface
in Microsoft Visual C++ in Windows.  Instead of compiling netcdf_c++ 
separately, we've update the CMakeList.txt file to incorporate the
NetCDF C++ classes directly into the USML libraries.
It was the only way that we could figure out that supported all
combinations of Release/Debug and Dynamic/Static libraries in Windows.
Our intent is to eventually replace this with an interface to the new
NetCDF-CXX4 interface.





