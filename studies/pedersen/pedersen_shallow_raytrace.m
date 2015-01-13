%
% Movie of wavefronts for pedersen_shallow_raytrace.
% Illustrates the effect of caustic on wavefront folds.
% Also illustrate impact of wavefront on shallow targets.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
% Plots range and depth in yards for easier comparison to Pederson results.
%
% clear all;
% close all;
% earth_radius=6378101.030201019;
% d2r = pi / 180 ;
% time_init = 0.4 ;
% 
% % load wavefront and convert into range/depth coordinates
% 
% wavefront = load_wavefront('pedersen_shallow_raytrace.nc') ;
% de_index = 1:1:length(wavefront.source_de) ;
% % de_index = (186:191)+1 ;
% wlat = double( wavefront.latitude(:,de_index) ) ;
% wlat = (wlat-45)*d2r * earth_radius / 1e3 ;             % range in km
% walt = double( wavefront.altitude(:,de_index) ) ;       % depth in meters
% wsrf = double( wavefront.surface(:,de_index) ) ;
% wcst = double( wavefront.caustic(:,de_index) ) ;
% 
% % define target locations
% 
% xtarget = (500:1:1000)/1e3 ;    % range in km
% ytarget = -75.0 ;               % depth in meters
% scale = [ 0 1.2 -200 0 ] ;
% 
% % loop through the wavefront displays
% 
% figure; 
% max_time = length( wavefront.travel_time ) ;
% [p,time_index]= min( abs(wavefront.travel_time-time_init) ) ;
% while (1)
%     
%     % find direct, caustic, and surface reflected paths
%     
%     ndirect = find( wcst(time_index,:) == 0 & wsrf(time_index,:) == 0 ) ;
%     ncaustic = find( wcst(time_index,:) == 1 ) ;
%     nsurf = find( wsrf(time_index,:) == 1 ) ;
%     
%     % plot wavefronts and rays
%     
%     plot( wlat, walt, 'c-', ...
%           xtarget, ytarget, 'k.' ) ;
%     hold on;
%     h = plot( wlat(time_index,ndirect), walt(time_index,ndirect), 'bo-', ...
%           wlat(time_index,nsurf), walt(time_index,nsurf), 'gs-' ) ;
%     for n = 2:length(h)
%         set(h(n),'MarkerFaceColor',get(h(n),'Color'));
%     end
%     legend(h,'direct','surface','Location','NorthEast');
%     hold off
%     grid; axis( scale ) ;
%     xlabel('Range (km)');
%     ylabel('Depth (m)');
%     title(sprintf( 'pederson shallow raytrace t(%d)=%.0f msec', ...
%         time_index, 1000*wavefront.travel_time(time_index) ));
%     drawnow;
%     
%     % allows user to move the plot forward and backward in time
%     
%     choice = menu('Action','Quit','Next','+10','Previous','-10','Save') ;
%     switch ( choice )
%         case 1
%             break ;
%         case 2
%             if ( time_index < max_time ) time_index = time_index + 1 ; end ;
%         case 3
%             if ( time_index < max_time-10 ) time_index = time_index + 10 ; end ;
%         case 4
%             if ( time_index > 1 ) time_index = time_index - 1 ; end ;
%         case 5
%             if ( time_index > 10 ) time_index = time_index - 10 ; end ;
%         case 6
%             title('');
%             print -deps pedersen_shallow_raytrace
%     end
%     scale = axis ;      % store user changes to axes
% end

