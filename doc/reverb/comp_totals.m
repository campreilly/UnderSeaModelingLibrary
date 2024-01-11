
clc; clear ; close all

classic = load('classic_reverb.mat');
cass = load('cass_reverb.mat');
eigenverb = load('eigenverb_demo.mat');

% compute difference between classic and eigneverb

time2 = eigenverb.time2 ;
n = find( classic.time2 <= time2(end) ) ;
classic.time2 = classic.time2(n) ;
classic.total_reverb = classic.total_reverb(n) ;

eigenverb.diff = eigenverb.total_reverb - classic.total_reverb ;

% compute difference between classic and CASS

cass.total_reverb = interp1( cass.time2, cass.total_reverb, time2 ) ;
cass.diff = cass.total_reverb - classic.total_reverb ;

figure ;
plot( time2, eigenverb.total_reverb, ...
      time2, cass.total_reverb, '--', ...
      time2, classic.total_reverb, 'k:', ...
      'LineWidth', 2 ) ; grid
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Level (dB)');
legend({ 'Eigenverb', 'CASS', 'classic' },'Location','NorthEast');
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[55 135]);

figure;
plot( time2, eigenverb.diff, ...
      time2, cass.diff, '--', ...
      'LineWidth', 2 ) ; grid
xlabel('Two Way Travel Time (sec)');
ylabel('Reverberation Difference (dB)');
legend({ 'Eigenverb', 'CASS' },'Location','NorthEast');
set(gca,'Xlim',[0 7]);
set(gca,'Ylim',[-4 4]);

n = find( time2 > 2.0 ) ;
fprintf('average eigenverb diff = %.1f\n', mean(eigenverb.diff(n)) );
fprintf('average cass diff = %.1f\n', mean(cass.diff(n)) );
