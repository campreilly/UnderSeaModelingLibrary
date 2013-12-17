function refraction_surface_duct
    %
    % Movie of wavefronts for refraction_surface_duct.
    %
    % Illustrates the on_edge logic for a surface duct environment.
    %
    % Supports menu controls for next/prev wavefront.
    % Maintains user selections for plot axes for next/prev wavefront.
    %
    clear all ; close all;
    
    % Creates the default figure window
    f = figure('Units','characters',...
        'Position',[60 5 120 30],...
        'HandleVisibility','callback',...
        'IntegerHandle','off',...
        'Renderer','painters',...
        'Toolbar','figure',...
        'NumberTitle','off',...
        'MenuBar','none',...
        'Name','Norwegian Sea',...
        'ResizeFcn',@figResize);

    % Create the center panel
    centerPanel = uipanel('bordertype','etchedin',...
        'Units','characters',...
        'Position', [0 0 88 30],...
        'Parent',f);
    % Create the right side panel
    rightPanel = uipanel('bordertype','etchedin',...
        'Units','characters',...
        'Position',[85 0 35 30],...
        'Parent',f,...
        'ResizeFcn',@rightPanelResize);

    function figResize(src,evt)
        fpos = get(f,'Position');
        set(centerPanel,'Position', ...
            [0 0 fpos(3)*85/120 fpos(4)]);
        set(rightPanel,'Position', ...
            [fpos(3)*85/120 0 fpos(3)*35/120 fpos(4)])
    end

    function rightPanelResize(src, evt)
        rpos = get(rightPanel,'Position');
        set(t_plus, 'position', ...
            [rpos(3)*10/35 rpos(4)*28/30 rpos(4)*4/30 rpos(4)*1/30] ) ;
        set(t_inc, 'position', ...
            [rpos(3)*8/35 rpos(4)*28/30 rpos(4)*5/30 rpos(4)*1/30] ) ;
        set(t_minus, 'position', ...
            [rpos(3)*2/35 rpos(4)*28/30 rpos(4)*4/30 rpos(4)*1/30] ) ;
        set(axis_lims, 'position', ...
            [rpos(3)*21/35 rpos(4)*28/30 rpos(4)*11/30 rpos(4)*1/30] ) ;
    end

    t_plus = uicontrol(f, 'style', 'pushbutton', ...
        'units', 'characters', ...
        'position', [15 28 4 1], ...
        'string', '+', ...
        'parent', rightPanel, ...
        'callback', @step_forward) ;
    t_inc = uicontrol(f, 'style', 'edit', ...
        'units', 'characters', ...
        'position', [8 28 5 1], ...
        'backgroundcolor', 'white', ...
        'string', '1', ...
        'parent', rightPanel) ;
    t_minus = uicontrol(f, 'style', 'pushbutton', ...
        'units', 'characters', ...
        'position', [2 28 4 1], ...
        'string', '-', ...
        'parent', rightPanel, ...
        'callback', @step_backward) ;
    axis_lims = uicontrol(f, 'style', 'pushbutton', ...
        'units', 'characters', ...
        'position', [21 28 11 1], ...
        'string', 'Reset View', ...
        'parent', rightPanel, ...
        'callback', @reset_scale);
    
    %===========Main Plot function staging area===========
    a = axes('parent', centerPanel);
    view(a, 2);
    set(a,'XGrid','on','YGrid','on');
    t_index = 10 ;
    
    % load wavefront and convert into range/depth coordinates
    wavefront = load_wavefront('refraction_surface_duct.nc') ;

    az_index = 1 ;
    wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
    wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) / 1e3 ; % range in meters
    walt = squeeze(wavefront.altitude(:,:,az_index))  ;
    wcst = squeeze(wavefront.caustic(:,:,az_index))  ;
    wedg = squeeze(wavefront.on_edge(:,:,az_index))  ;
    max_time = length( wavefront.travel_time ) ;
    
    % search for caustics
    cst_index = zeros( size( wcst ) ) ;
    for t=2:max_time
        for d=1:length(wcst(1,:)) 
            if ( wcst(t,d) > wcst(t-1,d) ) 
                cst_index(t,d) = 1.0 ;
            end
        end
    end

    scale = [0 30 -500 0] ;
    begin = true ;
    function create_plot
        axes(a) ;
        cla ;
        plot( wlat(t_index,:), walt(t_index,:), 'r-', wlat, walt, 'c-', ...
                  wlat( cst_index>0 ), walt( cst_index>0 ), 'bo', ...
                  wlat( wedg~=0 ), walt( wedg~=0 ), 'g.' ) ;
        title( sprintf('Surface Duct, time = %.1f sec', ...
               wavefront.travel_time(t_index)) ) ;
        xlabel('Range (km)') ;
        ylabel('Depth (m)') ;
        grid on ;
        axis( scale ) ;
    end

    function step_forward(src,evt)
        inc = str2double(get(t_inc,'string')) ;
        t_index = t_index + inc ;
        if t_index > max_time
            msgbox(sprintf('Reached maximum time %.1f sec', ...
                 wavefront.travel_time(max_time)),'Time bounds') ;
            t_index = t_index - inc ;
        else
            if begin
                scale = [0 30 -500 0] ;
                begin = false ;
            else
                scale = cat(2, xlim, ylim) ;
            end
            create_plot ;
        end
    end

    function step_backward(src,evt)
        inc = str2double(get(t_inc,'string')) ;
        t_index = t_index - inc ;
        if t_index <= 0
            msgbox('Reached minimum time 0.0 sec','Time bounds') ;
            t_index = t_index + inc ;
        else
            if begin
                scale = [0 30 -500 0] ;
                begin = false ;
            else
                scale = cat(2, xlim, ylim) ;
            end
            create_plot ;
        end
    end

    function reset_scale(src,evt)
        scale = [0 30 -500 0] ;
        create_plot ;
    end
end