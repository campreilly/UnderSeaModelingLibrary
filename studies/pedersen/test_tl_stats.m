% test_tl_stats.m

clear all; 
close all;

% load FFP transmission loss from disk

data =importdata('ffp_n2shallow.csv',',');
true.range = data(:,1)'/1e3 ;
true.tl = data(:,2)' ;
clear data
dr = true.range(2) - true.range(1) ;
threshold = -80 ;

% build a test signal

test.range = true.range + 0 * dr ;
test.tl = true.tl + 1 + 1 * randn( size(test.range) ) ;

% plot signals

figure; 
plot( true.range, true.tl, test.range, test.tl ) ;
grid ;
xlabel('Range (km)');
ylabel('Transmission Loss (dB)');
set(gca,'Xlim',[0.5 1.0]);
set(gca,'Ylim',[-80 -40]);

% compare TL signals
% limit comparison to area where true TL > -80 dB 

[ bias, dev, detcoef ] = tl_stats( ...
    test.range, test.tl, true.range, true.tl, threshold ) ;
fprintf('bias = %.2f dB dev = %.2f dB detcoef = %.1f%%\n',...
    bias, dev, detcoef ) ;
