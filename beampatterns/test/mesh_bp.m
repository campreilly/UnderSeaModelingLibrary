function [ax] = mesh_bp(filename, ax, data)
% produces a mesh plot of a beam pattern spit out from the usml test
% proceures for the bp package
%
% ax is an optional input, if passed then the mesh will go onto the same
% axes
%
% data is an optional input, its a 361x181 matrix of points, the same that
% would be in the csv file passed, but it skips that csv read step.

    if ~isempty(ax)
        hold all;
    else
        fig = figure;
        ax = axes;
    end

    if isempty(data)
        data=importdata(filename,',');
    end    
    
    db = 30 + 10.0 * log10( abs(data) ) ;
    
    % in the event that we limited the dynamic range we would have some
    % negative db entries.  get rid of those
    db(db<0) = 0;
    
    de_gv = (-90:90)*pi/180 ;    % D/E angles where to evaluate
    az_gv = (0:360)*pi/180 ;   % AZ angles where to evaluate
    [DE,AZ] = meshgrid(de_gv,az_gv) ;
    xx = db .* cos(DE) .* cos(AZ) ;
    yy = -db .* cos(DE) .* sin(AZ) ;
    zz = db .* sin(DE) ;
    
    % plot mech using dB size vectors, but linear color scaling.  this will
    % retain the sign of the intensity
    mesh(ax, xx, yy, zz, data) ;

    axis(ax,'square');
    daspect(ax,[1 1 1]);
    set(gca,'CLim',[0 1]);
    colorbar(ax);
    xlabel(ax, 'front'); ylabel(ax, 'right'); zlabel(ax, 'up');
    yt = get(ax, 'YTick');
    set(ax, 'YTick',yt, 'YTickLabel',fliplr(yt))
    
    
    
    
    % polar plots
%     figure;
%     subplot(131);   polarplot( AZ(:,1), abs(data(:,91))' );
%                     title( 'az plot intensity linear (0 is north)' );
%                     axx = gca;
%                     axx.ThetaZeroLocation = 'top';
%                     axx.ThetaDir = 'clockwise';
%                     
%     subplot(132);   polarplot( DE(1,:), abs(data(181,:)) );
%                     title( 'de plot intensity linear sliced at 180 (-90..90)' );
%                     
%     subplot(133);   polarplot( DE(1,:), abs(data(91,:)) );
%                     title( 'de plot intensity linear sliced at 90  (-90..90)' );
                    
                    
    % change axes bach to the mesh
    axes(ax);


end