function pedersen_shallow_raytrace
    
    global de ;
    global az ;
    global time ;
    global x_dim ;
    global y_dim ;
    global follow ;
    
    follow = false ;
    lat_offset = 45.0 ;

    wavefront = load_netcdf_wavefront('pedersen_shallow_raytrace.nc') ;
    proploss = load_netcdf_proploss('pedersen_shallow_proploss.nc') ;
    wave_max_time = length(wavefront.travel_time) ;
    time = 48 ;
    de = 480:1100 ;
    az = 1 ;
    t_index = 1:time ;
    
    %% Figure properties
    grey = [0.8 0.8 0.8] ;
    f = figure('units', 'pixels', ...
               'name', 'Pedersen Shallow Wavefront Propagation', ...
               'HandleVisibility','callback',...
               'IntegerHandle','off',...
               'Renderer','painters',...
               'Toolbar','figure',...
               'NumberTitle','off',...
               'MenuBar','none',...
               'resizefcn', @resize_fig) ;
    figp = get(f, 'position') ;

    center_panel = uipanel('parent', f, ...
                           'bordertype', 'etchedin', ...
                           'units', 'pixels', ...
                           'position', [0 0 figp(3)*0.8 figp(4)], ...
                           'backgroundcolor', grey) ;
                       
    right_panel = uipanel('parent', f, ...
                          'bordertype', 'etchedin', ...
                          'units', 'pixels', ...
                          'position', [figp(3)*0.8 0 figp(3)*0.2 figp(4)], ...
                          'backgroundcolor', grey, ...
                          'resizefcn', @resize_rpanel) ;
    
    function resize_fig(src, obj)
        fpos = get(f, 'position') ;
        set(center_panel, 'position', ...
            [0 0 fpos(3)*0.8 fpos(4)]) ;
        set(right_panel, 'position', ...
            [fpos(3)*0.8 0 fpos(3)*0.2 fpos(4)]) ;
    end

    function resize_rpanel(src, obj)
        rpos = get(right_panel, 'position') ;
        set(de_table, 'Position', ...
            [0 rpos(4)*0.2 rpos(3)*0.6 rpos(4)*0.8]) ;
        set(az_table, 'Position', ...
            [rpos(3)*0.6 rpos(4)*0.2 rpos(3)*0.4 rpos(4)*0.8]) ;
        set(t_box_label, 'position', ...
            [rpos(3)*0.4 rpos(4)*0.12 rpos(3)*0.4 rpos(4)*0.03]) ;
        set(t_box_decrease, 'Position', ...
            [rpos(3)*0.275 rpos(4)*0.05 rpos(3)*0.1 rpos(4)*0.04]);
        set(t_box, 'position', ...
            [rpos(3)*0.4 rpos(4)*0.05 rpos(3)*0.4 rpos(4)*0.04]) ;
        set(t_box_increase, 'Position', ...
            [rpos(3)*0.825 rpos(4)*0.05 rpos(3)*0.1 rpos(4)*0.04]);
        set(follow_toggle, 'position', ...
            [rpos(3)*0.4 rpos(4)*0.01 rpos(3)*0.4 rpos(4)*0.03]) ;
    end

    %% GUI Controls
    rpos = get(right_panel, 'position') ;
    label_temp = '<html><center>DE</center>' ;
    de_table = uitable(f, ...
        'Units', 'pixels', ...
        'Position', [0 rpos(4)*0.2 rpos(3)*0.6 rpos(4)*0.8], ...
        'BackgroundColor', [1 1 1], ...
        'ColumnName', label_temp, ...
        'Parent', right_panel, ...
        'CellSelectionCallback', @choose_de_rays) ;
    label_temp = '<html><center>AZ</center>' ;
    az_table = uitable(f, ...
        'Units', 'pixels', ...
        'Position', [rpos(3)*0.6 rpos(4)*0.2 rpos(3)*0.4 rpos(4)*0.8], ...
        'BackgroundColor', [1 1 1], ...
        'ColumnName', label_temp, ...
        'Parent', right_panel, ...
        'CellSelectionCallback', @choose_az_rays) ;

    label = sprintf('Time: %.2f', wavefront.travel_time(t_index(end))) ;
    t_box_label = uicontrol(f, 'Style', 'text', ...
        'Units','pixel', ...
        'Position', [rpos(3)*0.4 rpos(4)*0.12 rpos(3)*0.4 rpos(4)*0.03], ...        
        'String', label, ...
        'Parent', right_panel) ;
    t_box = uicontrol(f, 'Style', 'edit', ...
        'Units','pixel', ...
        'Position', [rpos(3)*0.4 rpos(4)*0.05 rpos(3)*0.4 rpos(4)*0.04], ...
        'String', num2str(time), ...
        'BackgroundColor', 'white', ...
        'Parent', right_panel, ...
        'Callback', @ChangeT_Callback ) ;
    t_box_decrease = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'pixel', ...
        'Position', [rpos(3)*0.275 rpos(4)*0.05 rpos(3)*0.1 rpos(4)*0.04], ...
        'String', '-', ...
        'Parent', right_panel, ...
        'Callback', @t_decrease_Callback ) ;
    t_box_increase = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'pixel', ...
        'Position', [rpos(3)*0.825 rpos(4)*0.05 rpos(3)*0.1 rpos(4)*0.04], ...
        'String', '+', ...
        'Parent', right_panel, ...
        'Callback', @t_increase_Callback ) ;

    follow_toggle = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'pixels', ...
        'Position', [rpos(3)*0.4 rpos(4)*0.01 rpos(3)*0.4 rpos(4)*0.03], ...
        'String', 'Follow', ...
        'Parent', right_panel, ...
        'Callback', @follow_wave) ;

    
    % function to populate the de/az tables
    function populate_ray_table
        num_de = length(wavefront.source_de) ;
        de_list = zeros(num_de,1) ;
        for i = 1:num_de
           de_list(i,1) = wavefront.source_de(i) ;
        end
        set(de_table, 'Data', de_list) ;
        num_az = length(wavefront.source_az) ;
        az_list = zeros(num_az,1) ;
        for i = 1:num_az
           az_list(i,1) = wavefront.source_az(i) ;
        end
        set(az_table, 'Data', az_list) ;
    end
    
    %callbacks for choosing rays        
    function choose_de_rays(src, evt)
        de = evt.Indices(:,1) ;
        create_plot ;
    end

    function choose_az_rays(src, evt)
        az = evt.Indices(:,1) ;
        create_plot ;
    end
    
    %% Plot area
    a = axes('parent', center_panel) ;
    view(a,[0 90]) ;
    set(a,'XGrid','on','YGrid','on') ;
    
    function create_plot
        axes(a) ;
        cla ;
        [phi,el] = view();
        if follow
            x_dim = xlim ;
            y_dim = ylim ;
        end
        if isempty(de)
            de = 1 ;
        end
        if isempty(az)
            az = 1 ;
        end
            % wavefront log
        wlat = squeeze(wavefront.latitude(az,de,t_index)) ;
        wlat = convert_range( wlat, lat_offset, true ) ;
        walt = squeeze(wavefront.altitude(az,de,t_index)) ;
            % end point
        t_lat = squeeze(wavefront.latitude(az,de,t_index(end))) ;
        t_lat = convert_range( t_lat, lat_offset, true ) ;
        t_alt = squeeze(wavefront.altitude(az,de,t_index(end))) ;
            % edges
        [n,m] = find( wavefront.on_edge(az,de,t_index(end)) == true ) ;
        e_lat = squeeze(wavefront.latitude(az,de,t_index(end))) ;
        e_lat = e_lat(n,m) ;
        e_lat = convert_range( e_lat, lat_offset, true ) ;
        e_alt = squeeze(wavefront.altitude(az,de,t_index(end))) ;
        e_alt = e_alt(n,m) ;
        
        [n,m] = find( wavefront.caustic(az,de,t_index(end-1)) ...
                    < wavefront.caustic(az,de,t_index(end)) ) ;
        c_lat = squeeze(wavefront.latitude(az,de,t_index(end))) ;
        c_lat = c_lat(n,m) ;
        c_lat = convert_range( c_lat, lat_offset, true ) ;
        c_alt = squeeze(wavefront.altitude(az,de,t_index(end))) ;
        c_alt = c_alt(n,m) ;
        
        plot( wlat', walt', 'c' ) ;
        hold on ;
        plot( t_lat, t_alt, 'b-o' ) ;
        plot( e_lat, e_alt, 'rd', ...
              'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r' ) ;
        plot( c_lat, c_alt, 'g*', ...
              'markerfacecolor', 'g', 'markeredgecolor', 'g' ) ;
        plat = squeeze(proploss.latitude) ;
        plat = convert_range( plat, lat_offset, true ) ;
        palt = squeeze(proploss.altitude) ;
        plot( plat, palt, 'ko', ...
              'MarkerFaceColor', 'k', 'markerfacecolor', 'k' ) ;
        hold off ;
        view(phi,el) ;
        if follow
            window = (x_dim(2) - x_dim(1)) / 10.0 ;
            left = min(wavefront.latitude(az,de,t_index(end))) ;
            left = convert_range( left, lat_offset, true ) ;
            right = max(wavefront.latitude(az,de,t_index(end))) ;
            right = convert_range( right, lat_offset, true ) ;
            x_dim(1) = left - window ;
            x_dim(2) = right + window ;
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
        else
            x_dim = [0 1.2] ;
            y_dim = [-200 0] ;
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
        end
        xlabel('Range (km)') ;
        ylabel('Depth (m)') ;
        grid on ;
    end

    populate_ray_table ;
    
    function ChangeT_Callback(src, evt)
        time = str2double(get(src,'string')) ;
        if time > wave_max_time  
            errordlg(sprintf('Time must be t <= %s.', num2str(wave_max_time)), ...
                'Invalid Time', 'modal') ;
        elseif time < 1
            errordlg('Time must be 0 <= t.', ...
                'Invalid Time', 'modal') ;
        else
            t_index = 1:time ;
            label = sprintf('Time: %.2f', wavefront.travel_time(t_index(end))) ;
            set(t_box_label,'string',label) ;
            create_plot ;
        end
    end

    function t_increase_Callback(src,evt)
       if time > wave_max_time
           msgbox('Reached max_time.', 'Maximum time', 'modal') ;
       else
            t_index = 1:(t_index(end)+1) ;
            label = sprintf('Time: %.2f', wavefront.travel_time(t_index(end))) ;
            set(t_box_label,'string',label) ;
            set(t_box,'string',num2str(t_index(end))) ;
            create_plot ;
       end
    end

    function t_decrease_Callback(src,evt)
       if time < 1
           msgbox('Reached minimum time.', 'Minimum time', 'modal') ;
       else
            t_index = 1:(t_index(end)-1) ;
            label = sprintf('Time: %.2f', wavefront.travel_time(t_index(end))) ;
            set(t_box_label,'string',label) ;
            set(t_box,'string',num2str(t_index(end))) ;
            create_plot ;
       end
    end

    function follow_wave(src,evt)
        v = get(src,'value') ;
        if v == get(src,'Max')
            set(src,'string','Unfollow') ;
            follow = true ;
        else
            set(src,'string','Follow') ;
            follow = false ;
        end
        create_plot ;
    end

    function R = convert_range( latitude, offset, kilo )
       if kilo
            R = ( latitude - offset ) * 1852.0 * 60 / 1e3 ; % convert to km
       else
            R = ( latitude - offset ) * 1852.0 * 60 ;  % convert to meters
       end
    end

end