%%
% eigenverb_demo.m
%
% Compute bottom reverberation using the Eigenverb model.  
% Parallels the case implemented in the classic_reverb.m script.
% In the classic algorithm, ranges are evenly gridded.  But in this one,
% the ranges are determined by the launch angle of the ray.
% Rays are launched from a tangent spaced ray fan, like they would
% be in most WaveQ3D scenarios.
%
clc; clear ; close all

speed = 1500.0 ;        % speed of sound in water
bottom_speed = 1.10 ;   % speed of sound in bottom
bottom_density = 1.9 ;  % density of bottom
bottom_atten = 0.8 ;    % attenuation in the bottom
depth = 200.0 ;         % water depth in meters

linear = 0 ;            % use linear fan if = 1, tan fan if ~= 1
num_rays = 91 ;         % number of rays to create
maxRange = 3000 ;       % maximum plot range
maxTime = 7.0 ;         % maximum reverb time

dphi = 2*pi ;           % azimuthal extent in radians
dtime = 0.001 ;         % time step in seconds
T0 = 0.25 ;              % pulse duration (in seconds)
bss = 10^(-27/10);      % scattering strength
SL = 10.^(200/10) ;     % source level (linear)
path = 1:2:9 ;          % number of path segments
header = sprintf('depth=%.0f m  SL=%d dB  T=%.0f ms',depth,10*log10(SL),T0*1000);

% find the ranges appropriate to each ray in a tangent spaced ray fan
% Using same algorithm as seg_rayfan.h, but limit to downward facing rays

if ( linear )           % use linear ray spacing
    angle1 = (1:-1/(num_rays-1):0)*pi/2 ; 
else                    % use tangent ray spacing (like seq_rayfan.h)
    spread  = 6.0 ;     % controls spacing of rays
    first_ang = atan( -90/spread ) ;
    last_ang = atan( 0/spread ) ;
    scale = (last_ang - first_ang) / (num_rays - 1) ;
    n=1:num_rays ;
    x = first_ang + scale * (n-1) ;
    angle1 = -tan(x)*spread*pi/180 ;  % D/E angle with positive down
end
% angle1 = cat(2, angle1, angle1(:,end-1:-1:1)) ;    % increase the number of rays to be equivalent to a rayfan de, 181
num_paths = length(angle1) ;
[angle1,path] = meshgrid( angle1, path ) ;
range = depth .* cot(angle1) .* path ;

% compute the one-way eigenray components
% in the same format used for classic_reverb.m

path_length = path .* sqrt( depth.^2 + (range./path).^2 ) ;
time1 = path_length ./ speed ;          % one way travel time
loss1 = abs(reflection(pi/2-angle1,bottom_density,bottom_speed,bottom_atten)) ...
    .^ (path-1) ./ path_length.^2 ;     % one way TL with bottom loss
    % note that the squaring of bottom loss cancels the 1/2 term in (n-1)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% use Eigenverb algorithm to compute reverberation

% compute beam width in D/E using half distance to prev/next rays

mu_half = 0.5 * ( angle1(:,1:end-1) + angle1(:,2:end) ) ;
mu_diff = -[ 2*(mu_half(:,1)-pi/2), diff(mu_half')', -2*mu_half(:,end) ] ;

% compute length and width terms for each ensonified patch

% SL = SL / 2 ;
L = path_length .* mu_diff ./ sin(angle1) ;
L2 = L .* L ;
W = range .* dphi ;
W2 = W .* W ;

% loop over source and receiver path types

path = path(:,1)' ;                 % un-do meshgrid for path types
time2 = 0:dtime:maxTime ;           % two-way travel time
reverb = 1e-20*ones( length(path)^2, length(time2) ) ; % empty reverb
type = cell( length(path)^2, 1 ) ;  % plot legend
n = 0 ;                             % index number for each contribution
for s=1:length(path)
    for r=1:length(path)
        n = n + 1 ;
        type{n} = sprintf('s%dr%d',path(s),path(r)) ;
        disp(type{n});
        
        % loop over the patches in each path type
        
        for sp=2:(num_paths-1)
            for rp=2:(num_paths-1)
                
                % compute integrated intensity for this path type combo
                
                scatter = bss .* sin(angle1(s,sp)) .* sin(angle1(r,rp)) ;
                S2 = L2(s,sp) + L2(r,rp) ;  % sum of covariances
                area = 0.5 * L(s,sp) * L(r,rp) * W(s,sp) * W(r,rp) ...
                     / sqrt( S2 * ( W2(s,sp) + W2(r,rp)) ) ...
                    * exp( -0.5*( range(s,sp) - range(r,rp) ).^2 / S2 ) ;
                Esr = SL * T0 * loss1(s,sp) * loss1(r,rp) * scatter * area ;
                
                % only add to result if peak intensity is significant
                
                if ( Esr > 1e2 )
                    Lp = sqrt( 1 / (1/L2(s,sp)+1/L2(r,rp) ) ) ;
                    Tarea = Lp * sin(angle1(r,rp)) / speed ;
                    Tsr = sqrt( T0*T0 + Tarea*Tarea ) / 2 ;
                    t0 = time1(s,sp) + time1(r,rp) + Tsr ;
                    contrib = Esr * exp(-0.5*((time2-t0)/Tsr).^2) ...
                        / ( Tsr * sqrt(2*pi) ) ;
                    reverb(n,:) = reverb(n,:) + contrib ;
                end
            end
        end
    end
end

% compute total reverberation in dB

total_reverb = 10*log10( 2.0*sum(reverb) ) ;
cumulative_total_reverb = 10*log10( cumsum(2.0*reverb,1) ) ;
reverb = 10*log10( reverb ) ;

figure ;
plot( time2, cumulative_total_reverb, 'LineWidth', 2 ) ;
grid on ;
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Level (dB)');
% legend({ type{m} 'total' },'Location','NorthEast');
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[-2 2]);
title(header);
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[55 135]);

save eigenverb_demo time2 reverb total_reverb type loss1 mu_half mu_diff ...
        angle1 range path_length L W speed depth dphi dtime bss SL

% load classic result and trim to use the same times as Eigenverb

classic = load('classic_reverb.mat');
n = find( classic.time2 <= time2(end) ) ;
classic.time2 = classic.time2(n) ;
classic.reverb = classic.reverb(:,n) ;
classic.total_reverb = classic.total_reverb(n) ;

eigenverb.total_reverb = total_reverb ;
eigenverb.reverb = reverb ;
eigenverb.time2 = time2 ;

% plot results side by side

m = [ 1 2 3 5 6 9 ] ;   % unique entries

figure;
plot( eigenverb.time2, eigenverb.reverb(m,:), '-', eigenverb.time2, eigenverb.total_reverb, 'k-', 'LineWidth', 2 ) ; grid
grid
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Level (dB)');
legend({ type{m} 'total' },'Location','NorthEast');
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[-2 2]);
title(header);
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[55 135]);
hold on
plot( classic.time2, classic.reverb(m,:), '--', classic.time2, classic.total_reverb, 'k--', 'LineWidth', 2 ) ; grid
hold off

% compute differences and plot them

reverb = eigenverb.reverb(m,:) - classic.reverb(m,:) ;
total_reverb = eigenverb.total_reverb - classic.total_reverb ;

figure;
plot( time2, reverb, time2, total_reverb, 'k', 'LineWidth', 2 ) ; grid
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Difference (dB)');
legend({ type{m} 'total' },'Location','NorthEast');
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[-4 4]);
title(header);

