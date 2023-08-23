% The script compares different methods of computing rotated sin/cos beam
% patterns.
% 
% 
% e.  Cosine channel.  The cosine channel shall receive acoustic energy from 
% the ocean through a dipole sonar transducer having maximum sensitivity to 
% signals along the reference axis of the buoy.  This signal shall be in 
% phase with respect to the omni channel for acoustic energy originating 
% within ?90o of the positive reference axis and shall be phased 180o with 
% respect to the omni channel for acoustic energy originating at angles 
% greater than ?90o of the positive reference axis.  This signal shall 
% double sideband suppressed carrier modulate a 15 kHz subcarrier phased to 
% lead the phase pilot by the sum of 90 plus the clockwise angle (as viewed 
% from above) from magnetic north to the positive reference axis of the buoy 
% (see Figure 6).  The cosine channel subcarrier frequency shall be exactly 
% % equal to that of the phase pilot.
% 
% 
%          North
%          ^    Ref Axis
%          |  /
%          | /
%          +------>  East
% 


function [] = beam_pattern_cosine_sine_c()

    close all

    % Orientation of the sensor
    yaw = deg2rad(10);
    pitch = deg2rad(10);
    roll = deg2rad(10);
    
    % plot cosine response
    plot_db( 'cos', yaw, pitch, roll );
    f1 = gcf;
    set(f1, 'Name', 'cos');
    
    % plot sine response
    plot_db('sin', yaw, pitch, roll );
    f2 = gcf;
    set(f2, 'Name', 'sin');

    % make a mesh plot of both responses
    % this is matlab hack code
    f3 = figure;
    axesObjs = get(f1, 'Children');
    dataObjs = get(axesObjs, 'Children'); 
    xdata = get(dataObjs{4}, 'XData');
    ydata = get(dataObjs{4}, 'YData');
    zdata = get(dataObjs{4}, 'ZData');
    cdata = get(dataObjs{4}, 'CData');
    mesh(xdata,ydata,zdata,cdata);
    hold all;
    
    axesObjs = get(f2, 'Children');
    dataObjs = get(axesObjs, 'Children'); 
    xdata = get(dataObjs{4}, 'XData');
    ydata = get(dataObjs{4}, 'YData');
    zdata = get(dataObjs{4}, 'ZData');
    cdata = get(dataObjs{4}, 'CData');
    mesh(xdata,ydata,zdata,cdata);
    
    daspect([1 1 1]);
    colorbar;
    xlabel('x'); ylabel('y');
    title('combined response, dB, color scaled to linear intensity');
    
   

end


