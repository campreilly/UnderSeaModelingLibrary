% eigenray_lloyds - plot results of eigenray_lloyds
%
% Puts errors in travel time, source D/E, and target D/E onto a single plot
% with three subplot() panels.  This format tells a good story, but only
% "looks right" when the plot is displayed on a larger area than normal.
%
clear all ; 
close all
path_name = [
    'Direct Path           '
    'Surface Reflected Path' ] ;

%% load individual eigenrays for WaveQ3D model and analytic solution

[ plr.proploss, plr.eigenrays ] = load_proploss('eigenray_lloyds.nc');
[ analytic.proploss, analytic.eigenrays ] = load_proploss('eigenray_lloyds_analytic.nc');
latitude = plr.proploss.latitude(:,1)'-45 ;
altitude = plr.proploss.altitude(1,:) ;
blank = zeros( size(plr.proploss.latitude') ) ;
window = find( latitude >= 0.1 ) ;

%% loop through propagation paths (direct and surface)

for path = 1:2

    % find the error in time and angles for this path

    travel_time = blank ;
    source_de = blank ;
    target_az = blank ;
    for r = 1:length(altitude) ;
        for c = 1:length(latitude)
            travel_time(r,c) = abs(plr.eigenrays(c,r).travel_time(path) - analytic.eigenrays(c,r).travel_time(path)) ;
            source_de(r,c) = abs(plr.eigenrays(c,r).source_de(path) - analytic.eigenrays(c,r).source_de(path)) ;
            target_de(r,c) = abs(plr.eigenrays(c,r).target_de(path) - analytic.eigenrays(c,r).target_de(path)) ;
        end
    end
    
    % plot errors in travel time
    
    figure ;
    units=get(gcf,'units');
    set(gcf,'units','normalized','outerposition',[0 0 1 1]);
    set(gcf,'units',units);
    height = 0.275 ;
    
    subplot(3,1,1);
    h = semilogy(latitude,travel_time) ;
    a = get(gca,'Position'); a(4) = height ; set(gca,'Position',a) ;
    set(h(1),'LineStyle','-','Color','black','LineWidth',1.5);
    set(h(2),'LineStyle',':','Color','black','LineWidth',1.5);
    set(h(3),'LineStyle','-.','Color','black','LineWidth',1.5);
    set(h(4),'LineStyle','--','Color','black','LineWidth',1.5);
    grid
    set(gca,'XTickLabel',[])
    ylabel('Travel Time Errors (s)');
    set(gca,'YLim',[1e-10 1e-2])
    set(gca,'YTick',get(gca,'YTick'));
    fprintf('%s max travel time error = %e\n',...
        path_name(path,:), max(max(travel_time(:,window))));
    
    % plot errors in source D/E angle
    
    subplot(3,1,2);
    h = semilogy(latitude,source_de) ;
    a = get(gca,'Position'); a(4) = height ; set(gca,'Position',a) ;
    set(h(1),'LineStyle','-','Color','black','LineWidth',1.5);
    set(h(2),'LineStyle',':','Color','black','LineWidth',1.5);
    set(h(3),'LineStyle','-.','Color','black','LineWidth',1.5);
    set(h(4),'LineStyle','--','Color','black','LineWidth',1.5);
    set(gca,'YLim',[1e-8 1e0])
    set(gca,'YTick',get(gca,'YTick'));
    set(gca,'XTickLabel',[])
    grid
    ylabel('Source D/E Errors (deg)');
    fprintf('%s max source D/E error = %e\n',...
        path_name(path,:), max(max(source_de(:,window))));
    
    % plot errors in target D/E angle
    
    subplot(3,1,3);
    h = semilogy(latitude,target_de) ;
    a = get(gca,'Position'); a(4) = height; set(gca,'Position',a) ;
    set(h(1),'LineStyle','-','Color','black','LineWidth',1.5);
    set(h(2),'LineStyle',':','Color','black','LineWidth',1.5);
    set(h(3),'LineStyle','-.','Color','black','LineWidth',1.5);
    set(h(4),'LineStyle','--','Color','black','LineWidth',1.5);
    set(gca,'YLim',[1e-8 1e0])
    set(gca,'YTick',get(gca,'YTick'));
    grid
    xlabel('Latitude Change (deg)'); 
    ylabel('Target D/E Errors (deg)');
    legend('0 m','10 m','100 m','1000 m','Location','SouthWest');
    fprintf('%s max target D/E error = %e\n',...
        path_name(path,:), max(max(target_de(:,window))) );
    
    filename = sprintf('eigenray_lloyds_%d.eps',path-1);
    print('-deps',filename)
    
end
