%
% malta_movie_plot - plot wavefront movie from previously loaded data
%
close all ;

% plot bottom and surface

% scrsz = get(0,'ScreenSize');
% h1 = figure('Position',[1 scrsz(4) scrsz(3) scrsz(4)], ...
%        'Name','Propagation up the Malta Escarpment',...
%        'NumberTitle','off');
figure;

colormap(copper(128));
hb = surf( bathymetry.longitude, bathymetry.latitude, bathymetry.altitude ) ;
set(hb,'EdgeColor','white') ;
set(gca,'XLim',[minlng maxlng]);
set(gca,'YLim',[minlat maxlat]);
% set(gca,'FontSize',20);
xlabel('Longitude (deg)');
ylabel('Latitude (deg)');

% set(gca,'ZLim',[-4000.0 100.0]);
set(gca,'ZLim',[-4000.0 0.0]);
view(135,80) ;
% view([50 40]);

% ho = surface( bathymetry.longitude, bathymetry.latitude, ocean ) ;
% set(ho,'FaceColor','none');
% set(ho,'EdgeColor','blue');

% plot wacvefronts

if ( from_malta_movie )
    hwait = warndlg('Ready to start movie','Ready');
    uiwait(hwait) ;
end;

tic
tindex = 3 ;
max_time = min(9999,length(wavefront.travel_time)) ;
hw = 0 ;
Ncolor = 1.0 + ceil(sqrt( ...
    double( max(max(max(wavefront.bottom))) ) + ...
    double( max(max(max(wavefront.surface))) ) ...
    ));
color = flipud(autumn(Ncolor));
while ( true )

    % plot wavefront
    
	wlng = double( squeeze( wavefront.longitude(tindex,:,:) ) );
	wlat = double( squeeze( wavefront.latitude(tindex,:,:) ) );
	walt = double( squeeze( wavefront.altitude(tindex,:,:) ) );
	wbtm = double( squeeze( wavefront.bottom(tindex,:,:) ) );
	wsrf = double( squeeze( wavefront.surface(tindex,:,:) ) );
    [N,M] = size(wbtm) ;
    wclr = zeros(N,M,3) ;
    for n=1:N
        for m=1:M
            k = 1 + floor( sqrt( double(wbtm(n,m)) + double(wsrf(n,m)) ) ) ;
            wclr(n,m,1) = color(k,1) ;
            wclr(n,m,2) = color(k,2) ;
            wclr(n,m,3) = color(k,3) ;
        end
    end
    
	hw = surface( wlng, wlat, walt, wclr ) ;
	set(hw,'FaceColor','interp');
	set(hw,'EdgeColor','black');
    title(sprintf('WOA December Average - Travel Time = %.1f',wavefront.travel_time(tindex)));
    drawnow() ;
    
    % get ready for next plot

    tindex = tindex + 3 ;
    if ( tindex > max_time ) break; end ;
    pause(0.12);
	delete(hw) ;

end
toc