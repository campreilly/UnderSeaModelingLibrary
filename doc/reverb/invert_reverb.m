%%
% invert_reverb.m
%
% Compute the plots used to illustrate range/time inversion in the report.
% Specialized from the classic_reverb.m script.
%
clc; clear ; close all

speed = 1500.0 ;        % speed of sound in water
bottom_speed = 1.10 ;   % speed of sound in bottom
bottom_density = 1.9 ;  % density of bottom
bottom_atten = 0.8 ;    % attenuation in the bottom
depth = 200.0 ;         % water depth in meters

maxRange = 1000 ;       % maximum plot range

dphi = 2*pi ;           % angular extent in radians
dtime = 0.001 ;         % time step in seconds
T = 100 ;               % pulse duration (in samples)
bss = 10^(-27/10);      % scattering strength
SL = 200 ;              % source level (dB)
path = 1:2:5 ;          % number of path segments
[range,path] = meshgrid( 0:50:3000, path ) ;

% compute the one-way eigenray components

path_length = path .* sqrt( depth.^2 + (range./path).^2 ) ;
angle1 = atan2( path .* depth, range ) ; % launch and grazing angle
time1 = path_length ./ speed ;          % one way trvel time
loss1 = abs(reflection(pi/2-angle1,bottom_density,bottom_speed,bottom_atten)) ...
    .^ (path-1) ./ path_length.^2 ;  % one way TL with bottom loss
    % note that the squaring of bottom loss cancels the 1/2 term in (n-1)
range = range(1,:) ;
path = path(:,1) ;
type = { 'n=1'; 'n=3'; 'n=5'; 'r/c' } ;

figure ;
h=plot( range, time1, 'o-', range, range/speed, 'k', 'LineWidth', 2 ) ; grid
for n=1:3, set(h(n),'MarkerFaceColor',get(h(n),'Color')); end;
xlabel('Range (m)');
ylabel('One Way Travel Time (sec)');
set(gca,'Xlim',[0 maxRange]);

% compute the two-way reverberation components as function of time

dtime = 0.1 ;
time2 = 0:dtime:2*max(time1(:));
range2 = NaN*ones( length(path)^2, length(time2) ) ;
loss2 = range2 ;
angleI = range2 ;
angleS = range2 ;
area = range2 ;
reverb = 1e-20 * ones(size(range2)) ;
total_reverb = 1e-20 * ones(size(time2)) ;
type = cell( length(path)^2, 1 ) ;

n = 0 ;
for s=1:length(path)
    for r=1:length(path)
        n = n + 1 ;
        t = time1(s,:) + time1(r,:)  ;
        range2(n,:) = interp1( t, range, time2, 'linear','extrap' ) ;
        loss2(n,:) = interp1( t, loss1(s,:) .* loss1(r,:), time2 ) ;
        angleI(n,:) = interp1( t, angle1(s,:), time2 ) ;
        angleS(n,:) = interp1( t, angle1(r,:), time2 ) ;
        type{n} = sprintf('s%dr%d',path(s),path(r));

        % compute ensonified area as A = r dr dphi
        
        m = find( ~isnan(range2(n,:)) ) ;
        area(n,m) = range2(n,m) .* [ diff(range2(n,m)) NaN ] .* dphi ;
        
        % compute reverb as RL = TL1 + TL2 + 10*log( BSS * A )
        
        scattering = bss .* sin(angleI(n,m)) .* sin(angleS(n,m));
        reverb(n,m) = loss2(n,m) .* scattering .* area(n,m) ;
        reverb(n,:) = filter(ones(1,T), 1, reverb(n,:) ) ;
        total_reverb(1,m) = total_reverb(1,m) + 2 * reverb(n,m) ;
    end
end
reverb = 10*log10(reverb);
total_reverb = 10*log10(total_reverb);
ideal_area = (speed/2).^2 * time2 * dtime * dphi ;
m = [ 1 2 3 5 6 9 ] ;   % unique entries

save classic_reverb time2 range2 loss2 area ideal_area total_reverb ...
        speed depth dphi dtime bss SL

figure ;
h=plot( time2, range2(m,:), '-o', time2, speed*time2/2, 'k', 'LineWidth', 2 ) ; grid
xlabel('Two Way Travel Time (sec)');
ylabel('Range (m)');
set(gca,'Xlim',[0 2.5])
set(gca,'Ylim',[0 2000])
for n=1:length(m), set(h(n),'MarkerFaceColor',get(h(n),'Color')); end;
