function ax_geo = visualize_rays_3d(wavefront, bathymetry, profile, glimits)
% 3D visualizer for USML ray paths
%   A uifigure application that supports interactive visualization of
%   acoustic ray paths. The central panel displays 2D bathymetry and ray
%   path data. The left panel is a interactive list of azimuth and
%   depression/elevation angles. The right panel displays sound speed as a
%   function of depth (only if profile_file provided).
% SYNTAX
%   visualize_rays_3d(wavefront, bathymetry, profile, limits);
%   visualize_rays_3d(wavefront, bathymetry, [], limits);
%   visualize_rays_3d(wavefront, bathymetry);
% INPUTS
%   wavefront  = USML 3D wavefront data loaded from NetCDF file
%   bathymetry = USML 3D bathymetry data loaded from NetCDF file
%   profile    = USML 3D profile data loaded from NetCDF file (optional)
%   glimits    = longitude, latitude, and depth limits on surface plot (optional)
% OUTPUTS
%   ax_geo     = graphics axis handle for geo-spatial plot

azList = []; % select all azimuths
deList = 0; % select just one D/E

% manage app layout

fig = uifigure('Name', 'Visualize Rays 3D', ...
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
    glimits = [];
end

% display bathymetry in center panel

ax_geo = uiaxes(layout);
ax_geo.Layout.Row = 1;
ax_geo.Layout.Column = 2;
plot_bathymetry_3d(ax_geo, bathymetry, glimits);
zLimits = zlim(ax_geo);
hold(ax_geo, 'on');

% display wavefront in center panel, on top of bathymetry

hrays = plot_raytrace_3d(ax_geo, wavefront, azList, deList);
hold(ax_geo, 'off');

% display angle table on the left side

angle_table(layout, wavefront);

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
    ylim(ax_profile, zLimits);
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
        'ColumnName', 'AZ', 'RowName', []);
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
    azList = [];
    selection = event.Selection(:,1);
    for n = 1:length(selection)
        row = selection(n);
        value = src.Data(row);
        if ~isempty(value)
            azList = [azList; value]; %#ok<AGROW>
        end
    end

    delete(hrays); % erase old copy of ray plots
    hold(ax_geo, 'on'); % overlay new plot on bathymetry
    hrays = plot_raytrace_3d(ax_geo, wavefront, azList, deList);
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
    delete(hrays); % erase old copy of ray plots
    hold(ax_geo, 'on'); % overlay new plot on bathymetry
    hrays = plot_raytrace_3d(ax_geo, wavefront, azList, deList);
    hold(ax_geo, 'off');
    end
end
