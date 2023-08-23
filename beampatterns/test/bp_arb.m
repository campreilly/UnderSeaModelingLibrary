function [] = bp_arb(steer_az_d, steer_de_d)

% sensor params
c = 1500;
f = 900;            %hz
spacing = (c/1000)/2.0 ;
lambda = c/f;
N = 5;
p = [...
    	0.0, 0.0, -3.0*spacing;...	% uneven vertical line array
		0.0, 0.0, -1.0*spacing;...
		0.0, 0.0,  0.0*spacing;...
		0.0, 0.0,  1.0*spacing;...
		0.0, 0.0,  4.0*spacing;...
         ];
steera = pi/2;
steerd = 0;

if nargin == 2
    steera = deg2rad( steer_az_d );
    steerd = deg2rad( steer_de_d );
else
%     steera = 0;
%     steerd = 0;
end
 
% make mesh grids to plot over
dr = deg2rad(1);
de_gv = deg2rad([-90:1:90]);
az_gv = deg2rad([0:1:360]); 
[DE, AZ, R] = meshgrid(de_gv,az_gv,1);

% Get AZ and DE in cartesian. 
[X Y Z] = sph2cart(AZ,DE,ones(size(AZ)));
XYZ = cat(3,X,Y,Z);


P = zeros(size(AZ));
P_real = zeros(size(AZ));
P_imag = zeros(size(AZ));
for n=1:size(AZ,1)          % all az
    for m=1:size(AZ,2)      % all de
        for o=1:size(p,1)   % all phones
            
            % this part could be vectoriced, but i want to have a nice way
            % to test c imdplementations here
            k_diff(1) = 2*pi/lambda * (X(n,m) - cos(steera) * cos(steerd));
            k_diff(2) = 2*pi/lambda * (Y(n,m) - sin(steera) * cos(steerd));
            k_diff(3) = 2*pi/lambda * (Z(n,m) - sin(steerd));
            temp = dot( p(o,:), k_diff);
            
            % sum the response across all phones
            P_real(n,m) = P_real(n,m) + cos( temp ); 
            P_imag(n,m) = P_imag(n,m) + sin( temp ); % exp^(-j*(...
        end
    end
end

P = 1/size(p,1).^2 * (P_real.^2 + P_imag.^2);

mesh_bp([],[],P);

% numerical directivty
% 
DI_c = 10*log10(  4*pi*max(max(abs(P))) ./ sum(sum(dr*dr*cos(DE).*abs(P)))  );  % max P should always be 1
disp(['DI calculated is ' num2str(DI_c)]); 

% % analystic solution for directivity
% DI_a = 10.0*log10( 2*N/lambda ) ;
% disp(['DI analytic is ' num2str(DI_a)]);