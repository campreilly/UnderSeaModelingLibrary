%
% malta_rays.m - ray trace of a slice of the Malta movie
%
% Demonstrates the model's ability to visualize the wavefront in 3-D.  
% Uses the World Ocean Atlas 2009 and the ETOPO1 databases to construct a
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
% The resulting wavefronts are stored in the "malta_movie.nc" netCDF
% file for later plotting by Matlab, Octave, or other analysis routines.
%
% Plots a limited number of rays and times so that details of the
% ray paths can be clearly seen.
%
clear all ; close all
az = 270 ; 
max_time = 40.0 ; 

bathymetry = load_bathymetry('../../data/bathymetry/med_ocean.nc') ;
wavefront = load_wavefront('malta_movie.nc') ;

[p,n] = min( abs( bathymetry.latitude - 36.0 ) ) ;
blng  = bathymetry.longitude ;
balt  = bathymetry.altitude(n,:) ;

[p,m] = min( abs( wavefront.source_az - az ) ) ;
wlng = squeeze(wavefront.longitude(:,:,m)) ;
walt = squeeze(wavefront.altitude(:,:,m)) ;

N = length(wavefront.source_de) ;
q = fliplr(1:N) ;
m = [ 1 ceil(power(2.0,1:0.5:N)) ] ;
n = find( m <= N ) ; q=q(m(n));
k = find( wavefront.travel_time <= max_time ) ;

plot( blng, balt, 'k-', wlng(k,q), walt(k,q), '-'); % , wlng(k,q)', walt(k,q)', '-' ) ;
set(gca,'Xlim',[15.3 16.0]);
set(gca,'Ylim',[-4000.0 0]);
xlabel('Longitude (deg)');
ylabel('Depth (m)');
title('Malta Escarpment');
