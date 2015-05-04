%%
% envelope_basic.m - analyze results of envelope_basic test
%
clear all; close all
envelopes = load_envelopes('envelope_basic.nc');

% plot envelopes as a function of time

time =  envelopes.travel_time ;
data =  squeeze( envelopes.intensity(1,1,1,:,:) );
plot( time, data ) ;
grid
xlabel('Travel Time (sec)');
ylabel('Intensity (dB)');
set(gca,'YLim',[-63 -53]);

% compare envelop peak to theory
% hardcode values from envelope_basic

energy = [ 
    -42.0182 -41.8649 -41.7282
    -48.0388 -47.8855 -47.7488 ] ;
duration = [
    0.5 0.5 0.5
    0.5 0.5 0.5 ] ;
arrival = [
    10 10 10
    30 30 30 ] ;
f = 0:2;
en = 0.2 ;
l2 = 400 + 10 * f ;
w2 = 100 + 10 * f ;
scatt = 0.1 + 0.01 * f;
theory = 10*log10( pi * en.^2 .* scatt ./ sqrt(l2.*w2) ) 

coeff = 4.0 * pi * sqrt(2*pi) ;
peak = theory - 10*log10( coeff * duration(1,:) ) ;
hold on;
plot(arrival(1,:)+duration(1,:),peak,'ko');
hold off;
