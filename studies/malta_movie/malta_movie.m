%
% malta_movie - wavefront 3-D visualization
%
% Demonstrates the model's ability to visualize the wavefront in 3-D.  
% Uses the World Ocean Atlas and the ETOPO1 databases to construct a
% real world environment near the Malta escarpment south-east of Sicily.
% 
%      - Area: 34.5 to 37.0 North, 14,5 to 17.0 East
%      - Month: December
%      - Source: 36N, 16.0E, 10 meters deep
%      - Frequency: 3000 Hz
%      - Travel Time: 80 seconds
%      - Time Step: 50 msec
%      - D/E: [-45,0] in 1.0 deg steps
%      - AZ: [180,360] in 10.0 deg steps
%
% The resulting wavefronts are stored in the "malta_movie_test.nc" netCDF
% file for later plotting by Matlab, Octave, or other analysis routines.
%
clear all ; close all
disp('*** malta_movie - wavefront 3-D visualization ***');
hbar = waitbar(0,['Loading data ...']);
waitbar(0.05);

% minlat = 35.5 ; maxlat=36.5 ;
% minlng = 15.3; maxlng=16.1 ;
minlat = 35.3 ; maxlat=36.7 ;
minlng = 15.0; maxlng=16.4 ;

bathymetry = load_bathymetry('../../data/bathymetry/med_ocean.nc') ;
n = find( bathymetry.latitude >= minlat & bathymetry.latitude <= maxlat ) ;
m = find( bathymetry.longitude >= minlng & bathymetry.longitude <= maxlng ) ;
bathymetry.latitude = bathymetry.latitude(n) ;
bathymetry.longitude = bathymetry.longitude(m) ;
bathymetry.altitude = bathymetry.altitude(n,m) ;
ocean = 0.0 * bathymetry.altitude ;
waitbar(0.25);
wavefront = load_wavefront('malta_movie.nc') ;
close(hbar);

from_malta_movie = true ;
while ( true )
    malta_movie_plot
    from_malta_movie = false ;
    choice = questdlg('Restart movie?', 'Restart', 'Yes', 'No', 'Yes' ) ;
    if ( strcmp(choice,'No') ), break ; end ;
end;

