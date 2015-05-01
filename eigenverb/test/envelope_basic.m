%%
% envelope_basic.m - analyze results of envelope_basic test
%
clear all; close all
envelopes = load_envelopes('envelope_basic.nc');
time =  envelopes.travel_time ;
data =  squeeze( envelopes.intensity(1,1,1,:,:) );
plot( time, data ) ;
grid
xlabel('Travel Time (sec)');
ylabel('Intensity (dB)');
% set(gca,'YLim',[-72 -52]);

% energy = [ 
%     -42.0182 -41.8649 -41.7282
%     -48.0388 -47.8855 -47.7488 ] ;
% duration = [
%     0.5 0.5 0.5
%     0.5 0.5 0.5 ] ;
% arrival = [
%     10 10 10
%     30 30 30 ] ;
% 
% coeff = 4.0 * pi * sqrt(2*pi) ;
% peak = energy - 10*log10( coeff * duration ) ;
% 
