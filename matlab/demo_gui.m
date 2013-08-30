function demo_gui
    clear all; close all;
    system_dependent(14,0) ;    % turn off the matlab error beep

%%-----------------------------Global Variables----------------------------
    global wavefront ;
    global proploss ;
    global eigenrays ;
    global f;
    global colors ;
    global l ;
    global line_color ;
    global new_az ;
    global new_de ;
    global wlat ;
    global wlon ;
    global walt ;
    global de_min ;
    global de_max ;
    global az_min ;
    global az_max ;
    global bathymetry ;
    global t_index ;
    global max_time ;
    global bathy_show ;
    global k ;
    
    colors = {'b','r','g','y','m','k','w'};

    minlat = 28.9 ; maxlat=29.1 ;
    minlng = -80.1; maxlng=-79.9 ;
    bathymetry = load_bathymetry('lvcms_bathy_test_area.nc') ;

    
%%-------------------------------GUI SETUP---------------------------------
    % Creates the default figure window
    f = figure('Units','characters',...
        'Position',[60 7 120 35],...
        'HandleVisibility','callback',...
        'IntegerHandle','off',...
        'Renderer','painters',...
        'Toolbar','figure',...
        'NumberTitle','off',...
        'Name','LVCMS 3-D Demo',...
        'ResizeFcn',@figResize);
    
    % Create the center panel
    centerPanel = uipanel('bordertype','etchedin',...
        'Units','characters',...
        'Position', [1/20 8 88 27],...
        'Parent',f);
    % Create the bottom uipanel
    botPanel = uipanel('BorderType','etchedin',...
        'Units','characters',...
        'Position',[1/20 1/20 119.9 8],...
        'Parent',f,...
        'ResizeFcn',@botPanelResize);

    % Create the right side panel
    rightPanel = uipanel('bordertype','etchedin',...
        'Units','characters',...
        'Position',[85 8 35 30],...
        'Parent',f,...
        'ResizeFcn',@rightPanelResize);

