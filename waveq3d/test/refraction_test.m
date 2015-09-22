% refraction_test.m - plot results of all refraction_test sub-tests
%
clear ; close all

% plot great circle paths

sheet=importdata('refraction_great_circle.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;

[N,M]=size(data);
n=2:4:M; 
m=3:4:M;

r = 0:18 ;   % distance in degrees of latitude
bearing = (30:30:90) * pi / 180 ;
x = r' * sin(bearing) - 45 ;
y = r' * cos(bearing) + 45 ;

figure ;
plot( data(:,m), data(:,n), 'k-', x, y, 'k--', 'LineWidth', 1.25 )
grid
axis([-46 -26 42 60]);
xlabel('Longitude (deg)');
ylabel('Latgitude (deg)');
print -deps refraction_great_circle.eps

% Compare the model's ray refraction accuracy to the 
% analytic solution for a linear profile. 

sheet=importdata('refraction_linear.csv',',');
data = sheet.data ;

dt = ( data(2,1) - data(1,1) ) * 1000.0 ;
R = data(1,2) + 1000 ;
range = data(:,6) / 1e3 ;
zmodel = data(:,2) - R ;
ztheory = data(:,5) ;

[p,n] = max(abs(data(:,6))) ;
fprintf('n2_linear: max error = %f meters at range = %f km\n',...
    zmodel(n)-ztheory(n),range(n)/1e3);

figure ;
h = plot( range, zmodel, 'k-', range, ztheory, 'k--', 'LineWidth', 1.5 ) ;
grid ;
ylabel('Depth (m)')
xlabel('Range (km)')
legend('Model','Theory','Location','NorthWest') ; % ,'Location','Best')
print -deps refraction_linear.eps

% Compare the model's ray refraction accuracy to the 
% analytic solution for a n^2 linear profile. 

sheet=importdata('refraction_n2_linear.csv',',');
data = sheet.data ;

dt = ( data(2,1) - data(1,1) ) * 1000.0 ;
R = data(1,2) + 1000 ;
range = data(:,6) / 1e3 ;
zmodel = data(:,2) - R ;
ztheory = data(:,5) ;

[p,n] = max(abs(data(:,6))) ;
fprintf('n2_linear: max error = %f meters at range = %f km\n',...
    zmodel(n)-ztheory(n),range(n)/1e3);

figure ;
h = plot( range, zmodel, 'k-', range, ztheory, 'k--', 'LineWidth', 1.5 ) ;
grid ;
ylabel('Depth (m)')
xlabel('Range (km)')
legend('Model','Theory','Location','NorthWest') ; % ,'Location','Best')
print -deps refraction_n2_linear.eps

% load Munk wavefront and plot in range/depth coordinates

clear
wavefront = load_wavefront('refraction_munk_range.nc') ;
az_index = 1 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) / 1e3 ; % range in meters
walt = squeeze(wavefront.altitude(:,:,az_index))  ;

figure; 
d = 0:10:5000;
z = 2/1300*(d-1300);
c = 1500 * ( 1 + 7.37e-3 * ( z - 1 + exp(-z) ) ) ;
subplot(1,4,1);
plot( c, -d, 'k-' ) ; grid
axis( [ 1500 1550 -5000 0 ] ) ;
xlabel('Speed (m/s)');
ylabel('Depth (m)');

subplot(1,4,2:4);
plot( wlat, walt, 'k-' ) ;
grid;
axis( [ 0 140 -5000 0 ] ) ;
xlabel('Range (km)');
set(gca,'YTickLabel',[])
print -deps refraction_munk_range_wave.eps

% load Munk error spreadsheet and plot errors

clear
sheet=importdata('refraction_munk_range.csv',',');
data = sheet.data ;
[N,M]=size(data);

[p,n] = max(abs(data(:,5))) ;

figure; 
n = find( data(:,2) > 0.0 ) ;
m = find( data(:,2) <= 0.0 ) ;
plot( data(n,4)/1e3, data(n,5), 'ko', data(m,4)/1e3, data(m,5), 'k*' ) ;
grid;
xlabel('Range (km)');
ylabel('Range Error (m)');
legend('Launched Up','Launched Down','Location','Best');
set(gca,'XLim',[0 140]);

data_bias = mean(data(:,5));
data_std = std(data(:,5));
[p,n] = max(abs(data(:,6))) ;
fprintf('munk: max error = %f = %f%% at range = %f km, bias=%.4f, std=%4f\n',...
    data(n,5),data(n,6),data(n,4)/1e3,data_bias,data_std);
print -deps refraction_munk_range.eps

% munk profile sensitivity to time step 
% hand jammed from multiple runs

figure;
a=[25 50 100 150 200 250 300 350 400];
b=[ 0.25 1.10 8.62 28.43 67.12 131.10 223.54 353.51 521.99 ];
c=25:400;
d=interp1(a,b,c,'pchip');
semilogy(c,d,'k-',a,b,'ko');
grid
set(gca,'XLim',[0 450]);
xlabel('Time Step (msec)');
ylabel('Range Error (m)');
print -deps refraction_munk_sensitivity.eps

%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% load Pedersen wavefront and plot in range/depth coordinates

clear
wavefront = load_wavefront('refraction_pedersen_range.nc') ;
az_index = 1 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) / 1e3 ; % range in meters
walt = squeeze(wavefront.altitude(:,:,az_index))  ;

figure; 
plot( wlat, walt, 'k-' ) ;
grid;
set(gca,'XLim',[0 3.5]);
set(gca,'YLim',[-1200 0]);
xlabel('Range (km)');
ylabel('Depth (m)');
print -deps refraction_pedersen_range_wave.eps

% load Pedersen error spreadsheet and plot errors

clear
sheet=importdata('refraction_pedersen_range.csv',',');
data = sheet.data ;
[N,M]=size(data);

[p,n] = max(abs(data(:,5))) ;

figure; 
plot( data(:,4)/1e3, data(:,5), 'ko' ) ;
grid;
xlabel('Range (km)');
ylabel('Range Error (m)');
set(gca,'XLim',[2.0 3.5]);

data_bias = mean(data(:,5));
data_std = std(data(:,5));
[p,n] = max(abs(data(:,6))) ;
fprintf('pedersen: max error = %f = %f%% at range = %f km, bias=%.4f, std=%4f\n',...
    data(n,5),data(n,6),data(n,4)/1e3,data_bias,data_std);
print -deps refraction_pedersen_range.eps

% pedersen profile sensitivity to time step 
% hand jammed from multiple runs of refraction_pedersen_range

figure;
a=[25 50 100 150 200 250 300 350 400];
b=[0.0483053 0.41326 1.29146 1.96816 5.11535 8.75183 18.46631 22.95168 526.201945 ] ;
c=25:400;
d=interp1(a,b,c,'pchip');
semilogy(c,d,'k-',a,b,'ko');
grid
set(gca,'XLim',[0 450]);
xlabel('Time Step (msec)');
ylabel('Range Error (m)');
print -deps refraction_pedersen_sensitivity.eps
