function demo_gui
    clear all; close all;

%%-----------------------------Global Variables----------------------------
    global wavefront ;
    global proploss ;
    global eigenrays ;
    global bathymetry ;
    global default_wavefront ;
    global default_proploss ;
    global default_bathy ;
    global initialize ;
    global init_hbar ;
    global wave_max_time ;
    global eigenray_data ;
        % plots and figures
    global f ;
    global l ;
    global h ;
    global k ;
        % gui controls
    global colors ;
    global line_color ;
    global play_img ;
    global pause_img ;
    global state ;
        % plot variables
    global new_az ;
    global new_de ;
    global wlat ;
    global wlon ;
    global walt ;
    global plot_timerange ;
    global targets ;
    global tar_num ;
    global er ;
    global neighbor ;
    global dep ;
    global azi ;
    global minlat ;
    global maxlat ;
    global minlng ;
    global maxlng ;
        % plot controls
    global de_min ;
    global de_max ;
    global az_min ;
    global az_max ;
    global t_index ;
    global max_time ;
    global bathy_show ;
    global ray_show ;
    global plot_erays ;
    global freeze_axes ;
    global x_dim ;
    global y_dim ;
    global z_dim ;
    global xmin ;
    global xmax ;
    global ymin ;
    global ymax ;
    global zmin ;
    global zmax ;
    global time_check ;
    global show_mesh ;
    
    play_img = imread('play.jpg') ;
    [r,c,d] = size(play_img) ;
    r_skip = ceil(r/16) ;
    c_skip = ceil(c/16) ;
    play_img = play_img(1:r_skip:end,1:c_skip:end,:) ;
    
    pause_img = imread('pause.jpg') ;
    [r,c,d] = size(pause_img) ;
    r_skip = ceil(r/16) ;
    c_skip = ceil(c/16) ;
    pause_img = pause_img(1:r_skip:end,1:c_skip:end,:) ;

    colors = {'b','c','r','g','y','m','k','w'} ;

    % default values
    line_color = 'b' ;
    time_check = 1 ;
    default_wavefront = 'gulf_oman_eigenray_wave.nc' ;
    default_proploss = 'gulf_oman_eigenray.nc' ;
%     default_wavefront = 'eigenray_branch_pt_wave.nc' ;
%     default_proploss = 'eigenray_branch_pt.nc' ;
    default_bathy = 'gulf_of_oman.nc' ;
    initialize = true ;

    
