%%
% Computes complex pressure in the Deane/Buckingham wedge solution for all
% combinations of source image and target locations.  Limits the evaluation
% to 2-D contributions, those in the plane of the source. Sums the
% wedge_integ_2d() contributions over all source images, where each image
% is defined by a specific combination of source and bottom reflections.
% Implements equation (24) in March 1993 paper.
%
%   pressure = wedge_pressure_2d( ...
%       wave_number, max_bottom, tolerance, ...
%       source_range, source_zeta, target_range, target_zeta )
%
%   pressure        = matrix of complex pressures (one for each target)
%                     summed over multiple source images.
%   wave_number     = magnitude of the acoustic wave number
%   max_bottom      = max number of bottom bounces to model
%   tolerance       = relative size for cutting off # of bottom bounces early
%   source_range    = source range from apex
%   source_zeta     = source D/E angle relative to ocean bottom
%   target_range    = matrix of target ranges from apex
%   target_zeta     = matrix of target D/E angles relative to ocean bottom
%
% Relies on global variables for the bottom properties of wedge_angle.
%
% Although the maximum number of bottom bounces can be controlled using
% max_bottom, the series terminates prematurely if the current contribution
% is smaller than tolerance.
%
function pressure = wedge_pressure_2d( ...
    wave_number, max_bottom, tolerance, ...
    source_range, source_zeta, target_range, target_zeta )

    global wedge_angle

    % source contribution = s(0,0) term

    [ range, zeta ] = spherical_add( ...
        target_range, target_zeta, 0.0, ...
        -source_range, source_zeta, 0.0 ) ;
    pressure = wedge_integ_2d( 0, 0, wave_number, range, (zeta+pi/2) ) ;				%adjusted for x to z axis orientation (needs to be corrected by pi)
	printf("zeta_0_0: %f\n", (zeta+pi/2-pi)*180/pi);		%debug only

    % 1st surface image = s(1,0) term

    [ range, zeta ] = spherical_add( ...
        target_range, target_zeta, 0.0, ...
        -source_range, -source_zeta, 0.0 ) ;
    pressure = pressure + wedge_integ_2d( 1, 0, wave_number, range, (zeta+pi/2) ) ;			%adjusted for x to z axis orientation
	printf("zeta_1_0: %f\n", (zeta+pi/2)*180/pi);		%debug only

    % loop through images that include bottom reflection
    % need to check the source_zeta terms in spherical_add()!!!

    for num_bottom = 1:max_bottom

        % source->bottom->...->target path = s'(n-1,n) term (below, bottom is first interaction, l = -(2n-1), n=1,2,3,...)

        [ range, zeta ] = spherical_add( ...
            -target_range, target_zeta, 0.0, ...
            source_range, -source_zeta+wedge_angle*(2*num_bottom), 0.0 ) ;
	printf("zeta'_%i_%i: %f\n", num_bottom-1, num_bottom, -(180-(zeta+pi/2)*180/pi));		%debug only
        new = wedge_integ_2d( ...
            num_bottom-1, num_bottom, wave_number, range, 3*pi/2-(zeta+pi/2) ) ;			%adjusted for x to z axis orientation

        % source->surface->bottom->...->target path = s'(n,n) term (below, surface is the first interaction, l = -2n, n=1,2,3,...)

        [ range, zeta ] = spherical_add( ...
            -target_range, target_zeta, 0.0, ...
            source_range, source_zeta+wedge_angle*(2*num_bottom), 0.0 ) ;
	printf("zeta'_%i_%i: %f\n", num_bottom, num_bottom, -(180-(zeta+pi/2)*180/pi));			%debug only
        new = new + wedge_integ_2d( ...
            num_bottom, num_bottom, wave_number, range, 3*pi/2-(zeta+pi/2) ) ;				%adjusted for x to z axis orientation

        % source->bottom->surface->...->target path = s(n,n) term (above, bottom is the first interaction, l = 2n, n=1,2,3,...)

        [ range, zeta ] = spherical_add( ...
            target_range, target_zeta, 0.0, ...
	    -source_range, source_zeta-2*wedge_angle*num_bottom, 0.0 ) ;
	printf("zeta_%i_%i: %f\n", num_bottom, num_bottom, (zeta+pi/2)*180/pi);		%debug only
        new = new + wedge_integ_2d( ...
            num_bottom, num_bottom, wave_number, range, (zeta+pi/2) ) ;					%adjusted for x to z axis orientation

        % source->surface->bottom->...->target path = s(n+1,n) term (above, surface is the first interaction, l=2n+1, n=1,2,3,...)

        [ range, zeta ] = spherical_add( ...
            target_range, target_zeta, 0.0, ...
            -source_range, -source_zeta-2*wedge_angle*num_bottom, 0.0 ) ;
	printf("zeta_%i_%i: %f\n", num_bottom+1, num_bottom, (zeta+pi/2)*180/pi);		%debug only
        new = new + wedge_integ_2d( ...
            num_bottom+1, num_bottom, wave_number, range, (zeta+pi/2) ) ;				%adjusted for x to z axis orientation

        % incorporate new contribution into the sum
        % exit early if the new contribution is small

        pressure = pressure + new ;
        if ( max(abs(new./pressure)) < tolerance )
            break ;
        end

    end

end