% takes sin/cos as a string and yaw, pitch, roll of the REFERENCE AXIS
% the cosine beam will have positive MRA on the positive reference axis and
% negative MRA in the negative direction of the reference axis. 
% this function calcualtes sin and cos beams the same, but applies a
% different rotation depnding on which one it was.
function [] = plot_db( s, y_, p_, r_ )

    
    % build a grid vector, and a mesh grid, of 1deg spaced points in
    % azimuth and elevation.  radius needs to be fixed to 1.
    % careful if you change this, we are making some assumptions becuase of
    % the 1deg seperation
    dr = deg2rad(1);
    de_gv = deg2rad([-90:1:90]);
    az_gv = deg2rad([0:1:360]); 
    [DE, AZ, R] = meshgrid(de_gv,az_gv,1);
    

    


    

    
    % Build the beam pattern
    % This swithc statement switches a bunch of different methods of doing
    % this.
    switch "CARTESIAN_ROTATED"
        
        % uses a spehrical dot product directly utilising incomming angles.
        % doesnt support roll
        case "SPERICAL"
            % pattern response, no correction for orientation
            P = cos(AZ) .* cos(DE)  ;
            
            % spherical dot product with a correction for DE definition
            switch s
                case 'sin'
                    P =  sin((-DE+pi/2)).*sin((-p_+pi/2)).*cos(AZ-y_+pi/2) + cos((-DE+pi/2)).*cos((-p_+pi/2));
                case 'cos'            
                    P =  sin((-DE+pi/2)).*sin((-p_+pi/2)).*cos(AZ-y_) + cos((-DE+pi/2)).*cos((-p_+pi/2));
            end
            
            

        % find an orientation indenedpnat pattern then dots this pattern
        % into a rotated referenece axis ( so only the reference axis
        % rotation is done )
        % the pattern is calculated in cartesian coordinates
        case "CARTESIAN"             
            % pattern response, no correction for orientation
            P = cos(AZ) .* cos(DE)  ;
    
            % this is a rotation matrix for performing roll pitch yaw in 
            % cartesian coordinates using tait–bryan chained rotations
            
            % roll, phi
            Rx = [ 1    0        0;
                   0   cos(r_)   -sin(r_);
                   0   sin(r_)  cos(r_) ] ;

            % pitch, theta    
            Ry = [  cos(p_)   0   sin(p_);
                    0         1   0;
                    -sin(p_)  0   cos(p_) ] ;

            % yaw, psi
            Rz = [ cos(y_) -sin(y_) 0;
                   sin(y_) cos(y_)  0;
                   0       0        1  ] ;

            R = Rz * Ry * Rx ;
            
            % the reference axis is the axis in the unrotated space that
            % has the MRA on it.
            % find a vector for the reference axis using the rotation matrix
            % roll will have no impact on cosine beams becuase they are ON the
            % reference axis, but they have a big impact on sine beams
            % this 'reference axis' is basically picking which of the two
            % oreitnation adjustments we can about.  cosine uses yaw and
            % pitch, sine uses yaw and roll.
            switch s
                case 'sin'
                    ref_axis = [0;1;0]; % points 'east'?
                case 'cos'            
                    ref_axis = [1;0;0]; % points 'north'
            end
            mra_vect = R * ref_axis;
            
           
            % repmat and permute the reference axis to match the size of the
            % beam pattenr reponse so we can use the built in dotproduct
            mra_vect = repmat(mra_vect, 1, size(AZ,1), size(AZ,2));
            mra_vect = permute(mra_vect, [2 3 1]);
            
            % get pattern in cartesian coordinates
            % make a jumbo vector with the orthongal axis along the
            % 3rd dimension
            [X Y Z] = sph2cart(AZ,DE,P);
            temp = cat(3,X,Y,Z);
            
            P = dot(temp, mra_vect, 3) ./ P;
        
            
        % same as the last one, except instead of caluclating hte pattenr
        % first (in AZ and DE) and taking the dot product of that, this
        % version caluclates the pattern in cartesian coordinates using a
        % cartesian dot product
        case "CARTESIAN_ROTATED"
            
            % build a rotation matrix
            % roll, phi
            Rx = [ 1    0        0;
                   0   cos(r_)   -sin(r_);
                   0   sin(r_)  cos(r_) ] ;

            % pitch, theta    
            Ry = [  cos(p_)   0   sin(p_);
                    0         1   0;
                    -sin(p_)  0   cos(p_) ] ;

            % yaw, psi
            Rz = [ cos(y_) -sin(y_) 0;
                   sin(y_) cos(y_)  0;
                   0       0        1  ] ;

            R = Rz * Ry * Rx ;
            
            % Get AZ and DE in cartesian. It seems that this is available to us in the library. 
            [X Y Z] = sph2cart(AZ,DE,ones(size(AZ)));
             XYZ = cat(3,X,Y,Z);
            
            % rotate a reference axis
            switch s
                case 'sin'
                    ref_axis = [0;1;0]; % points 'east'?
                case 'cos'            
                    ref_axis = [1;0;0]; % points 'north'
            end            
            ref_axis = R * ref_axis;
            
            % make the beam pattern using a dot product
            temp = repmat(ref_axis, 1, size(XYZ,1), size(XYZ,2));
            temp = permute(temp, [2 3 1]);
            P = dot(XYZ, temp, 3);

            
            
        case "QUATERNION_CLOUD"
            % make a quaternion to represent the rotation
            q = quatFromEuler(y_,p_,r_);
            
            % put the response in cartesian
            % well, not the response, really the 'sampling sphere'
            [X Y Z] = sph2cart(AZ,DE,ones(size(AZ)));
            XYZ = cat(3,X,Y,Z);
            
            % rotate the sampling point cloud
            XYZ = quatRotByQuat( XYZ, quatInv(q) );
            
            
            % convert back to spherical
            [AZp, DEp, ~] = cart2sph( XYZ(:,:,1), XYZ(:,:,2), XYZ(:,:,3) );
            
            
            % need a different source function for each one now, no more
            % reference axis stuff here
            switch s
                case 'sin'
                    P = sin(AZp) .* cos(-DEp)  ;
                case 'cos'            
                    P = cos(AZp) .* cos(-DEp)  ;
            end

            
            
    end
    
    
    % fix null depth and make sure we keep the correct sign for P (in phase
    % north for cosine, in phase east for sine)
    % 1e-13 ensures no value is 0
    Kn = 10^(-30/10);   % worst null depth
    Ks = 1 - Kn;        % "gain"
    P_sign = sign(P+1e-13);  
    P = (Kn + Ks * abs(P.^2)) .* P_sign;
    P_dB = 10*log10(abs(P)) + -10*log10(Kn);    % conver to dB, shift to make sure there are no negative values


    % convert the response, in dB, into a manifold (right?) in cartesian
    % coordaintes, cuase this is easier to plot with mesh
    [X Y Z] = sph2cart(AZ,DE,P_dB);
    
    figure;
    subplot(141);   mesh(X,Y,Z, P);
                    daspect([1 1 1]);
                    title(' response in dB');
                    xlabel('x')
                    ylabel('y')
                    
    subplot(142);   polarplot( AZ(:,1), abs(P(:,91))' );
                    title( 'az plot intensity linear (0 is north)' );
                    ax = gca;
                    ax.ThetaZeroLocation = 'top';
                    ax.ThetaDir = 'clockwise';
                    
    subplot(143);   polarplot( DE(1,:), abs(P(181,:)) );
                    title( 'de plot intensity linear sliced at 180 (-90..90)' );
                    ax = gca;
                    
    subplot(144);   polarplot( DE(1,:), abs(P(91,:)) );
                    title( 'de plot intensity linear sliced at 90  (-90..90)' );
                    ax = gca;

    
    % numerical directivty
    % 
    DI_c = 10*log10(  4*pi*max(max(abs(P))) ./ sum(sum(dr*dr*cos(DE).*abs(P)))  );  % max P should always be 1
    disp(['DI calculated is ' num2str(DI_c)]);    
    
    % analystic solution for directivity
    DI_a = 10.0*log10( 3.0 / ( 3.0*Kn^2 + Ks^2 ) ) ;
    disp(['DI analytic is ' num2str(DI_a)]);
    
    
    


end