%% ------------------------------GUI SETUP---------------------------------

    % Creates the default figure window
    f = figure('Units','characters',...
        'Position',[60 7 120 35],...
        'HandleVisibility','callback',...
        'IntegerHandle','off',...
        'Renderer','painters',...
        'Toolbar','figure',...
        'NumberTitle','off',...
        'MenuBar','none',...
        'Name','WaveQ3D Visualization',...
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
            [bpos(3)*3/120 bpos(4)*6/8 bpos(3)*35/120 1])
        set(wavebox_button,'Position',...
            [bpos(3)*41/120 bpos(4)*6/8 bpos(3)*15/120 1])
        set(propbox,'Position',...
            [bpos(3)*3/120 bpos(4)*4/8 bpos(3)*35/120 1])
        set(propbox_button,'Position',...
            [bpos(3)*41/120 bpos(4)*4/8 bpos(3)*15/120 1])
        set(bathybox,'Position',...
            [bpos(3)*3/120 bpos(4)*2/8 bpos(3)*35/120 1])
        set(bathybox_button,'Position',...
            [bpos(3)*41/120 bpos(4)*2/8 bpos(3)*15/120 1])
        set(eigenraybox,'Position', ...
            [bpos(3)*58/120 bpos(4)*1/8 bpos(3)*61/120 bpos(4)*6/8])
        ebox_pos = get(eigenraybox,'Position');
        if ebox_pos(3) > 61
            eboxn = get(eigenraybox,'Extent') ;
            ebox_pos(3) = eboxn(3) ;
            set(eigenraybox,'Position',ebox_pos) ;
        end
    end

    % Right panel resize function
    function rightPanelResize(src, evt)
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
        set(ray_toggle, 'Position', ...
            [rpos(3)*4/32 rpos(4)*16/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(bathy_toggle, 'Position', ...
            [rpos(3)*18/32 rpos(4)*16/27 rpos(3)*10/32 rpos(4)*1/27]);
    
        %-------Setup where the time controls are in the rightpanel--------
        set(t_box_label, 'Position', ...
            [rpos(3)*1/32 rpos(4)*14/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(t_max_box, 'Position', ...
            [rpos(3)*12/32 rpos(4)*14/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(time_range, 'Position', ...
            [rpos(3)*18/32 rpos(4)*14/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(t_box, 'Position', ...
            [rpos(3)*3/32 rpos(4)*12/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(t_reverse, 'Position', ...
            [rpos(3)*9/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_box_decrease, 'Position', ...
            [rpos(3)*12/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_play_button, 'Position', ...
            [rpos(3)*15/32 rpos(4)*12/27 rpos(3)*5/32 rpos(4)*1/27]);
        set(t_box_increase, 'Position', ...
            [rpos(3)*20/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_forward, 'Position', ...
            [rpos(3)*23/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        set(t_increment, 'Position', ...
            [rpos(3)*27/32 rpos(4)*12/27 rpos(3)*3/32 rpos(4)*1/27]);
        
        %-------Setup where the plot customization controls are------------
        set(freeze_plot, 'Position', ...
            [rpos(3)*4/32 rpos(4)*10/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(show_mesh_plot, 'Position', ...
            [rpos(3)*18/32 rpos(4)*10/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(x_box_label, 'Position', ...
            [rpos(3)*1/32 rpos(4)*6/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(x_min_box, 'Position', ...
            [rpos(3)*11/32 rpos(4)*6/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(x_max_box, 'Position', ...
            [rpos(3)*21/32 rpos(4)*6/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(y_box_label, 'Position', ...
            [rpos(3)*1/32 rpos(4)*8/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(y_min_box, 'Position', ...
            [rpos(3)*11/32 rpos(4)*8/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(y_max_box, 'Position', ...
            [rpos(3)*21/32 rpos(4)*8/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(z_box_label, 'Position', ...
            [rpos(3)*1/32 rpos(4)*4/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(z_min_box, 'Position', ...
            [rpos(3)*21/32 rpos(4)*4/27 rpos(3)*8/32 rpos(4)*1/27]);
        set(z_max_box, 'Position', ...
            [rpos(3)*11/32 rpos(4)*4/27 rpos(3)*8/32 rpos(4)*1/27]);

        %-------------Setup proploss/eigenray data controls----------------
        set(show_eigenrays, 'Position', ...
            [rpos(3)*4/32 rpos(4)*2/27 rpos(3)*10/32 rpos(4)*1/27]);
        set(show_neighbors, 'Position', ...
            [rpos(3)*18/32 rpos(4)*2/27 rpos(3)*10/32 rpos(4)*1/27]);
    end

    % Create a field that allows the user to specify the filename for the
    % wavefront, proploss, and bathy file with buttons to load the files
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
        'String', default_wavefront, ...
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
        'String', default_proploss, ...
        'Parent', botPanel) ;

    bathybox_button = uicontrol(f, 'Style','pushbutton', ...
        'Units', 'characters', ...
        'Position', [41 2 15 1], ...
        'BackgroundColor', 'white', ...
        'String', 'Load Bathy', ...
        'Parent', botPanel, ...
        'Callback', @bathy_load_Callback) ;
    bathybox = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Position', [5 2 34 1], ...
        'BackgroundColor', 'white', ...
        'String', default_bathy, ...
        'Parent', botPanel) ;
    
    col_labels = {'<html><center>Time<br>S</center>', ...
                        '<html><center>Source<br>DE</center>', ...
                        '<html><center>Source<br>AZ</center>', ...
                        '<html><center>Target<br>DE</center>', ...
                        '<html><center>Target<br>AZ</center>', ...
                        '<html><center>Level<br>dB</center>', ...
                        '<html><center>Freq<br>Hz</center>', ...
                        '<html><center>Phase<br>RAD</center>', ...
                        'Surface', ...
                        'Bottom', ...
                        'Caustics'} ;
    eigenraybox = uitable(f, ...
        'Units', 'characters', ...
        'BackgroundColor', [1 1 1], ...
        'FontUnits', 'normalized', ...
        'Position', [58 1 60 6], ...
        'ColumnName', col_labels, ...
        'Parent', botPanel, ...
        'CellSelectionCallback', {@ChooseErays_Callback}) ;
    
    % Create a drop-down box to change the color of the plot lines
    color_line_label = uicontrol(f,'Style','text', ...
        'Units','characters', ...
        'Position',[1 1 1 1], ...
        'String','Line Color', ...
        'Parent',rightPanel) ;
    color_line_box = uicontrol(f,'Style','popupmenu', ...
        'Units','characters', ...
        'Position',[1 1 1 1], ...
        'BackgroundColor','white', ...
        'String',{'Blue','Cyan','Red','Green','Yellow','Magenta','Black','White'},...
        'Parent',rightPanel, ...
        'Callback', @LineColorCallback) ;
    set(color_line_box,'tooltipstring','Choose a color to plot the rays in') ;

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

    % Setup Radial and 3D switch
    ray_toggle = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Radials', ...
        'Enable', 'off', ...
        'Parent', rightPanel, ...
        'Callback', @ray_Callback) ;

    % Setup Bathy button to show hide bathymetry
    bathy_toggle = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Show Bathy', ...
        'Parent', rightPanel, ...
        'Callback', @bathy_Callback) ;
    str = '<html>Show or hide the bathymetry<br>of the Gulf of Oman.' ;
    set(bathy_toggle,'tooltipstring',str) ;
    
    % Create the textbox and title for the DE input area
    debox_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'DE index', ...
        'Parent', rightPanel) ;
    set(debox_label,'tooltipstring','Depression/Elevation angles') ;
    debox_min = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'String', 'min', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeDE_Callback) ;
    set(debox_min,'tooltipstring','Minimum de angle') ;
    debox_max = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'String', 'max', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeDE_Callback) ;
    set(debox_max,'tooltipstring','Maximum de angle') ;
    
    % Create the textbox and title for the AZ input area
    azbox_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'AZ index', ...
        'Parent', rightPanel) ;
    set(azbox_label,'tooltipstring','Azimuthal angles') ;
    azbox_min = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'String', 'min', ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeAZ_Callback) ;
    set(azbox_min,'tooltipstring','Minimum az angle') ;
    azbox_max = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'String', 'max', ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeAZ_Callback) ;
    set(azbox_max,'tooltipstring','Maximum az angle') ;

    % Create the textbox, buttons, and title for the time input area
    t_box_label = uicontrol(f, 'Style', 'text', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...        
        'String', 'Time', ...
        'Parent', rightPanel) ;
    set(t_box_label,'tooltipstring','Time indecies for the wavefront file') ;
    t_max_box = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeT_max_Callback) ;
    set(t_max_box,'tooltipstring','Set the maximum run time') ;
    time_range = uicontrol(f,'Style','popupmenu', ...
        'Units','characters', ...
        'Position',[1 1 1 1], ...
        'BackgroundColor','white', ...
        'String',{'All time','Start:Current','Current:End'},...
        'Parent',rightPanel, ...
        'Callback', @SetTimeRange_Callback) ;

    % Setup the area for the time play back series
    t_box = uicontrol(f, 'Style', 'edit', ...
        'Units','characters', ...
        'Position', [1 1 1 1], ...
        'String', 't', ...
        'BackgroundColor', 'white', ...
        'Parent', rightPanel, ...
        'Callback', @ChangeT_Callback ) ;
    set(t_box,'tooltipstring','Current time') ;
    t_reverse = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '<<', ...
        'Parent', rightPanel, ...
        'Callback', @update_state_Callback) ;
    set(t_reverse,'tooltipstring','Play backwards in time') ;
    t_box_decrease = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '-', ...
        'Parent', rightPanel, ...
        'Callback', @t_decrease_Callback ) ;
    set(t_box_decrease,'tooltipstring','Take a step back in time') ;
    t_play_button = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'CData', play_img, ...
        'Parent', rightPanel, ...
        'Callback', @t_playback_Callback ) ;
    set(t_play_button,'tooltipstring','Animate wavefront') ;
    t_box_increase = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '+', ...
        'Parent', rightPanel, ...
        'Callback', @t_increase_Callback ) ;
    set(t_box_increase,'tooltipstring','Take a step forward in time') ;
    t_forward = uicontrol(f, 'Style', 'pushbutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', '>>', ...
        'Parent', rightPanel, ...
        'Callback', @update_state_Callback) ;
    set(t_forward,'tooltipstring','Play forwards in time') ;
    t_increment = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'String', '1', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel ) ;
    set(t_increment,'tooltipstring','Set time step') ;

    % Plot customization options
    freeze_plot = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'String', 'Freeze', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @freeze_plot_Callback) ;
    set(freeze_plot,'tooltipstring','Freeze plot axes') ;
    show_mesh_plot = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'String', 'Mesh', ...
        'Enable', 'off', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @MeshPlot_Callback) ;
    set(show_mesh_plot,'tooltipstring','Switch 3D/Mesh') ;
    y_box_label = uicontrol(f, 'Style', 'text', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Latitude', ...
        'Parent', rightPanel) ;
    set(y_box_label,'tooltipstring','Set the latitude axis min/max') ;
    y_min_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeY_Callback) ;
    set(y_min_box,'tooltipstring','Minimum latitude') ;
    y_max_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeY_Callback) ;
    set(y_max_box,'tooltipstring','Maximum latitude') ;
    x_box_label = uicontrol(f, 'Style', 'text', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Longitude', ...
        'Parent', rightPanel) ;
    set(x_box_label,'tooltipstring','Set the longitude axis min/max') ;
    x_min_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeX_Callback) ;
    set(x_min_box,'tooltipstring','Minimum longitude') ;
    x_max_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeX_Callback) ;
    set(x_max_box,'tooltipstring','Maximum longitude') ;
    z_box_label = uicontrol(f, 'Style', 'text', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Depth', ...
        'Parent', rightPanel) ;
    set(z_box_label,'tooltipstring','Set the depth axis min/max') ;
    z_max_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeZ_Callback) ;
    set(z_max_box,'tooltipstring','Maximum depth') ;
    z_min_box = uicontrol(f, 'Style', 'edit', ...
        'Units', 'characters', ...
        'Background', 'white', ...
        'Position', [1 1 1 1], ...
        'Parent', rightPanel, ...
        'Callback', @ChangeZ_Callback) ;
    set(z_min_box,'tooltipstring','Minimum depth') ;
    
    % Proploss/Eigenray Controls
    show_eigenrays = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Eigenrays', ...
        'Enable', 'off', ...
        'Parent', rightPanel, ...
        'Callback', @show_eigenrays_Callback ) ;
    show_neighbors = uicontrol(f, 'Style', 'togglebutton', ...
        'Units', 'characters', ...
        'Position', [1 1 1 1], ...
        'String', 'Neighbors', ...
        'Enable', 'off', ...
        'Parent', rightPanel, ...
        'Callback', @show_neighbors_Callback ) ;
    
%% ------------------------Main Plot Area Setup----------------------------
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
           uicontrol(ChangeAZ_Callback) ;
        end
        if isnan(new_de)
           uicontrol(ChangeDE_Callback) ; 
        end
        if isnan(t_index)
           uicontrol(ChangeT_Callback) ; 
        end
        axes(a);
        cla ;
        [az,el] = view();
        if ~isnan(t_index) & ( isempty(new_de) || length(new_de) > 1 ) & ( isempty(new_az) || length(new_az) > 1 )
            if (az_min > az_max) & (de_min > de_max)
                a_top = az_min:length(wavefront.source_az) ;       % rays for az_min:length(wavefront.source_az)
                a_bot = 1:az_max ;                                 % rays for 1:az_max
                d_top = de_min:length(wavefront.source_de) ;       % rays for de_min:length(wavefront.source_de)
                d_bot = 1:de_max ;                                 % rays for 1:de_max
                    % top-top quadrent
                lat_tt = squeeze(wavefront.latitude(t_index,d_top,a_top));
                lon_tt = squeeze(wavefront.longitude(t_index,d_top,a_top));
                alt_tt = squeeze(wavefront.altitude(t_index,d_top,a_top));
                    % top-bot quadrent
                lat_tb = squeeze(wavefront.latitude(t_index,d_top,a_bot));
                lon_tb = squeeze(wavefront.longitude(t_index,d_top,a_bot));
                alt_tb = squeeze(wavefront.altitude(t_index,d_top,a_bot));
                    % bot-top quadrent
                lat_bt = squeeze(wavefront.latitude(t_index,d_bot,a_top));
                lon_bt = squeeze(wavefront.longitude(t_index,d_bot,a_top));
                alt_bt = squeeze(wavefront.altitude(t_index,d_bot,a_top));
                    % bot-bot quadrent
                lat_bb = squeeze(wavefront.latitude(t_index,d_bot,a_bot));
                lon_bb = squeeze(wavefront.longitude(t_index,d_bot,a_bot));
                alt_bb = squeeze(wavefront.altitude(t_index,d_bot,a_bot));
                k = surf( lon_tt, lat_tt, alt_tt, 'FaceColor', 'interp' ) ;
                hold on ;
                surf( lon_tb, lat_tb, alt_tb, 'FaceColor', 'interp' ) ;
                surf( lon_bt, lat_bt, alt_bt, 'FaceColor', 'interp' ) ;
                surf( lon_bb, lat_bb, alt_bb, 'FaceColor', 'interp' ) ;
                hold off ;
            elseif az_min > az_max
                A = squeeze(wavefront.latitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.latitude(t_index,new_de,1:az_max)) ;
                wlat = cat(2, A, B);
                A = squeeze(wavefront.longitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.longitude(t_index,new_de,1:az_max)) ;
                wlon = cat(2, A, B);
                A = squeeze(wavefront.altitude(t_index,new_de,az_min:length(wavefront.source_az)));
                B = squeeze(wavefront.altitude(t_index,new_de,1:az_max)) ;
                walt = cat(2, A, B);
                k = surf( wlon, wlat, walt, 'FaceColor', 'interp' ) ;
            elseif de_min > de_max
                lat_A = squeeze(wavefront.latitude(t_index,de_min:length(wavefront.source_de),new_az));
                lat_B = squeeze(wavefront.latitude(t_index,1:de_max,new_az)) ;
                lon_A = squeeze(wavefront.longitude(t_index,de_min:length(wavefront.source_de),new_az));
                lon_B = squeeze(wavefront.longitude(t_index,1:de_max,new_az)) ;
                alt_A = squeeze(wavefront.altitude(t_index,de_min:length(wavefront.source_de),new_az));
                alt_B = squeeze(wavefront.altitude(t_index,1:de_max,new_az)) ;
                k = surf( lon_A, lat_A, alt_A, 'FaceColor', 'interp' ) ;
                hold on ;
                surf( lon_B, lat_B, alt_B, 'FaceColor', 'interp' ) ;
                hold off ;
            else
                wlat = squeeze(wavefront.latitude(t_index,new_de,new_az)) ;
                wlon = squeeze(wavefront.longitude(t_index,new_de,new_az)) ;
                walt = squeeze(wavefront.altitude(t_index,new_de,new_az)) ;
                k = surf( wlon, wlat, walt, 'FaceColor', 'interp' ) ;
            end
            colormap(winter(128)) ;
            if bathy_show == get(bathy_toggle,'Max')
                bathy_plot
            end
            gen_wave_labels(k) ;
            set(show_mesh_plot,'Enable','on') ;
            set(ray_toggle,'Enable','on') ;
            set(time_range,'Enable','off') ;
        else
            if length(new_de) > 1 && length(new_az) > 1
                uicontrol(t_box)
            elseif plot_erays == get(show_eigenrays,'Max')
                eigenray_plot ;
            else
                SetTimeRange(time_check) ;
                if az_min > az_max
                    A = squeeze(wavefront.latitude(plot_timerange,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.latitude(plot_timerange,new_de,1:az_max)) ;
                    wlat = cat(2, A, B);
                    A = squeeze(wavefront.longitude(plot_timerange,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.longitude(plot_timerange,new_de,1:az_max)) ;
                    wlon = cat(2, A, B);
                    A = squeeze(wavefront.altitude(plot_timerange,new_de,az_min:length(wavefront.source_az)));
                    B = squeeze(wavefront.altitude(plot_timerange,new_de,1:az_max)) ;
                    walt = cat(2, A, B);
                    l = plot3( wlon, wlat, walt, line_color );
                    hold on;
                    if ~isnan(t_index)
                        A = squeeze(wavefront.latitude(t_index,new_de,az_min:length(wavefront.source_az)));
                        B = squeeze(wavefront.latitude(t_index,new_de,1:az_max)) ;
                        t_lat = cat(1, A, B);
                        A = squeeze(wavefront.longitude(t_index,new_de,az_min:length(wavefront.source_az)));
                        B = squeeze(wavefront.longitude(t_index,new_de,1:az_max)) ;
                        t_lon = cat(1, A, B);
                        A = squeeze(wavefront.altitude(t_index,new_de,az_min:length(wavefront.source_az)));
                        B = squeeze(wavefront.altitude(t_index,new_de,1:az_max)) ;
                        t_alt = cat(1, A, B);
                        plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                    end
                    hold off;
                elseif de_min > de_max
                    lat_A = squeeze(wavefront.latitude(plot_timerange,de_min:length(wavefront.source_de),new_az));
                    lat_B = squeeze(wavefront.latitude(plot_timerange,1:de_max,new_az)) ;
                    lon_A = squeeze(wavefront.longitude(plot_timerange,de_min:length(wavefront.source_de),new_az));
                    lon_B = squeeze(wavefront.longitude(plot_timerange,1:de_max,new_az)) ;
                    alt_A = squeeze(wavefront.altitude(plot_timerange,de_min:length(wavefront.source_de),new_az));
                    alt_B = squeeze(wavefront.altitude(plot_timerange,1:de_max,new_az)) ;
                    l = plot3( lon_A, lat_A, alt_A, line_color, ...
                               lon_B, lat_B, alt_B, line_color );
                    hold on;
                    if ~isnan(t_index)
                        t_lat_A = squeeze(wavefront.latitude(t_index,de_min:length(wavefront.source_de),new_az));
                        t_lat_B = squeeze(wavefront.latitude(t_index,1:de_max,new_az)) ;
                        t_lon_A = squeeze(wavefront.longitude(t_index,de_min:length(wavefront.source_de),new_az));
                        t_lon_B = squeeze(wavefront.longitude(t_index,1:de_max,new_az)) ;
                        t_alt_A = squeeze(wavefront.altitude(t_index,de_min:length(wavefront.source_de),new_az));
                        t_alt_B = squeeze(wavefront.altitude(t_index,1:de_max,new_az)) ;
                        plot3( t_lon_A, t_lat_A, t_alt_A, 'ro-', t_lon_B, t_lat_B, t_alt_B, 'ro-' ) ;
                    end
                    hold off;
                else
                    wlat = squeeze(wavefront.latitude(plot_timerange,new_de,new_az)) ;
                    wlon = squeeze(wavefront.longitude(plot_timerange,new_de,new_az)) ;
                    walt = squeeze(wavefront.altitude(plot_timerange,new_de,new_az)) ;
                    l = plot3( wlon, wlat, walt, line_color ) ;
                    hold on;
                    if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,new_de,new_az)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,new_de,new_az)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,new_de,new_az)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                    end
                    hold off;
                end
                if bathy_show == get(bathy_toggle,'Max')
                    bathy_plot
                end
            end
%             gen_wave_labels(l,(1:max_time)) ;
            set(show_mesh_plot,'Enable','off') ;
            set(ray_toggle,'Enable','off') ;
            set(time_range,'Enable','on') ;
        end
        hold on;
        view(az,el);
        xtarget = proploss.latitude ;
        ytarget = proploss.longitude ;
        ztarget = proploss.altitude ;
        xsource = proploss.source_latitude ;
        ysource = proploss.source_longitude ;
        zsource = proploss.source_altitude ;
        h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                   ysource, xsource, zsource, 'ro');
        set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
        set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
        title(sprintf( 'WaveQ3D Visualization' )) ;
        gen_proploss_labels(h(1)) ;
        gen_proploss_labels(h(2)) ;
        if freeze_axes
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
            set(a,'ZLim',z_dim) ;
        end
        update_axis_vals
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        grid on
        set(gca,'color','none')
        hold off ;
        set(show_eigenrays,'Enable','on') ;
    end

    % Plot the bathymetry from the file
    function bathy_plot
        hold(a,'on') ;
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
        if freeze_axes
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
            set(a,'ZLim',z_dim) ;
        end
        colorbar ;
        set(hb, 'FaceColor', 'interp') ;
        set(gca,'color','none')
        hold(a,'off') ;        
    end

    % Plot the radials instead of 3-D surface
    function ray_plot
        cla ;
        [az,el] = view() ;
        hold on ;
        set(time_range,'Enable','on') ;
        SetTimeRange(time_check) ;
        if (az_min > az_max) & (de_min > de_max)
            a_top = az_min:length(wavefront.source_az) ;       % rays for az_min:length(wavefront.source_az)
            a_bot = 1:az_max ;                                 % rays for 1:az_max
            d_top = de_min:length(wavefront.source_de) ;       % rays for de_min:length(wavefront.source_de)
            d_bot = 1:de_max ;                                 % rays for 1:de_max
            t_lat = squeeze(wavefront.latitude(t_index,d_top,a_top));
            t_lon = squeeze(wavefront.longitude(t_index,d_top,a_top));
            t_alt = squeeze(wavefront.altitude(t_index,d_top,a_top));
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_top
                wlat = squeeze(wavefront.latitude(plot_timerange,d_top,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_top,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_top,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_top,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_top,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_top,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
            t_lat = squeeze(wavefront.latitude(t_index,d_top,a_bot));
            t_lon = squeeze(wavefront.longitude(t_index,d_top,a_bot));
            t_alt = squeeze(wavefront.altitude(t_index,d_top,a_bot));
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_bot
                wlat = squeeze(wavefront.latitude(plot_timerange,d_top,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_top,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_top,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_top,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_top,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_top,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
            t_lat = squeeze(wavefront.latitude(t_index,d_bot,a_top));
            t_lon = squeeze(wavefront.longitude(t_index,d_bot,a_top));
            t_alt = squeeze(wavefront.altitude(t_index,d_bot,a_top));
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_top
                wlat = squeeze(wavefront.latitude(plot_timerange,d_bot,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_bot,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_bot,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_bot,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_bot,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_bot,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
            t_lat = squeeze(wavefront.latitude(t_index,d_bot,a_bot));
            t_lon = squeeze(wavefront.longitude(t_index,d_bot,a_bot));
            t_alt = squeeze(wavefront.altitude(t_index,d_bot,a_bot));
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_bot
                wlat = squeeze(wavefront.latitude(plot_timerange,d_bot,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_bot,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_bot,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_bot,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_bot,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_bot,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
        elseif isempty(new_az)                                             % az wraps around
            a_top = az_min:length(wavefront.source_az) ;       % rays for az_min:length(wavefront.source_az)
            a_bot = 1:az_max ;                                 % rays for 1:az_max
            t_lat = squeeze(wavefront.latitude(t_index,new_de,a_top)) ;
            t_lon = squeeze(wavefront.longitude(t_index,new_de,a_top)) ;
            t_alt = squeeze(wavefront.altitude(t_index,new_de,a_top)) ;
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_top
                wlat = squeeze(wavefront.latitude(plot_timerange,new_de,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,new_de,i));
                walt = squeeze(wavefront.altitude(plot_timerange,new_de,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,new_de,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,new_de,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,new_de,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
            t_lat = squeeze(wavefront.latitude(t_index,new_de,a_bot)) ;
            t_lon = squeeze(wavefront.longitude(t_index,new_de,a_bot)) ;
            t_alt = squeeze(wavefront.altitude(t_index,new_de,a_bot)) ;
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = a_bot
                wlat = squeeze(wavefront.latitude(plot_timerange,new_de,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,new_de,i));
                walt = squeeze(wavefront.altitude(plot_timerange,new_de,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,new_de,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,new_de,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,new_de,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
        elseif isempty(new_de)                                             % de wrap around
            d_top = de_min:length(wavefront.source_de) ;       % rays for de_min:length(wavefront.source_de)
            d_bot = 1:de_max ;                                 % rays for 1:de_max
            t_lat = squeeze(wavefront.latitude(t_index,d_top,new_az)) ;
            t_lon = squeeze(wavefront.longitude(t_index,d_top,new_az)) ;
            t_alt = squeeze(wavefront.altitude(t_index,d_top,new_az)) ;
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = new_az
                wlat = squeeze(wavefront.latitude(plot_timerange,d_top,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_top,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_top,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_top,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_top,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_top,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
            t_lat = squeeze(wavefront.latitude(t_index,d_bot,new_az)) ;
            t_lon = squeeze(wavefront.longitude(t_index,d_bot,new_az)) ;
            t_alt = squeeze(wavefront.altitude(t_index,d_bot,new_az)) ;
            mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
            for i = new_az
                wlat = squeeze(wavefront.latitude(plot_timerange,d_bot,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,d_bot,i));
                walt = squeeze(wavefront.altitude(plot_timerange,d_bot,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
                if ~isnan(t_index)
                       t_lat = squeeze(wavefront.latitude(t_index,d_bot,i)) ;
                       t_lon = squeeze(wavefront.longitude(t_index,d_bot,i)) ;
                       t_alt = squeeze(wavefront.altitude(t_index,d_bot,i)) ;
                       plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end
        else                                                               % neither de or az wraps
            for i = new_az
                wlat = squeeze(wavefront.latitude(plot_timerange,new_de,i));
                wlon = squeeze(wavefront.longitude(plot_timerange,new_de,i));
                walt = squeeze(wavefront.altitude(plot_timerange,new_de,i));
                l = plot3( wlon, wlat, walt, line_color ) ;
            end
            if ~isnan(t_index)
                t_lat = squeeze(wavefront.latitude(t_index,new_de,new_az)) ;
                t_lon = squeeze(wavefront.longitude(t_index,new_de,new_az)) ;
                t_alt = squeeze(wavefront.altitude(t_index,new_de,new_az)) ;
                mesh( t_lon, t_lat, t_alt, 'EdgeColor', 'red', 'FaceColor', 'none' ) ;
                for i = new_az
                   t_lat = squeeze(wavefront.latitude(t_index,new_de,i)) ;
                   t_lon = squeeze(wavefront.longitude(t_index,new_de,i)) ;
                   t_alt = squeeze(wavefront.altitude(t_index,new_de,i)) ;
                   plot3( t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end            
        end
        xtarget = proploss.latitude ;
        ytarget = proploss.longitude ;
        ztarget = proploss.altitude ;
        xsource = proploss.source_latitude ;
        ysource = proploss.source_longitude ;
        zsource = proploss.source_altitude ;
        h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                   ysource, xsource, zsource, 'ro');
        set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
        set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
        gen_proploss_labels(h(1)) ;
        gen_proploss_labels(h(2)) ;
        view(az,el);
        title(sprintf( 'WaveQ3D Visualization' )) ; 
        if freeze_axes
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
            set(a,'ZLim',z_dim) ;
        end
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        grid on
        set(gca,'color','none')
        hold off;
        if bathy_show == get(bathy_toggle,'Max')
            bathy_plot
        else
            colorbar off ;
        end
    end

    % Show a plot of 3D mesh instead of the surface plot
    function mesh_plot
        cla ;
        [az,el] = view();
        hold on ;
        set(time_range,'Enable','off') ;
        pstr = strcat(line_color,'o-') ;
        if (az_min > az_max) & (de_min > de_max)
            a_top = az_min:length(wavefront.source_az) ;       % rays for az_min:length(wavefront.source_az)
            a_bot = 1:az_max ;                                 % rays for 1:az_max
            d_top = de_min:length(wavefront.source_de) ;       % rays for de_min:length(wavefront.source_de)
            d_bot = 1:de_max ;                                 % rays for 1:de_max
                % top-top quadrent
            lat_tt = squeeze(wavefront.latitude(t_index,d_top,a_top));
            lon_tt = squeeze(wavefront.longitude(t_index,d_top,a_top));
            alt_tt = squeeze(wavefront.altitude(t_index,d_top,a_top));
                % top-bot quadrent
            lat_tb = squeeze(wavefront.latitude(t_index,d_top,a_bot));
            lon_tb = squeeze(wavefront.longitude(t_index,d_top,a_bot));
            alt_tb = squeeze(wavefront.altitude(t_index,d_top,a_bot));
                % bot-top quadrent
            lat_bt = squeeze(wavefront.latitude(t_index,d_bot,a_top));
            lon_bt = squeeze(wavefront.longitude(t_index,d_bot,a_top));
            alt_bt = squeeze(wavefront.altitude(t_index,d_bot,a_top));
                % bot-bot quadrent
            lat_bb = squeeze(wavefront.latitude(t_index,d_bot,a_bot));
            lon_bb = squeeze(wavefront.longitude(t_index,d_bot,a_bot));
            alt_bb = squeeze(wavefront.altitude(t_index,d_bot,a_bot));
            k = mesh( lon_tt, lat_tt, alt_tt, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            mesh( lon_tb, lat_tb, alt_tb, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            mesh( lon_bt, lat_bt, alt_bt, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            mesh( lon_bb, lat_bb, alt_bb, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            plot3( lon_tt, lat_tt, alt_tt, pstr ) ;
            plot3( lon_tb, lat_tb, alt_tb, pstr ) ;
            plot3( lon_bt, lat_bt, alt_bt, pstr ) ;
            plot3( lon_bb, lat_bb, alt_bb, pstr ) ;
        elseif az_min > az_max
            A = squeeze(wavefront.latitude(t_index,new_de,az_min:length(wavefront.source_az)));
            B = squeeze(wavefront.latitude(t_index,new_de,1:az_max)) ;
            wlat = cat(2, A, B);
            A = squeeze(wavefront.longitude(t_index,new_de,az_min:length(wavefront.source_az)));
            B = squeeze(wavefront.longitude(t_index,new_de,1:az_max)) ;
            wlon = cat(2, A, B);
            A = squeeze(wavefront.altitude(t_index,new_de,az_min:length(wavefront.source_az)));
            B = squeeze(wavefront.altitude(t_index,new_de,1:az_max)) ;
            walt = cat(2, A, B);
            k = mesh( wlon, wlat, walt, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            plot3( wlon, wlat, walt, pstr ) ;
        elseif de_min > de_max
            lat_A = squeeze(wavefront.latitude(t_index,de_min:length(wavefront.source_de),new_az));
            lat_B = squeeze(wavefront.latitude(t_index,1:de_max,new_az)) ;
            lon_A = squeeze(wavefront.longitude(t_index,de_min:length(wavefront.source_de),new_az));
            lon_B = squeeze(wavefront.longitude(t_index,1:de_max,new_az)) ;
            alt_A = squeeze(wavefront.altitude(t_index,de_min:length(wavefront.source_de),new_az));
            alt_B = squeeze(wavefront.altitude(t_index,1:de_max,new_az)) ;
            k = mesh( lon_A, lat_A, alt_A, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            mesh( lon_B, lat_B, alt_B, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            plot3( lon_A, lat_A, alt_A, pstr ) ;
            plot3( lon_B, lat_B, alt_B, pstr ) ;
        else
            wlat = squeeze(wavefront.latitude(t_index,new_de,new_az)) ;
            wlon = squeeze(wavefront.longitude(t_index,new_de,new_az)) ;
            walt = squeeze(wavefront.altitude(t_index,new_de,new_az)) ;
            k = mesh( wlon, wlat, walt, 'EdgeColor', line_color, 'FaceColor', 'none' ) ;
            plot3( wlon, wlat, walt, pstr ) ;
        end
        gen_wave_labels(k) ; 
        xtarget = proploss.latitude ;
        ytarget = proploss.longitude ;
        ztarget = proploss.altitude ;
        xsource = proploss.source_latitude ;
        ysource = proploss.source_longitude ;
        zsource = proploss.source_altitude ;
        h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                   ysource, xsource, zsource, 'ro');
        set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
        set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
        gen_proploss_labels(h(1)) ;
        gen_proploss_labels(h(2)) ;
        view(az,el);
        title(sprintf( 'WaveQ3D Visualization' )) ; 
        if freeze_axes
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
            set(a,'ZLim',z_dim) ;
        end
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        grid on
        set(gca,'color','none')
        hold off;
        if bathy_show == get(bathy_toggle,'Max')
            bathy_plot
        else
            colorbar off ;
        end
    end

    % Plot only the eigenrays for the selected target
    function eigenray_plot
        if ~isempty(tar_num)
            cla ;
            [az,el] = view() ;
            hold on ;
            set(time_range,'Enable','on') ;
            SetTimeRange(time_check) ;
            xtarget = proploss.latitude(tar_num) ;
            ytarget = proploss.longitude(tar_num) ;
            ztarget = proploss.altitude(tar_num) ;
            xsource = proploss.source_latitude ;
            ysource = proploss.source_longitude ;
            zsource = proploss.source_altitude ;

            wlat = squeeze(wavefront.latitude) ;
            wlon = squeeze(wavefront.longitude) ;
            walt = squeeze(wavefront.altitude) ;

            if isempty(er)
                num_erays = length(eigenrays(tar_num,1).source_de) ;
                for i = 1:num_erays
                    for j = 1:(length(wavefront.source_de)-1)
                        if eigenrays(tar_num,1).source_de(i) >= wavefront.source_de(j) && ...
                            eigenrays(tar_num,1).source_de(i) <= wavefront.source_de(j+1)
                            if abs(abs((eigenrays(tar_num,1).source_de(i)) - abs(wavefront.source_de(j)))) ...
                                    < abs(abs((eigenrays(tar_num,1).source_de(i)) - abs(wavefront.source_de(j+1))))
                                n(i) = j ;
                            else
                                n(i) = j+1 ;
                            end
                        end
                    end
                    for j = 1:(length(wavefront.source_az)-1)
                        if eigenrays(tar_num,1).source_az(i) < 0
                           eray_az = eigenrays(tar_num,1).source_az(i) + 360 ;
                        else
                           eray_az = eigenrays(tar_num,1).source_az(i) ;
                        end
                        if eray_az >= wavefront.source_az(j) && eray_az <= wavefront.source_az(j+1)
                            if abs(eray_az - abs(wavefront.source_az(j))) ...
                                    < abs(eray_az - abs(wavefront.source_az(j+1)))
                                m(i) = j ;
                            else
                                m(i) = j+1 ;
                            end
                        end
                    end
                end
                for i = 1:num_erays
                    lat = wlat(plot_timerange,n(i),m(i)) ; 
                    lon = wlon(plot_timerange,n(i),m(i)) ; 
                    alt = walt(plot_timerange,n(i),m(i)) ;
                    t_lat = wlat(t_index,n(i),m(i)) ;
                    t_lon = wlon(t_index,n(i),m(i)) ;
                    t_alt = walt(t_index,n(i),m(i)) ;
                    plot3( lon, lat, alt, line_color, ...
                        t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            else
                for i = 1:length(er)
                    for j = 1:(length(wavefront.source_de)-1)
                        if eigenrays(tar_num,1).source_de(er(i)) >= wavefront.source_de(j) && ...
                            eigenrays(tar_num,1).source_de(er(i)) <= wavefront.source_de(j+1)
                            if abs(abs((eigenrays(tar_num,1).source_de(er(i))) - abs(wavefront.source_de(j)))) ...
                                    < abs(abs((eigenrays(tar_num,1).source_de(er(i))) - abs(wavefront.source_de(j+1))))
                                n(i) = j ;
                            else
                                n(i) = j+1 ;
                            end
                        end
                    end
                    for j = 1:(length(wavefront.source_az)-1)
                        if eigenrays(tar_num,1).source_az(er(i)) < 0
                           eray_az = eigenrays(tar_num,1).source_az(er(i)) + 360 ;
                        else
                           eray_az = eigenrays(tar_num,1).source_az(er(i)) ;
                        end
                        if eray_az >= wavefront.source_az(j) && eray_az <= wavefront.source_az(j+1)
                            if abs(eray_az - abs(wavefront.source_az(j))) ...
                                    < abs(eray_az - abs(wavefront.source_az(j+1)))
                                m(i) = j ;
                            else
                                m(i) = j+1 ;
                            end
                        end
                    end
                end
                for i = 1:length(er)
                    lat = wlat(plot_timerange,n(i),m(i)) ; 
                    lon = wlon(plot_timerange,n(i),m(i)) ; 
                    alt = walt(plot_timerange,n(i),m(i)) ;
                    t_lat = wlat(t_index,n(i),m(i)) ;
                    t_lon = wlon(t_index,n(i),m(i)) ;
                    t_alt = walt(t_index,n(i),m(i)) ;
                    plot3( lon, lat, alt, line_color, ...
                        t_lon, t_lat, t_alt, 'ro-' ) ;
                end
            end

            h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                       ysource, xsource, zsource, 'ro');
            set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
            set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
            gen_proploss_labels(h(1)) ;
            gen_proploss_labels(h(2)) ;
            view(az,el);
            title(sprintf( 'WaveQ3D Visualization' )) ; 
            if freeze_axes
                set(a,'XLim',x_dim) ;
                set(a,'YLim',y_dim) ;
                set(a,'ZLim',z_dim) ;
            end
            xlabel('Longitude')
            ylabel('Latitude')
            zlabel('Depth (m)')
            grid on
            set(gca,'color','none')
            hold off;
            if bathy_show == get(bathy_toggle,'Max')
                bathy_plot
            else
                colorbar off ;
            end
        end
    end

    % Show the neighbors to the CPA in the eigenray_plot
    function neighbor_plot
        cla ;
        [az,el] = view() ;
        hold on ;
        set(time_range,'Enable','on') ;
        SetTimeRange(time_check) ;
        xtarget = proploss.latitude(tar_num) ;
        ytarget = proploss.longitude(tar_num) ;
        ztarget = proploss.altitude(tar_num) ;
        xsource = proploss.source_latitude ;
        ysource = proploss.source_longitude ;
        zsource = proploss.source_altitude ;
        
        wlat = squeeze(wavefront.latitude) ;
        wlon = squeeze(wavefront.longitude) ;
        walt = squeeze(wavefront.altitude) ;
        for j = 1:(length(wavefront.source_de)-1)
            if eigenrays(tar_num,1).source_de(er) >= wavefront.source_de(j) && ...
                eigenrays(tar_num,1).source_de(er) <= wavefront.source_de(j+1)
                if abs(abs((eigenrays(tar_num,1).source_de(er)) - abs(wavefront.source_de(j)))) ...
                        < abs(abs((eigenrays(tar_num,1).source_de(er)) - abs(wavefront.source_de(j+1))))
                    n = j ;
                else
                    n = j+1 ;
                end
            end
        end
        for j = 1:(length(wavefront.source_az)-1)
            if eigenrays(tar_num,1).source_az(er) < 0
               eray_az = eigenrays(tar_num,1).source_az(er) + 360 ;
            else
               eray_az = eigenrays(tar_num,1).source_az(er) ;
            end
            if eray_az >= wavefront.source_az(j) && eray_az <= wavefront.source_az(j+1)
                if abs(eray_az - abs(wavefront.source_az(j))) ...
                        < abs(eray_az - abs(wavefront.source_az(j+1)))
                    m = j ;
                else
                    m = j+1 ;
                end
            end
        end
        dep = [ n-1, n, n+1 ] ;
        if m+1 > length(wavefront.source_az)
            azi = [ m-1, m, 2 ] ;
        elseif m-1 == 0
            azi = [ length(wavefront.source_az)-2, m, m+1 ] ;
        else
            azi = [ m-1, m, m+1 ] ;
        end
        lat = wlat(plot_timerange,dep,azi) ;
        lon = wlon(plot_timerange,dep,azi) ;
        alt = walt(plot_timerange,dep,azi) ;
        t_lat = wlat(t_index,dep,azi) ;
        t_lon = wlon(t_index,dep,azi) ;
        t_alt = walt(t_index,dep,azi) ;
        d_lon = ones(3,3) ;
        d_lat = d_lon ;
        d_alt = d_lon ;
        for i = 1:3
            for j = 1:3
                plot3( lon(:,j,i), lat(:,j,i), alt(:,j,i), line_color ) ;
                plot3( t_lon(1,j,i), t_lat(1,j,i), t_alt(1,j,i), 'ro-' ) ;
                d_lon(i,j) = t_lon(1,i,j) ;
                d_lat(i,j) = t_lat(1,i,j) ;
                d_alt(i,j) = t_alt(1,i,j) ;
            end
        end
        k = surf( d_lon, d_lat, d_alt, ...
            'EdgeColor', 'r', 'FaceColor', 'none' ) ;
        h = plot3( ytarget, xtarget, ztarget, 'ko', ...
                   ysource, xsource, zsource, 'ro');
        set(h(1),'MarkerEdgeColor','k','MarkerFaceColor','k');
        set(h(2),'MarkerEdgeColor','r','MarkerFaceColor','r');
        gen_proploss_labels(h(1)) ;
        gen_proploss_labels(h(2)) ;
        gen_wave_labels(k) ;
        view(az,el);
        title(sprintf( 'WaveQ3D Visualization' )) ; 
        if freeze_axes
            set(a,'XLim',x_dim) ;
            set(a,'YLim',y_dim) ;
            set(a,'ZLim',z_dim) ;
        end
        xlabel('Longitude')
        ylabel('Latitude')
        zlabel('Depth (m)')
        grid on
        set(gca,'color','none')
        hold off;
        if bathy_show == get(bathy_toggle,'Max')
            bathy_plot
        else
            colorbar off ;
        end        
        
    end

%% ----Callback functions for wavefronts, proploss/eigenrays and bathy-----
    % All variables are set to global to allow use in any function
    function wavefront_load_Callback(src,evt)
        if initialize
            init_hbar = waitbar(0,'Setting up framework','Name','Initializing GUI') ;
            filename = get(wavebox, 'string') ;
            waitbar(2/11,init_hbar,'Loading Wavefront...') ;
            wavefront = load_wavefront(filename) ;
            num_de = length(wavefront.source_de) ;
            label = strcat('DE index (1:',num2str(num_de),')') ;
            set(debox_label,'String',label) ;
            num_az = length(wavefront.source_az) ;
            label = strcat('AZ index (1:',num2str(num_az),')') ;
            set(azbox_label,'String',label) ;
            find_max_time ;
            set(t_max_box,'String',num2str(wave_max_time));
        else
            filename = get(wavebox, 'string') ;
            if isempty(filename)
                filename = uigetfile({'*.nc','NetCDF Files (*.nc)'; ...
                    '*.*', 'All Files (*.*)'},'Choose the wavefront file.') ;
            end
            hbar = waitbar(0,['Loading Wavefront...']) ;
            set(wavebox, 'String', filename);
            waitbar(4/11);
            clear wavefront;
            wavefront = load_wavefront(filename);
            num_de = length(wavefront.source_de) ;
            label = strcat('DE index (1:',num2str(num_de),')') ;
            set(debox_label,'String',label) ;
            num_az = length(wavefront.source_az) ;
            label = strcat('AZ index (1:',num2str(num_az),')') ;
            set(azbox_label,'String',label) ;
            find_max_time ;
            set(t_max_box,'String',num2str(wave_max_time));
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                eigenray_plot ;
            else
                create_plot ;
            end
            if ~isempty(wavefront)
                waitbar(1.0);
                pause(0.1)
                msgbox('Wavefront loaded successfully','Load Complete','modal')
                close(hbar);
            end
        end
    end

    function proploss_load_Callback(src,evt)
        if initialize
            waitbar(6/11,init_hbar,'Loading Proploss...') ;
            pause(0.5);
            filename = get(propbox, 'string') ;
            [proploss, eigenrays] = load_proploss(filename) ;
            populate_eigenrays ;
            targets = ones(length(proploss),3) ;
            for i = 1:length(proploss.latitude) ;
                targets(i,:) = [ proploss.longitude(i), proploss.latitude(i), ...
                               proploss.altitude(i) ] ;
            end
        else
            filename = get(propbox, 'string') ;
            clear proploss ;
            clear eigenrays ;
            if isempty(filename)
                filename = uigetfile({'*.nc','NetCDF Files (*.nc)'; ...
                    '*.*', 'All Files (*.*)'},'Choose the proploss file.') ;
            end
            set(propbox, 'String', filename);
            [proploss, eigenrays] = load_proploss(filename);
            populate_eigenrays ;
            targets = ones(length(proploss),3) ;
            for i = 1:length(proploss.latitude) ;
                targets(i,:) = [ proploss.longitude(i), proploss.latitude(i), ...
                               proploss.altitude(i) ] ;
            end
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                eigenray_plot ;
            else
                create_plot ;
            end
            if( ~isempty(proploss) && ~isempty(eigenrays) )
                msgbox('Proploss and Eigenray information loaded successfully', ...
                'Load Complete','modal') ;
            end
        end
    end

    function bathy_load_Callback(src,evt)
        if initialize
            waitbar(9/11,init_hbar,'Loading Bathymetry...') ;
            pause(0.2);
            filename = get(bathybox, 'string') ;
            bathymetry = load_bathymetry(filename) ;
            minlat = 24.9 ; maxlat = 25.65 ;
            minlng = 56.65 ; maxlng = 57.5 ; 
            waitbar(1.0) ;
            msgbox('GUI setup complete','Load Complete','modal')
            close(init_hbar) ;
            initialize = false ;
        else
            filename = get(bathybox, 'string') ;
            clear bathymetry ;
            if isempty(filename)
                filename = uigetfile({'*.nc','NetCDF Files (*.nc)'; ...
                    '*.*', 'All Files (*.*)'},'Choose the bathymetry file.') ;
            end
            set(bathybox, 'String', filename);
            bathymetry = load_bathymetry(filename);
            if( ~isempty(bathymetry) )
                uiwait(msgbox('Bathymetry loaded successfully', ...
                'Load Complete','modal') ) ;
            end
            range = bathyrange( bathymetry.latitude(length(bathymetry.latitude)), ...
                        bathymetry.latitude(1), ...
                        bathymetry.longitude(length(bathymetry.longitude)), ...
                        bathymetry.longitude(1) ) ;
            if ~isempty(range)
                maxlat = range(1) ;
                minlat = range(2) ;
                maxlng = range(3) ;
                minlng = range(4) ;
            end
        end
    end
    % initialize the above buttons and callbacks
    wavefront_load_Callback(wavebox_button,[]) ;
    proploss_load_Callback(propbox_button,[]) ;
    bathy_load_Callback(bathybox_button,[]) ;

    %% -------Callbacks for min/max of DE and AZ--------
    function ChangeDE_Callback(src,evt)
        de_min = str2double(get(debox_min,'string')) ;
        de_max = str2double(get(debox_max,'string')) ;
        if isnan(de_min)
            uicontrol(debox_min)
        elseif isnan(de_max)
            uicontrol(debox_max)
        else
            dstr = strcat('DE(',num2str(de_min),'):',num2str(wavefront.source_de(de_min))) ;
            set(debox_min,'tooltipstring',dstr) ;
            dstr = strcat('DE(',num2str(de_max),'):',num2str(wavefront.source_de(de_max))) ;
            set(debox_max,'tooltipstring',dstr) ;
            new_de = de_min:de_max ;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            else
                create_plot ;
            end
        end
    end

    function ChangeAZ_Callback(src,evt)
        az_min = str2double(get(azbox_min,'string')) ;
        az_max = str2double(get(azbox_max,'string')) ;
        if isnan(az_min)
            uicontrol(azbox_min)
        elseif isnan(az_max)
            uicontrol(azbox_max)
        else
            astr = strcat('AZ(',num2str(az_min),'):',num2str(wavefront.source_az(az_min))) ;
            set(azbox_min,'tooltipstring',astr) ;
            astr = strcat('AZ(',num2str(az_max),'):',num2str(wavefront.source_az(az_max))) ;
            set(azbox_max,'tooltipstring',astr) ;
            new_az = az_min:az_max ;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            else
                create_plot ;
            end
        end
    end

    %% -------Callbacks for time values-------
    function ChangeT_Callback(hObj, eventdata, handles)
        time = str2double(get(hObj,'string')) ;
        if time > max_time  
            errordlg('Time must be 1 <= t <= max_time.', ...
                'Invalid Time', 'modal') ;
        elseif time < 1
            errordlg('Time must be 1 <= t <= max_time.', ...
                'Invalid Time', 'modal') ;
        else
            t_index = time ;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                create_plot ;
            end
        end
    end

    function ChangeT_max_Callback(hObj, eventdata, handles)
        time = str2double(get(hObj,'string')) ;
        if time > wave_max_time  
            errordlg('Max time may not be greater than wave_max_time.', ...
                'Invalid Time', 'modal') ;
        elseif time < 1
            errordlg('Max time must be greater than 1.', ...
                'Invalid Time', 'modal') ;
        else
            max_time = time ;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                create_plot ;
            end
        end
    end

    function t_increase_Callback(src,evt)
       inc = str2double(get(t_increment,'string')) ;
       if isnan(inc)
           inc = 1 ;
       end
       t_temp = t_index + inc ;
       if t_temp > max_time
           msgbox('Reached max_time.', 'Maximum time', 'modal') ;
       else
           t_index = t_temp ;
           set(t_box,'String',t_index) ;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                create_plot ;
            end
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
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                create_plot ;
            end
       end
    end

    function find_max_time
        wave_max_time = length( wavefront.travel_time ) ;
        label = strcat('Time (1:',num2str(wave_max_time),')') ;
        set(t_box_label,'String',label) ;
        max_time = wave_max_time ;
    end

    function SetTimeRange_Callback(src,evt)
        time_check = get(src,'Value') ;
        SetTimeRange(time_check) ;
        if ray_show == get(ray_toggle,'Max')
            ray_plot ;
        elseif show_mesh == get(show_mesh_plot,'Max')
            mesh_plot ;
        elseif plot_erays == get(show_eigenrays,'Max')
            if neighbor == get(show_neighbors,'Max')
                neighbor_plot ;
            else
                eigenray_plot ;
            end
        else
            create_plot ;
        end
    end

    function SetTimeRange(val)
        if val == 1
            plot_timerange = 1:max_time ;
        elseif val == 2
            plot_timerange = 1:t_index ;
        else
            plot_timerange = t_index:max_time ;
        end
    end

    %% -------Time Playback interface-------
    function t_playback_Callback(src,evt)
        play_button = get(src,'value') ;
        t_index = str2double(get(t_box,'string')) ;
        inc = str2double(get(t_increment,'string')) ;
        if isnan(inc)
           loop = true ;
           inc = 1 ;
        else
            loop = false ;
        end
        if play_button == get(src,'Max')
            set(t_play_button,'tooltipstring','Pause animation') ;
            set(src,'CData',pause_img) ;
            if strcmp(state,'Reverse')
                while(t_index > inc)
                   play_button = get(src,'Value') ;
                   if play_button == get(src,'Min')
                       break;
                   end;
                   t_index = t_index - inc ;
                   set(t_box,'String',t_index) ;
                   if ray_show == get(ray_toggle,'Max')
                       ray_plot ;
                   elseif show_mesh == get(show_mesh_plot,'Max')
                       mesh_plot ;
                   elseif plot_erays == get(show_eigenrays,'Max')
                       if neighbor == get(show_neighbors,'Max')
                           neighbor_plot ;
                       else
                           eigenray_plot ;
                       end
                   else
                       create_plot ;
                   end
                   pause(0.01)
                end
                set(src,'CData',play_img) ;
            elseif strcmp(state,'Forward')
                while(t_index <= max_time-inc)
                   if loop
                       if t_index == max_time-1
                           t_index = 1 ;
                           pause(0.5) ;
                       end
                   end
                   play_button = get(src,'Value') ;
                   if play_button == get(src,'Min')
                       break;
                   end;
                   t_index = t_index + inc ;
                   set(t_box,'String',t_index) ;
                   if ray_show == get(ray_toggle,'Max')
                       ray_plot ;
                   elseif show_mesh == get(show_mesh_plot,'Max')
                       mesh_plot ;
                   elseif plot_erays == get(show_eigenrays,'Max')
                       if neighbor == get(show_neighbors,'Max')
                           neighbor_plot ;
                       else
                           eigenray_plot ;
                       end
                   else
                       create_plot ;
                   end
                   pause(0.1) ;
                end
                set(src,'Value',0) ;
                set(src,'CData',play_img) ;
            else
                errordlg('Please choose the play state before pressing play.', ...
                    'Playback Error','modal') ;
                set(src,'CData',play_img) ;
            end
        elseif play_button == get(src,'Min')
            set(src,'CData',play_img) ;
            if strcmp(state,'Reverse')
                set(t_play_button,'tooltipstring','Animate wavefront backwards in time') ;
            else
                set(t_play_button,'tooltipstring','Animate wavefront forward in time') ;
            end
            set(t_box,'String',t_index) ;
        end
    end

    function update_state_Callback(src,evt)
        state = get(src,'String') ;
        if strcmp(state,'<<')
            state = 'Reverse' ;
            set(t_play_button,'tooltipstring','Animate wavefront backwards in time') ;
        elseif strcmp(state,'>>')
            state = 'Forward' ;
            set(t_play_button,'tooltipstring','Animate wavefront forward in time') ;
        else
            set(t_state,'String','state')
        end
    end
    
    %% View settings
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
    
    %% -----Bathy Toggle-----
    function bathy_Callback(src,evt)
        bathy_show = get(src,'Value') ;
        if bathy_show == get(src,'Max')
            bathy_plot ;
            set(src,'String','Hide Bathy') ;
        else
            set(src,'String','Show Bathy') ;
            colorbar off;
            if ray_show == get(ray_toggle,'Max')
                ray_plot ;
            elseif show_mesh == get(show_mesh_plot,'Max')
                mesh_plot ;
            elseif plot_erays == get(show_eigenrays,'Max')
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                create_plot ;
            end
        end
    end
    
    %% -----Switch between Radials and 3D view-----
    function ray_Callback(src,evt)
        ray_show = get(src,'Value') ;
        if ray_show == get(src,'Max')
            set(src,'String','3D') ;
            set(show_mesh_plot,'Enable','off') ;
            ray_plot ;
        elseif ray_show == get(src,'Min')
            set(src,'String','Radials') ;
            set(show_mesh_plot,'Enable','on') ;
            create_plot ;
        end
    end

    %% -----Callback for Line Color-----
    function LineColorCallback(src,evt)
        line_color = get(src, 'Value');
        line_color = char(colors(line_color)) ;
        if ray_show == get(ray_toggle,'Max')
            ray_plot ;
        elseif show_mesh == get(show_mesh_plot,'Max')
            mesh_plot ;
        elseif plot_erays == get(show_eigenrays,'Max')
            if neighbor == get(show_neighbors,'Max')
                neighbor_plot ;
            else
                eigenray_plot ;
            end
        else
            create_plot ;
        end
    end
    
    %% -----Plot Customization Callbacks and buttons-----
    function freeze_plot_Callback(src,evt)
        check = get(src,'Value') ;
        if check == get(src,'Max')
            x_dim = xlim ;
            y_dim = ylim ;
            z_dim = zlim ;
            freeze_axes = true ;
            set(freeze_plot,'String','Unfreeze') ;
            set(freeze_plot,'tooltipstring','Unfreeze plot axes') ;
        else
            xlim('auto') ;
            ylim('auto') ;
            zlim('auto') ;
            update_axis_vals ;
            freeze_axes = false ;
            set(freeze_plot,'String','Freeze') ;
            set(freeze_plot,'tooltipstring','Freeze plot axes') ;
        end
    end

    function MeshPlot_Callback(src,evt)
        show_mesh = get(src,'Value') ;
        if show_mesh == get(src,'Max')
            set(show_mesh_plot,'String','Surface') ;
            set(ray_toggle,'Enable','off') ;
            mesh_plot ;
        else
            set(show_mesh_plot,'String','Mesh')
            set(ray_toggle,'Enable','on') ;
            create_plot ;
        end
    end

    function update_axis_vals
        x_vals = xlim ;
        set(x_min_box,'String',x_vals(1)) ;
        set(x_max_box,'String',x_vals(2)) ;
        y_vals = ylim ;
        set(y_min_box,'String',y_vals(1)) ;
        set(y_max_box,'String',y_vals(2)) ;
        z_vals = zlim ;
        set(z_min_box,'String',z_vals(1)) ;
        set(z_max_box,'String',z_vals(2)) ;
    end

    function ChangeX_Callback(src,evt)
        xmin = str2double(get(x_min_box,'string')) ;
        xmax = str2double(get(x_max_box,'string')) ;
        x_dim = [xmin xmax] ;
        if isempty(xmax)
            uicontrol(x_max_box) ;
        elseif isempty(xmin)
            uicontrol(x_min_box) ;
        else
            xlim([xmin xmax]) ;
        end
    end

    function ChangeY_Callback(src,evt)
        ymin = str2double(get(y_min_box,'string')) ;
        ymax = str2double(get(y_max_box,'string')) ;
        y_dim = [ymin ymax] ;
        if isempty(ymax)
            uicontrol(y_max_box) ;
        elseif isempty(ymin)
            uicontrol(y_min_box) ;
        else
            ylim([ymin ymax]) ;
        end
    end

    function ChangeZ_Callback(src,evt)
        zmin = str2double(get(z_min_box,'string')) ;
        zmax = str2double(get(z_max_box,'string')) ;
        z_dim = [zmin zmax] ;
        if isempty(zmax)
            uicontrol(z_max_box) ;
        elseif isempty(zmin)
            uicontrol(z_min_box) ;
        else
            zlim([zmin zmax]) ;
        end
    end

    %% ---------Create Datacursor labels for all wavefront/targets---------
    function gen_wave_labels(handle)
        if isempty(dep) & isempty(azi)
            if isempty(new_az) & isempty(new_de)
                A = de_min:length(wavefront.source_de) ;
                B = 1:de_max ;
                dep = cat(2,A,B) ;
                A = az_min:length(wavefront.source_az) ;
                B = 1:az_max ;
                azi = cat(2,A,B) ;
            elseif isempty(new_az)
                dep = new_de ;
                A = az_min:length(wavefront.source_az) ;
                B = 1:az_max ;
                azi = cat(2,A,B) ;
            elseif isempty(new_de)
                A = de_min:length(wavefront.source_de) ;
                B = 1:de_max ;
                dep = cat(2,A,B) ;
                azi = new_az ;
            else
                dep = new_de ;
                azi = new_az ;
            end
        end
        vde = length(dep) ;
        vaz = length(azi) ;
        labels = cell(vde,vaz) ;
        for j = 1:vaz
            for i = 1:vde
                if wavefront.on_edge(t_index,dep(i),azi(j))
                    label = strcat('DE: ',num2str(wavefront.source_de(dep(i))), ...
                        ' AZ: ',num2str(wavefront.source_az(azi(j))),'*') ;                        
                else
                    label = strcat('DE: ',num2str(wavefront.source_de(dep(i))), ...
                        ' AZ: ',num2str(wavefront.source_az(azi(j)))) ;
                end
                labels{i,j} = label ;
            end
        end
        waveDataCursor(handle, labels) ;
        dep = [] ;
        azi = dep ;
    end
    
    function gen_proploss_labels(handle)
        if handle == h(2) ;
            labels = cell(1) ;
            labels{1} = 'Source' ;
        else
            if isempty(tar_num)
                num_tar = length(proploss.latitude) ;
            else
                num_tar = length(proploss.latitude(tar_num)) ;
            end
            labels = cell(num_tar) ;
            for i = 1:num_tar
                labels{i} = strcat('Target #',num2str(i)) ;
            end
        end
        proplossDataCursor(handle,labels) ;
    end

    %% ----------------Popluates eigenray data-----------------
    % Updates the uitable data field with the eigenray data for the
    % selected target
    function populate_eigenrays
        num_tar = length(proploss.latitude) ;
        eigenray_data = cell(num_tar,1) ;
        for i = 1:length(proploss.latitude)
            num_erays = length(eigenrays(i,1).travel_time) ;
            for j = 1:num_erays
                temp_data(j,:) = [eigenrays(i,1).travel_time(j) ...
                                  eigenrays(i,1).source_de(j) ...
                                  eigenrays(i,1).source_az(j) ...
                                  eigenrays(i,1).target_de(j) ...
                                  eigenrays(i,1).target_az(j) ...
                                  eigenrays(i,1).intensity(j) ...
                                  double(proploss.frequency) ...
                                  eigenrays(i,1).phase(j) ...
                                  double(eigenrays(i,1).surface(j)) ...
                                  double(eigenrays(i,1).bottom(j)) ...
                                  double(eigenrays(i,1).caustic(j)) ] ;
            end
            eigenray_data{i} = temp_data ;
        end
    end

    function update_eigenray_table(handle)
        dcm_obj = datacursormode(gcf) ;
        info = getCursorInfo(dcm_obj) ;
        if isempty(info)
            estr = strcat('Please choose a target with the data cursor', ...
                ' before switching to view eigeinrays mode.' ) ;
            errordlg(estr,'Eigenray Data Error','modal') ;
        else
            for i = 1:length(proploss.latitude) ;
                if targets(i,:)==info.Position
                    tar_num = i ;
                end
            end
            set(eigenraybox,'Data',eigenray_data{tar_num,1}) ;
        end
    end

    %% --------------------Generate plot of eigenrays-------------------
    function show_eigenrays_Callback(src,evt)
        plot_erays = get(src,'Value') ;
        if plot_erays == get(src,'Max')
            update_eigenray_table(h(1)) ;
            if isempty(tar_num)
               set(src,'Value',0) ;
            else
                set(show_eigenrays,'String','Wavefront') ;
                set(debox_min,'Enable','off') ;
                set(debox_max,'Enable','off') ;
                set(azbox_min,'Enable','off') ;
                set(azbox_max,'Enable','off') ;
                set(ray_toggle,'Enable','off') ;
                set(ray_toggle,'Value',0) ;
                set(ray_toggle,'String','Radials') ;
                ray_show = 0 ;
                set(show_mesh_plot,'Enable','off') ;
                set(show_mesh_plot,'Value',0) ;
                set(show_mesh_plot,'String','Mesh') ;
                show_mesh = 0 ;
                eigenray_plot ;
            end
        else
            set(show_eigenrays,'String','Eigenrays') ;
            set(debox_min,'Enable','on') ;
            set(debox_max,'Enable','on') ;
            set(azbox_min,'Enable','on') ;
            set(azbox_max,'Enable','on') ;
            set(ray_toggle,'Enable','on') ;
            set(show_mesh_plot,'Enable','on') ;
            set(eigenraybox,'Data',{}) ;
            set(show_neighbors,'Enable','off') ;
            set(show_neighbors,'String','Neighbors') ;
            set(show_neighbors,'Value',0) ;
            neighbor = 0 ;
            tar_num = [] ;
            create_plot ;
        end
    end

    function ChooseErays_Callback(src,evt)
        erays = evt.Indices ;
        ne = size(erays) ;
        er = [] ;
        for i = 1:ne(1)
            dup = false ;
            for j = 1:length(er)
                if er(j) == erays(i,1,1)
                    dup = true ;
                    break ; 
                end
            end
            if dup == true 
                break ; 
            end
            er(i) = erays(i,1,1) ;
        end
        if plot_erays == get(show_eigenrays,'Max')
            if length(er) == 1
                set(show_neighbors,'Enable','on') ;
                if neighbor == get(show_neighbors,'Max')
                    neighbor_plot ;
                else
                    eigenray_plot ;
                end
            else
                eigenray_plot ;
                set(show_neighbors,'Enable','off') ;
                set(show_neighbors,'String','Neighbors') ;
                set(show_neighbors,'Value',0) ;
                neighbor = 0 ;
            end
        end
    end

    function show_neighbors_Callback(src,evt)
        neighbor = get(src,'Value') ;
        if neighbor == get(src,'Max')
            set(show_neighbors,'String','Hide') ;
            neighbor_plot ;
        else
            set(show_neighbors,'String','Neighbors') ;
            eigenray_plot ;
        end
    end

 end