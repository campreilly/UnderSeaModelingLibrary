% Compare the eigenrays generate by GRAB and the WaveQ3D model
% for Pedersen deep source test.

clear all
close all

% load eigenray data from WaveQ3D model for each D/E increment

loss_tan = load_proploss('pedersen_deep_sensitivity_tan.nc');
loss_1000 = load_proploss('pedersen_deep_sensitivity_1000.nc');
loss_0500 = load_proploss('pedersen_deep_sensitivity_0500.nc');
loss_0250 = load_proploss('pedersen_deep_sensitivity_0250.nc');
loss_0125 = load_proploss('pedersen_deep_sensitivity_0125.nc');
range = (3000:0.25:3120)/1e3 ;
xscale = [range(1) range(end)] ;

% compare propagation loss data to FFP

sheet=importdata('ffp_n2deep.csv',',');
data = sheet ;
ffp = interp1( data(:,1)'/1e3, data(:,2)', range )' ;
n = find( ~isnan(ffp) ) ;
rms = [
    sqrt( mean( (ffp(n)+loss_tan.intensity(n)).^2 ) ) 
    sqrt( mean( (ffp(n)+loss_1000.intensity(n)).^2 ) ) 
    sqrt( mean( (ffp(n)+loss_0500.intensity(n)).^2 ) ) 
    sqrt( mean( (ffp(n)+loss_0250.intensity(n)).^2 ) ) 
    sqrt( mean( (ffp(n)+loss_0125.intensity(n)).^2 ) ) ] ;
a = corrcoef( ffp(n), loss_tan.intensity(n) ) ; cross(1,1) = a(1,2).^2 ;
a = corrcoef( ffp(n), loss_1000.intensity(n) ) ; cross(2,1) = a(1,2).^2 ;
a = corrcoef( ffp(n), loss_0500.intensity(n) ) ; cross(3,1) = a(1,2).^2 ;
a = corrcoef( ffp(n), loss_0250.intensity(n) ) ; cross(4,1) = a(1,2).^2 ;
a = corrcoef( ffp(n), loss_0125.intensity(n) ) ; cross(5,1) = a(1,2).^2 ;
table = [ rms cross*100 ] 
clear data

% compare coherent TL estimates for all D/E increments

figure ;
plot( range, ffp, '-', ...
      range, -loss_tan.intensity, '-', ...
      range, -loss_1000.intensity, '-', ...
      range, -loss_0500.intensity, '-', ...
      range, -loss_0250.intensity, '-', ...
      range, -loss_0125.intensity, '-', ...
      'LineWidth', 1.5 ) ;
grid ;
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
legend('theory','tan spaced','0.1 deg','0.05 deg','0.025 deg','0.0125 deg','Location','SouthWest');
set(gca,'XLim',xscale)
set(gca,'YLim',[-100 -40])
% print -deps pedersen_deep_sensitivity.eps
