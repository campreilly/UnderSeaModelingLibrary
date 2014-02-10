function demo_topdown
    % clear all;
    close all;
    de = 20;

    global wavefront ;
    global proploss ;
%     global eigenrays ;
    prop('ProplossPropertiesOutput.nc');
    wave('WaveFrontPropertiesOutput.nc') ;
    wlat = squeeze(wavefront.latitude(:,de,:)) ;
    wlon = squeeze(wavefront.longitude(:,de,:)) ;
    walt = squeeze(wavefront.altitude(:,de,:))  ;

    global figpos;
    f = figure;
    xtarget = proploss.latitude ;
    ytarget = proploss.longitude ;
    ztarget = proploss.altitude ;
    xsource = proploss.source_latitude;
    ysource = proploss.source_longitude;
    zsource = proploss.source_altitude;
    plot3( wlat, wlon, walt, 'b-');
    hold on;
    h = plot3( xtarget, ytarget, ztarget, 'ko', ...
               xsource, ysource, zsource, 'ro');
    set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
    set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
    title(sprintf( 'LVCMS 3-D view de=%1i ', de ) );
    xlabel('Latitude')
    ylabel('longitude')
    zlabel('Depth (m)')
    grid on
    set(f,'Toolbar','figure')
    figpos = get(f,'Position');
    pos = figpos(1:2);
    pos(1) = figpos(1)*0.05;
    uicontrol( 'Style', 'edit', ...
        'Position', [pos 120 20],...
        'Callback', @edittext1_Callback );
    pos(1) = figpos(1)*0;
    uicontrol( 'Style', 'text', ...
        'Position', [pos 20 20],...        
        'String', 'DE')
    hold off;
end

function edittext1_Callback(hObject, eventdata, handles)
    global wavefront ;
    global proploss ;
%     global eigenrays ;
    global figpos ;
    new_de = str2double(get(hObject,'string'));
    if isnan(new_de)
      errordlg('You must enter a numeric value','Bad Input','modal')
      uicontrol(hObject)
        return
    end
    wlat = squeeze(wavefront.latitude(:,new_de,:)) ;
    wlon = squeeze(wavefront.longitude(:,new_de,:)) ;
    walt = squeeze(wavefront.altitude(:,new_de,:))  ;
    xtarget = proploss.latitude ;
    ytarget = proploss.longitude ;
    ztarget = proploss.altitude ;
    plot3( wlat, wlon, walt, 'b-');
    hold on;
    h = plot3( xtarget, ytarget, ztarget, 'ko', ...
               xsource, ysource, zsource, 'ro');
    set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
    set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
    title(sprintf( 'LVCMS 3-D view de=%1i ', new_de ) );
    xlabel('Latitude')
    ylabel('longitude')
    zlabel('Depth (m)')
    grid on
    pos = figpos(1:2);
    pos(1) = figpos(1)*0.05;
    uicontrol( 'Style', 'edit', ...
        'Position', [pos 120 20],...
        'Callback', @edittext1_Callback );
    pos(1) = figpos(1)*0;
    uicontrol( 'Style', 'text', ...
        'Position', [pos 20 20],...        
        'String', 'DE')
    hold off;
end

function wave(filename)
    global wavefront;
    wavefront = load_wavefront(filename);
end

function prop(filename)
    global proploss;
    global eigenrays;
    [proploss, eigenrays] = load_proploss(filename);
end