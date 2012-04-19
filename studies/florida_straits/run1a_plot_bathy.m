% run1a_plot_bathy.m - Plot bathymetry for CALOPS-S Run 1A
%
% This routine overlays the area bathymetry with
% receiver location and  source track.
%
function run1a_plot_bathy( bathymetry ) 

surf(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,...
     'LineStyle','none','FaceColor','interp');
view([0 90]); colormap(coldhot);
axis([-80.1 -79.85 26 26.8]);
set(gca,'TickDir','out');
set(gca,'Clim',[-350 0]);
xlabel('Latitude (deg)');
ylabel('Longitude (deg)');
colorbar;

% add ship tracks

hold on;
plot3(-79.99054,26.0217,-250,'o',...
    'MarkerEdgeColor','yellow','MarkerFaceColor','yellow');
x = [ -79.99054 -79.88057 ] ;
y = [ 26.03545 26.73028 ] ;
z = [ -100 -100 ] ;
plot3(x,y,z,'Color','white');
hold off;
