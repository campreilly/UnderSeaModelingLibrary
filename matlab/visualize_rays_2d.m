function visualize_rays_2d(wavefront, bathymetry, profile, limits)
% 2D visualizer for USML ray paths
%   A uifigure application that supports interactive visualization of
%   acoustic ray paths. The central panel displays 2D bathymetry and ray
%   path data. The left panel is a interactive list of azimuth and
%   depression/elevation angles. The right panel displays sound speed as a
%   function of depth (only if profile_file provided).
% SYNTAX
%   visualize_rays_2d(wavefront, bathymetry, profile, limits);
%   visualize_rays_2d(wavefront, bathymetry, [], limits);
%   visualize_rays_2d(wavefront, bathymetry);
% INPUTS
%   wavefront       = USML 3D wavefront data loaded from NetCDF file
%   bathymetry      = USML 3D bathymetry data loaded from NetCDF file
%   profile         = USML 3D profile data loaded from NetCDF file (optional)
%   limits          = limits on range and depth axes (optional)

azSelect = 0; % select just one AZ
deList = []; % select all D/E

% manage app layout

fig = uifigure('Name', 'Visualize Rays 2D', ...
    'HandleVisibility', 'on', 'Visible', 'off');
pos = get(fig,'Position');
set(fig,'Position',[ pos(1)-pos(3) pos(2)-pos(4) pos(3)*2 pos(4)*2 ]);
enableLegacyExplorationModes(fig); % let "close all" work with this GUI

if nargin < 3 || isempty(profile) % just left and central panels
    layout = uigridlayout(fig, [1 2]);
    layout.ColumnWidth = {150, '1x'};
else % with profile data on right
    layout = uigridlayout(fig, [1 3]);
    layout.ColumnWidth = {150, '5x', '1x'};
end
if nargin < 4
    limits = [];
elseif length(limits) > 4
    limits = limits(1:4); % allow 3D limits to be used on 2D plot
end

% display ray trace in center panel

ax_geo = uiaxes(layout);
ax_geo.Layout.Row = 1;
ax_geo.Layout.Column = 2;
[hrays, geo_line] = plot_raytrace_2d(ax_geo, wavefront, azSelect, deList);
axis(ax_geo, [0 80 -6000 0]);
grid(ax_geo, 'on');
xlabel(ax_geo, 'Range (km)');
ylabel(ax_geo, 'Depth (m)');

if isempty(limits)
    drawnow % to get good limits from axis
    limits = axis(ax_geo);
    limits(4) = 0;
end
axis(ax_geo, limits);
hold(ax_geo, 'on');

% display angle table on the left side

angle_table(layout, wavefront);

% display bathymetry in center panel, on top of ray trace

hbathy = plot_bathymetry_2d(ax_geo, bathymetry, geo_line);
hold(ax_geo, 'off');

% display profile in right panel

if nargin >= 3 && ~isempty(profile)
    ax_profile = uiaxes(layout);
    ax_profile.Layout.Row = 1;
    ax_profile.Layout.Column = 3;
    latitude = wavefront.latitude(1, 1, 1);
    longitude = wavefront.latitude(1, 1, 1);
    altitude = wavefront.altitude(1, 1, 1);
    plot_profile(ax_profile, profile, latitude, longitude, altitude);
    grid(ax_profile);
    ylim(ax_profile, [limits(3) limits(4)]);
    ax_profile.XAxisLocation = 'top';
    xlabel(ax_profile, 'Speed (m/s)');
    ylabel(ax_profile, 'Depth (m)');
end

fig.Visible = 'on';

%%
    function ui_panel = angle_table(parent, wavefront)
    % Display interactive list of azimuth and depression/elevation angles
    % in left panel.

    ui_panel = uipanel(parent);
    ui_panel.Layout.Column = 1;
    ui_layout = uigridlayout(ui_panel, [1 2]);

    table1 = uitable(ui_layout, 'Data', wavefront.source_az, ...
        'ColumnWidth', '1x', 'RowStriping', 'off', ...
        'ColumnName', 'AZ', 'RowName', [], ...
        'Multiselect', 'off');
    table1.Layout.Column = 1;
    table1.SelectionChangedFcn = @selectAZ;

    table2 = uitable(ui_layout, 'Data', wavefront.source_de, ...
        'ColumnWidth', '1x', 'RowStriping', 'off', ...
        'ColumnName', 'DE', 'RowName', []);
    table2.Layout.Column = 2;
    table2.SelectionChangedFcn = @selectDE;
    end

%%
    function selectAZ(src, event)
    % Select zero or more azimuth angles from interactive display
    azSelect = [];
    selection = event.Selection(:,1);
    for n = 1:length(selection)
        row = selection(n);
        value = src.Data(row);
        if ~isempty(value)
            azSelect = [azSelect; value]; %#ok<AGROW>
        end
    end

    delete(hbathy); % erase old copy of ray bathymetry
    delete(hrays); % erase old copy of ray plots
    hold(ax_geo, 'on'); % overlay new plot on bathymetry
    [hrays, geo_line] = plot_raytrace_2d(ax_geo, wavefront, azSelect, deList);
    hbathy = plot_bathymetry_2d(ax_geo, bathymetry, geo_line);
    hold(ax_geo, 'off');
    end

%%
    function selectDE(src, event)
    % Select zero or more depression/elevation angles from interactive display
    deList = [];
    selection = event.Selection(:,1);
    for n = 1:length(selection)
        row = selection(n);
        value = src.Data(row);
        if ~isempty(value)
            deList = [deList; value]; %#ok<AGROW>
        end
    end
    delete(hbathy); % erase old copy of ray bathymetry
    delete(hrays); % erase old copy of ray plots
    hold(ax_geo, 'on'); % overlay new plot on bathymetry
    [hrays, geo_line] = plot_raytrace_2d(ax_geo, wavefront, azSelect, deList);
    hbathy = plot_bathymetry_2d(ax_geo, bathymetry, geo_line);
    hold(ax_geo, 'off');
    end
end
