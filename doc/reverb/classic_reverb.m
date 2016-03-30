%%
% classic_reverb.m
%
% Compute bottom reverberation using the classic method of collecting 
% ensonified areas from isochrons.  Uses a Lambert's Law scattering 
% strength with a coefficient of -27 dB.
%
clc; clear ; close all
debug = 0 ;

speed = 1500.0 ;        % speed of sound in water
bottom_speed = 1.10 ;   % speed of sound in bottom
bottom_density = 1.9 ;  % density of bottom
bottom_atten = 0.8 ;    % attenuation in the bottom
depth = 200.0 ;         % water depth in meters

maxRange = 3000 ;       % maximum plot range
maxTime = 7.0 ;         % maximum reverb time

dphi = 2*pi ;           % angular extent in radians
dtime = 0.001 ;         % time step in seconds
T = 250 ;               % pulse duration (in samples)
bss = 10^(-27/10);      % scattering strength
SL = 200 ;              % source level (dB)
path = 1:2:5 ;          % number of path segments
[range,path] = meshgrid( 0:1:6000, path ) ;
header = sprintf('depth=%.0f m  SL=%d dB  T=%.0f ms',depth,SL,T*dtime*1000);

% plot the model for bottom loss

angle = 0:90;
bl1 = -20*log10( abs( ...
    reflection( pi/2-angle*pi/180, bottom_density, bottom_speed, 0.0 ))) ;
bl2 = -20*log10( abs( ...
    reflection( pi/2-angle*pi/180, bottom_density, bottom_speed, bottom_atten ))) ;
if ( debug ) 
    figure ;
    plot( angle, bl1, '--', angle, bl2, 'LineWidth',2); grid
    xlabel('Grazing Angle (deg)');
    ylabel('Bottom Loss (dB)');
    legend({'no atten','atten=0.8'},'Location','NorthWest');
    title(sprintf('speed ratio = %.2f density ratio = %.2f',bottom_speed,bottom_density));
end

% compute the one-way eigenray components

path_length = path .* sqrt( depth.^2 + (range./path).^2 ) ;
angle1 = atan2( path .* depth, range ) ; % launch and grazing angle
time1 = path_length ./ speed ;          % one way trvel time
loss1 = abs(reflection(pi/2-angle1,bottom_density,bottom_speed,bottom_atten)) ...
    .^ (path-1) ./ path_length.^2 ;  % one way TL with bottom loss
    % note that the squaring of bottom loss cancels the 1/2 term in (n-1)
range = range(1,:) ;
path = path(:,1) ;
if ( debug ) 
    type = { 'n=1'; 'n=3'; 'n=5'; 'r/c' } ;

    figure ;
    plot( range', time1', range', range'/speed, 'k', 'LineWidth', 2 ) ; grid
    xlabel('Range (m)');
    ylabel('One Way Travel Time (sec)');
    set(gca,'Xlim',[0 maxRange]);
    legend(type,'Location','SouthEast');

    figure ;
    plot( range', angle1'*180/pi, 'LineWidth', 2 ) ; grid
    xlabel('Range (m)');
    ylabel('Incident Grazing Angle (deg)');
    set(gca,'Xlim',[0 maxRange]);
    set(gca,'YLim',[0,90]);
    type = { 'n=1'; 'n=3'; 'n=5' } ;
    legend(type,'Location','NorthEast');

    figure ;
    plot( range', 10*log10(loss1'), range(1,:)', -20*log10(range(1,:))', 'k', 'LineWidth', 2 ) ; grid
    xlabel('Range (m)');
    ylabel('One Way Transmission Loss (dB)');
    set(gca,'Xlim',[0 maxRange]);
    set(gca,'YLim',[-80,-30]);
    type{4} = '20logR' ;
    legend(type,'Location','NorthEast');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% use classic algorithm to compute reverberation

% compute the two-way reverberation components as function of time

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
        range2(n,:) = interp1( t, range, time2 ) ;
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
reverb = SL + 10*log10(reverb);
total_reverb = SL + 10*log10(total_reverb);
ideal_area = (speed/2).^2 * time2 * dtime * dphi ;
m = [ 1 2 3 5 6 9 ] ;   % unique entries
% m = 1:length(reverb(:,1));

save classic_reverb time2 reverb total_reverb type ...
        speed depth dphi dtime bss SL

if ( debug ) 
    figure ;
    plot( time2, 10*log10(loss2(m,:)), 'LineWidth', 2 ) ; grid
    xlabel('Two Way Travel Time (sec)');
    ylabel('Two Way Transmission Loss (dB)');
    set(gca,'Xlim',[0 maxTime]);
    legend(type{m},'Location','NorthEast');
    title(header);

    figure ;
    plot( time2, 10*log10(area(m,:))-10*log10(ones(length(m),1)*ideal_area), 'LineWidth', 2 ) ; grid
    xlabel('Two Way Travel Time (sec)');
    ylabel('Ensonified Area - Ideal (dB)');
    set(gca,'Xlim',[0 maxTime]);
    set(gca,'Ylim',[-0.25 0]);
    legend(type{m},'Location','SouthEast');
    title(header);
end

figure ;
plot( time2, reverb(m,:), time2, total_reverb, 'k', ...
      'LineWidth', 2 ) ; grid
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Level (dB)');
legend({ type{m} 'total' },'Location','NorthEast');
title(header);
set(gca,'Xlim',[0 maxTime]);
set(gca,'Ylim',[55 135]);

% generate example of reverberation signal

if ( debug ) 
    envelop = 10.^((total_reverb)/10.0) ;
    signal = envelop .* randn(1,length(envelop)) ;
    figure ;
    h = plot( time2, signal, time2, envelop, 'r' , time2, -envelop, 'r' ) ; grid
    set(h(2),'LineWidth',2);
    set(h(3),'LineWidth',2);
    set(gca,'Xlim',[0 1.4]);
    set(gca,'Ylim',[-3e13 3e13]);
    legend({'signal';'envelope'},'Location','NorthEast');
    xlabel('Received Time (sec)');
    ylabel('Reverberation Signal (uPa)');
    title(header);
end
