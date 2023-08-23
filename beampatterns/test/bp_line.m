% all in;uts given in degrees except hla_or_vla
% which should be a string
function [] = bp_line(yawd, pitchd, rolld, steerd, hla_or_vla)


switch hla_or_vla
    case 'hla'
        ref_axis = [1,0,0]';    %north
        
    case 'vla'
        ref_axis = [0,0,1]';    % up
end

% sensor params
steer = deg2rad(steerd);  % azmuthal steering angle
c = 1500;
f = 900;            %hz
N = 5;              % num phones in array
lambda = c/f;
d = (c/1000)/2;       % spacoing in meters

% make mesh grids to plot over
dr = deg2rad(1);
de_gv = deg2rad([-90:1:90]);
az_gv = deg2rad([0:1:360]); 
[DE, AZ, R] = meshgrid(de_gv,az_gv,1);

% Get AZ and DE in cartesian. 
[X Y Z] = sph2cart(AZ,DE,ones(size(AZ)));
XYZ = cat(3,X,Y,Z);

% rotate ref axis into world coordinates
% currently the c version does the opposite, rotating the arrival into the
% local system
ref_axis = rotate_vector_l2g(yawd,pitchd,rolld, ref_axis);

% make the beam pattern using a dot product
temp = repmat(ref_axis, 1, size(XYZ,1), size(XYZ,2));
temp = permute(temp, [2 3 1]);
comp = dot(XYZ, temp, 3);

switch hla_or_vla
    case 'hla'
        P = (sin(N*pi*f*d/c*(comp - cos(steer))) + 1e-200) ./ ...
            (N*sin(pi*f*d/c*(comp - cos(steer))) + 1e-200) ;
        
    case 'vla'
        P = (sin(N*pi*f*d/c*(comp - sin(steer))) + 1e-200) ./ ...
            (N*sin(pi*f*d/c*(comp - sin(steer))) + 1e-200) ;
end

P = P.^2;

mesh_bp([],[],P);

% numerical directivty
% 
DI_c = 10*log10(  4*pi*max(max(abs(P))) ./ sum(sum(dr*dr*cos(DE).*abs(P)))  );  % max P should always be 1
disp(['DI calculated is ' num2str(DI_c)]); 

% analystic solution for directivity
DI_a = 10.0*log10( 2*N/lambda ) ;
disp(['DI analytic is ' num2str(DI_a)]);