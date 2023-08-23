function [] = bp_piston()

% sensor params
c = 1500;
f = 900;            %hz
lambda = c/f;
D = 4*lambda;       % piston diameter in meters

% make mesh grids to plot over
dr = deg2rad(1);
de_gv = deg2rad([-90:1:90]);
az_gv = deg2rad([0:1:360]); 
[DE, AZ, R] = meshgrid(de_gv,az_gv,1);

% Get AZ and DE in cartesian. 
[X Y Z] = sph2cart(AZ,DE,ones(size(AZ)));
XYZ = cat(3,X,Y,Z);


P = (2*besselj(1,pi*D/lambda*cos(DE)+eps)) ./ (pi*D/lambda*cos(DE)+eps);
P = P.^2;

mesh_bp([],[],P);

% numerical directivty
% 
DI_c = 10*log10(  4*pi*max(max(abs(P))) ./ sum(sum(dr*dr*cos(DE).*abs(P)))  );  % max P should always be 1
disp(['DI calculated is ' num2str(DI_c)]); 

% analystic solution for directivity
DI_a = 10.0*log10( 2*N/lambda ) ;
disp(['DI analytic is ' num2str(DI_a)]);