% all in;uts given in degrees
function [] = bp_planar(yawd, pitchd, rolld, steerd_de, steerd_az,baffled)

ref_axis1 = [0,0,1]';    % up
ref_axis2 = [0,1,0]';    % east

% sensor params
steer_de = deg2rad(steerd_de);  % azmuthal steering angle
steer_az = deg2rad(steerd_az);  % azmuthal steering angle
c = 1500;
f = 900;            %hz
du = (c/1000)/2;       % spacoing in meters
de = du;
Nu = 5;
Ne = 7;

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
ref_axis1 = rotate_vector_l2g(yawd,pitchd,rolld, ref_axis1);
ref_axis2 = rotate_vector_l2g(yawd,pitchd,rolld, ref_axis2);

% make the beam pattern using a dot product
temp = repmat(ref_axis1, 1, size(XYZ,1), size(XYZ,2));
temp = permute(temp, [2 3 1]);
comp1 = dot(XYZ, temp, 3);

temp = repmat(ref_axis2, 1, size(XYZ,1), size(XYZ,2));
temp = permute(temp, [2 3 1]);
comp2 = dot(XYZ, temp, 3);

P1 = (sin(Nu*pi*f*du/c*(comp1 - sin(steer_de))) + 1e-200) ./ ...
     (Nu*sin(pi*f*du/c*(comp1 - sin(steer_de))) + 1e-200) ;
P2 = (sin(Ne*pi*f*de/c*(comp2 - sin(steer_az))) + 1e-200) ./ ...
     (Ne*sin(pi*f*de/c*(comp2 - sin(steer_az))) + 1e-200) ;
P = P1.^2.*P2.^2;

if baffled
    n = cos(AZ) <= 0.0 ;
    P(n) = 0.0 ;
end

mesh_bp([],[],P);

% numerical directivty
% 
DI_c = 10*log10(  4*pi*max(max(abs(P))) ./ sum(sum(dr*dr*cos(DE).*abs(P)))  );  % max P should always be 1
%DI2=-10*log10( (c/f)^2*3/(2*pi^2*du*de*sqrt((Nu^2-1)*(Ne^2-1))) );
DI2=-10*log10( (c/f)^2/(2*pi^2/3*du*de*sqrt((Nu^2-1)*(Ne^2-1))) );
if baffled
    DI2 = DI2 + 3 ;
end
disp(['DI calculated is ' num2str(DI_c) ' estimate is ' num2str(DI2)]); 