%%----------------------Resizing function calls----------------------------
    % Figure resize function
    function figResize(src,evt)
        fpos = get(f,'Position');
        set(botPanel,'Position', ...
            [1/20 1/20 fpos(3)-.1 fpos(4)*8/35])
        set(rightPanel,'Position', ...
            [fpos(3)*85/120 fpos(4)*8/35 fpos(3)*35/120 fpos(4)*27/35])
        set(centerPanel,'Position', ...
            [1/20 fpos(4)*8/35 fpos(3)*85/120 fpos(4)*27/35]);
    end

    % Bottom panel resize function
    function botPanelResize(src, evt)
        bpos = get(botPanel,'Position');
        set(wavebox,'Position',...
            [bpos(3)*15/120 bpos(4)*6/8 bpos(3)*24/120 1])
        set(wavebox_button,'Position',...
            [bpos(3)*41/120 bpos(4)*6/8 bpos(3)*15/120 1])
        set(propbox,'Position',...
            [bpos(3)*15/120 bpos(4)*4/8 bpos(3)*24/120 1])
        set(propbox_button,'Position',...
            [bpos(3)*41/120 bpos(4)*4/8 bpos(3)*15/120 1])
    end

    % Right panel resize function
    function rightPanelResize(src,evt)
        rpos = get(rightPanel,'Position');
        set(debox_label,'Position', ...
            [rpos(3)*1/32 rpos(4)*24/27 rpos(3)*18/32 rpos(4)*1/27]);
        set(debox_min,'Position', ...
            [rpos(3)*18/32 rpos(4)*24/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(debox_max,'Position', ...
            [rpos(3)*26/32 rpos(4)*24/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(azbox_label,'Position', ...
            [rpos(3)*1/32 rpos(4)*22/27 rpos(3)*18/32 rpos(4)*1/27]);
        set(azbox_min,'Position', ...
            [rpos(3)*18/32 rpos(4)*22/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(azbox_max,'Position', ...
            [rpos(3)*26/32 rpos(4)*22/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(color_line_label,'Position', ...
            [rpos(3)*3/32 rpos(4)*20/27 rpos(3)*12/32 rpos(4)*1/27]);
        set(color_line_box,'Position', ...
            [rpos(3)*18/32 rpos(4)*20/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(td_button, 'Position', ...
            [rpos(3)*4/32 rpos(4)*18/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(az_button, 'Position', ...
            [rpos(3)*18/32 rpos(4)*18/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(bathy_toggle, 'Position', ...
            [rpos(3)*18/32 rpos(4)*16/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(t_box_label, 'Position', ...
            [rpos(3)*1/32 rpos(4)*14/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(t_box, 'Position', ...
            [rpos(3)*12/32 rpos(4)*14/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(t_box_decrease, 'Position', ...
            [rpos(3)*18/32 rpos(4)*14/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_box_increase, 'Position', ...
            [rpos(3)*22/32 rpos(4)*14/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_increment, 'Position', ...
            [rpos(3)*26/32 rpos(4)*14/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_play_button, 'Position', ...
            [rpos(3)*2/32 rpos(4)*12/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(t_forward, 'Position', ...
            [rpos(3)*11/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_reverse, 'Position', ...
            [rpos(3)*15/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_state, 'Position', ...
            [rpos(3)*20/32 rpos(4)*12/27 rpos(3)*7/32 rpos(4)*1/27]);
    end

    % Setup a view buttons for specific angles of the plot
    td_button = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Top View', ...
        'Parent', rightPanel, ...
        'Callback', @TDView_Callback) ;
    az_button = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Side View', ...
        'Parent', rightPanel, ...
        'Callback', @SideView_Callback) ;

    % Setup Bathy button to show hide bathymetry
    bathy_toggle = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Show Bathy', ...
        'Parent', rightPanel, ...
        'Callback', @bathy_Callback) ;
    
    % Create the textbox and title for the DE input area
    debox_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'DE index (1:181)', ...
        'Parent', rightPanel) ;
    debox_min = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeDE_Callback_min );
    debox_max = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeDE_Callback_max );
    
    % Create the textbox and title for the AZ input area
    azbox_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'AZ index (1:25)', ...
        'Parent', rightPanel);
    azbox_min = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeAZ_Callback_min );
    azbox_max = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeAZ_Callback_max );

    % Create the textbox, buttons, and title for the time input area
    t_box_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'Time index', ...
        'Parent', rightPanel);
    t_box = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeT_Callback );
    t_box_decrease = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '-', ...
        'Parent', rightPanel, ...
        'Callback', @t_decrease_Callback );
    t_box_increase = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '+', ...
        'Parent', rightPanel, ...
        'Callback', @t_increase_Callback );
    t_increment = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'String', '1', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel ) ;
    
    % Setup the area for the time play back series
    t_play_button = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Play', ...
        'Parent', rightPanel, ...
        'Callback', @t_playback_Callback );
    t_forward = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '+', ...
        'Parent', rightPanel, ...
        'Callback', @update_state_Callback);
    t_reverse = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '-', ...
        'Parent', rightPanel, ...
        'Callback', @update_state_Callback);
    t_state = uicontrol(f, 'Style', 'text', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'state', ...
        'Parent', rightPanel) ;

    % Create a drop-down box to change the color of the plot lines
    color_line_label = uicontrol(f,'Style','text', ...
        'Units','characters', ...
        'Position',[1 1 1 1], ...
        'String','Line Color', ...
        'Parent',rightPanel);
    color_line_box = uicontrol(f,'Style','popupmenu', ...
        'Units','characters', ...
        'Position',[1 1 1 1], ...
        'BackgroundColor','white', ...
        'String',{'Blue','Red','Green','Yellow','Magenta','Black','White'},...
        'Parent',rightPanel, ...
        'Callback', @LineColorCallback);
    
    % Create a field that allows the user to specify the filename for the
    % wavefront and proploss file and then buttons to load the files
    wavebox_button = uicontrol(f, 'Style','pushbutton', ...
        'Units', 'characters', ...
        'Position', [41 6 15 1], ...
        'BackgroundColor', 'white', ...
        'String', 'Load Wavefront', ...
        'Parent', botPanel, ...
        'Callback', @wavefront_load_Callback) ;
    wavebox = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Position', [5 6 34 1], ...
        'BackgroundColor', 'white', ...
        'Parent', botPanel) ;

    propbox_button = uicontrol(f, 'Style','pushbutton', ...
        'Units', 'characters', ...
        'Position', [41 4 15 1], ...
        'BackgroundColor', 'white', ...
        'String', 'Load Proploss', ...
        'Parent', botPanel, ...
        'Callback', @proploss_load_Callback) ;
    propbox = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Position', [5 4 34 1], ...
        'BackgroundColor', 'white', ...
        'Parent', botPanel) ;
    
%%-------------------------Main Plot Area Setup----------------------------
    a = axes('parent', centerPanel);
    view(a, 3);
    set(a,'XGrid','on','YGrid','on','ZGrid','on');

    % Main plot function. Called each time that a variable is changed or
    % range is edited. Doing so will redraw the plot with the updated
    % information.
    function create_plot
        if isempty(wavefront)
           errordlg('No wavefront file loaded yet. Please load a file and try again.', ...
               'Missing Wavefront','modal') ; 
        end
        if isnan(new_az)
           uicontrol(ChangeAZ_Callback_min) ;
        end
        if isnan(new_de)
           uicontrol(ChangeDE_Callback_min) ; 
        end
        axes(a);
        xtarget = proploss.latitude ;
        ytarget = proploss.longitude ;
        ztarget = proploss.altitude ;
        xsource = proploss.source_latitude ;
        ysource = proploss.source_longitude ;
        zsource = proploss.source_altitude ;
        [az,el] = view();
        if ~isnan(t_index) & length(new_de) > 1 & ( isempty(new_az) || length(new_az) > 1 )
            if az_min > az_max
                A = squeeze(wavefront.latitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.latitude(t_index,new_de,1:az_max)) ;
                wlat = cat(2, A, B);
                A = squeeze(wavefront.longitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.longitude(t_index,new_de,1:az_max)) ;
                wlon = cat(2, A, B);
                A = squeeze(wavefront.altitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.altitude(t_index,new_de,1:az_max)) ;
                walt = cat(2, A, B);
            else
                wlat = squeeze(wavefront.latitude(t_index,new_de,new_az)) ;
                wlon = squeeze(wavefront.longitude(t_index,new_de,new_az)) ;
                walt = squeeze(wavefront.altitude(t_index,new_de,new_az)) ;
            end
            k = surf( wlon, wlat, walt ) ;
            colormap(winter(128)) ;
            if bathy_show == get(bathy_toggle,'Max')
                bathy_plot
            end
        else
            if length(new_de) > 1 & length(new_az) > 1
                uicontrol(t_box)
            else
            if az_min > az_max
                    A = squeeze(wavefront.latitude(:,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.latitude(:,new_de,1:az_max)) ;
                    wlat = cat(2, A, B);
                    A = squeeze(wavefront.longitude(:,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.longitude(:,new_de,1:az_max)) ;
                    wlon = cat(2, A, B);
                    A = squeeze(wavefront.altitude(:,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.altitude(:,new_de,1:az_max)) ;
                    walt = cat(2, A, B);
            else
                wlat = squeeze(wavefront.latitude(:,new_de,new_az)) ;
                wlon = squeeze(wavefront.longitude(:,new_de,new_az)) ;
                walt = squeeze(wavefront.altitude(:,new_de,new_az)) ;
            end
                l = plot3( wlon, wlat, walt, 'b' );
                if bathy_show == get(bathy_toggle,'Max')
                    bathy_plot
                end
            end
        end
        hold on;
        view(az,el);
        h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                   ysource, xsource, zsource, 'ro');
        set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
        set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
        set(gca,'XLim',[minlng maxlng])
        set(gca,'YLim',[minlat maxlat])
        set(gca,'ZLim',[-500 0])
        title(sprintf( 'LVCMS 3-D view' )) ; 
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        grid on
        hold off;
    end

    % Plot the bathymetry from the file
    function bathy_plot
        hold(a,'on') ;
        minlat = 28.9 ; maxlat=29.1 ;
        minlng = -80.1; maxlng=-79.9 ;

        bathymetry = load_bathymetry('lvcms_bathy_test_area.nc') ;
        n = find( bathymetry.latitude >= minlat & bathymetry.latitude <= maxlat ) ;
        m = find( bathymetry.longitude >= minlng & bathymetry.longitude <= maxlng ) ;
        bathymetry.latitude = bathymetry.latitude(n) ;
        bathymetry.longitude = bathymetry.longitude(m) ;
        bathymetry.altitude = bathymetry.altitude(n,m) ;
        colormap(copper(128));
        hb = surf( bathymetry.longitude, bathymetry.latitude, bathymetry.altitude ) ;
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        colorbar ;
        set(hb, 'FaceColor', 'interp') ; 
        hold(a,'off') ;        
    end
    
    %% Callbacks for min/max of DE
    function ChangeDE_Callback_min(hObj, eventdata, handles)
        de_min = str2double(get(hObj,'string')) ;
        if isnan(de_max)
            uicontrol(debox_max)
        else
            new_de = de_min:de_max ;
            create_plot ;
        end
    end

    function ChangeDE_Callback_max(hObj, eventdata, handles)
        de_max = str2double(get(hObj,'string')) ;
        if isnan(de_min)
            uicontrol(debox_min)
        else
            new_de = de_min:de_max ;
            create_plot ;
        end
    end

    %% Callbacks for min/max of AZ
    function ChangeAZ_Callback_min(hObj, eventdata, handles)
        az_min = str2double(get(hObj,'string')) ;
        if isnan(az_max)
            uicontrol(azbox_max)
        else
            new_az = az_min:az_max ;
            create_plot ;
        end
    end

    function ChangeAZ_Callback_max(hObj, eventdata, handles)
        az_max = str2double(get(hObj,'string')) ;
        if isnan(az_min)
            uicontrol(azbox_min)
        else
            new_az = az_min:az_max ;
            create_plot ;
        end
    end

    %% Callbacks for time values
    function ChangeT_Callback(hObj, eventdata, handles)
        time = str2double(get(hObj,'string')) ;
        if time > max_time  
            errordlg('Time must be 1 <= t <= max_time.', ...
                'Invalid Time', 'modal') ;
        elseif time < 1
            errordlg('Time must be 1 <= t <= max_time.', ...
                'Invalid Time', 'modal') ;
        else
            t_index = time;
            create_plot ;            
        end
    end

    function t_increase_Callback(src,evt)
       inc = str2double(get(t_increment,'string')) ;
       if isnan(inc)
           inc = 1 ;
       end
       t_temp = t_index + inc ;
       find_max_time ;
       if t_temp > max_time
           msgbox('Reached max_time.', 'Maximum time', 'modal') ;
       else
           t_index = t_temp ;
           set(t_box,'String',t_index) ;
           create_plot ;
       end
    end

    function t_decrease_Callback(src,evt)
       inc = str2double(get(t_increment,'string')) ;
       if isnan(inc)
           inc = 1 ;
       end
       t_temp = t_index - inc ; 
       if t_temp < 1
           msgbox('Reached minimum time.', 'Minimum time', 'modal') ;
       else
           t_index = t_temp ;
           set(t_box,'String',t_index) ;
           create_plot ;
       end
    end
    t_increase_Callback(t_box_increase,[]) ;
    t_decrease_Callback(t_box_decrease,[]) ;

    function find_max_time
        max_time = length( wavefront.travel_time ) ;
    end

    %% Buttons
    
    %---------Time Playback interface------------
    function t_playback_Callback(src,evt)
        play_button = get(src,'value') ;
        t_index = str2double(get(t_box,'string')) ;
        inc = str2double(get(t_increment,'string')) ;
        if isnan(inc)
           inc = 1 ;
        end
        if play_button == get(src,'Max')
            set(src,'String','Pause') ;
            state = get(t_state,'String');
            if strcmp(state,'Reverse')
                while(t_index > inc)
                   play_button = get(src,'Value') ;
                   if play_button == get(src,'Min')
                       break;
                   end;
                   t_index = t_index - inc ;
                   set(t_box,'String',t_index) ;
                   create_plot
                   pause(0.1)
                end
                set(src,'String','Play') ;
            elseif strcmp(state,'Forward')
                while(t_index <= max_time-inc)
                   play_button = get(src,'Value') ;
                   if play_button == get(src,'Min')
                       break;
                   end;
                   t_index = t_index + inc ;
                   set(t_box,'String',t_index) ;
                   create_plot
                   pause(0.1)
                end
                set(src,'String','Play') ;
            else
                errordlg('Please choose the play state before pressing play.', ...
                    'Playback Error','modal') ;
                set(src,'String','Play') ;
            end
        elseif play_button == get(src,'Min')
            set(src,'String','Play') ;
            set(t_box,'String',t_index) ;
        end
    end

    function update_state_Callback(src,evt)
        state = get(src,'String') ;
        if state == '-'
            set(t_state,'String','Reverse')
        elseif state == '+'
            set(t_state,'String','Forward')
        else
            set(t_state,'String','state')
        end
    end
    
    %---------View settings-----------
    function TDView_Callback(src,evt)
       view(a, [0 90]) ; 
    end

    function SideView_Callback(src,evt)
        tmp = length(new_az) ;
        if tmp ~= 1
            view(a, [90, 0])
        else
            tmp = 90.0 - wavefront.source_az(new_az,1);
            view(a, [tmp 0])
        end
    end

    % initialize the above buttons and callbacks
    TDView_Callback(td_button,[]) ;
    SideView_Callback(az_button,[]) ;
    
    % Callback for hold state toggle button
    function bathy_Callback(src,evt)
        bathy_show = get(src, 'Value') ;
        if bathy_show == get(src,'Max')
            bathy_plot ;
            set(src,'String','Hide Bathy') ;
        elseif bathy_show == get(src,'Min')
            if ~isempty(l) || ~isempty(k)
                create_plot ;
                set(src,'String','Show Bathy') ;
            else
                cla;
                colorbar off;
                set(src,'String','Show Bathy') ;
            end
        end
    end
    bathy_Callback(bathy_toggle,[]) ;

    % Callback functions for wavefronts and proploss/eigenrays
    % All variables are set to global to allow use in any function
    function wavefront_load_Callback(src,evt)
        filename = get(wavebox, 'string') ;
        if isnan(str2double(filename))
            filename = uigetfile({'*.nc','NetCDF Files (*.nc)'; ...
                '*.*', 'All Files (*.*)'},'Choose the wavefront file.') ;
        end
        hbar = waitbar(0,['Loading Wavefront...']) ;
        set(wavebox, 'String', filename);
        waitbar(4/11);
        clear wavefront;
        wavefront = load_wavefront(filename);
        if ~isempty(wavefront)
            waitbar(1.0);
            pause(0.1)
            msgbox('Wavefront loaded successfully','Load Complete','modal')
            close(hbar);
        end
    end

    function proploss_load_Callback(src,evt)
        filename = get(propbox, 'string') ;
        clear proploss ;
        clear eigenrays ;
        if isnan(str2double(filename))
            filename = uigetfile({'*.nc','NetCDF Files (*.nc)'; ...
                '*.*', 'All Files (*.*)'},'Choose the wavefront file.') ;
        end
        set(propbox, 'String', filename);
        [proploss, eigenrays] = load_proploss(filename);
        if( ~isempty(proploss) && ~isempty(eigenrays) )
            msgbox('Proploss and Eigenray information loaded successfully', ...
            'Load Complete','modal') ;
        end
    end

    % initialize the above buttons and callbacks
    wavefront_load_Callback(wavebox_button,[]) ;
    proploss_load_Callback(propbox_button,[]) ;

    %% Callback for Line Color
    function LineColorCallback(src,evt)
        line_color = get(src, 'Value');
        line_color = char(colors(line_color)) ;
        set(l, 'Color', line_color);
    end
    
 end