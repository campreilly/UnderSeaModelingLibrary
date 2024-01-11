% clear all ;
close all ;

check_paths = 0 ;
monostatic = 1 ;

if monostatic
    bottom_verbs = load_eigenverbs('monostatic_eigenverbs_bottom.nc') ;
    surface_verbs = load_eigenverbs('monostatic_eigenverbs_surface.nc') ;
    demo = load('eigenverb_demo.mat') ;
    classic = load('classic_reverb.mat') ;
    cass = load('cass_reverb.mat') ;
    angles = demo.angle1 * 180/pi ;
    tl = 10*log10(demo.loss1) ;
    travel_time = demo.path_length / 1500.0 ;
    loss = 10*log10(abs(reflection(pi/2-(71.878*pi/180),1.9,1.1,0.8))) ;

    mono = load_envelopes('monostatic_envelopes.nc') ;
    mono_time = mono(1).travel_time ;
    mono_inten = 200.0 + 10*log10( mono(1).intensity ) ;
%     mono_inten = mono(3).intensity ;%+ mono(3).intensity ;
%     for i=2:2
%         mono_inten = mono_inten + mono(i).intensity ;
%     end
%     mono_inten = 200.0 + 10*log10( mono_inten ) ;
    figure ;
    plot( demo.time2, demo.total_reverb, '-', ...
          cass.time2, cass.total_reverb, '--', ...
          mono_time, mono_inten, '-', ...
          classic.time2, classic.total_reverb, 'k:', 'linewidth', 2 ) ;
    xlabel('Time (s)') ;
    ylabel('Reverberation Level (dB)') ;
    legend('Eigenverb Matlab','CASS',...
            'Eigenverb C++','classic','location','northeast')
    set(gca, 'ylim', [55 135], 'xlim', [0 7]) ;
    grid on ;
    
    long_matlab_tl = interp1(demo.time2, demo.total_reverb, classic.time2) ;
    long_mono_tl = interp1(mono_time, mono_inten, classic.time2) ;
    long_cass_tl = interp1(cass.time2, cass.total_reverb, classic.time2) ;
    
    matlab_diff = long_matlab_tl - classic.total_reverb ;
    cass_diff = long_cass_tl - classic.total_reverb ;
    cpp_diff = long_mono_tl - long_matlab_tl ;
    m = find( (matlab_diff > -100) & ~isnan(matlab_diff) ) ;
    fprintf('matlab mean difference from classic: %.5f\n', mean(matlab_diff(m))) ;
    m = find( (cass_diff > -100) & ~isnan(cass_diff) ) ;
    fprintf('cass mean difference from classic: %.5f\n', mean(cass_diff(m))) ;
    m = find( (cpp_diff > -100) & ~isnan(cpp_diff) ) ;
    fprintf('cpp mean difference from matlab: %.5f\n', mean(cpp_diff(m))) ;
    
%     figure ;
%     plot( classic.time2, matlab_diff, '-', ...
%           classic.time2, cass_diff, '--', ...
%           classic.time2, cpp_diff, '-', 'linewidth', 2 ) ;
%     xlabel('Time (s)') ;
%     ylabel('Reverberation Difference (dB)') ;
%     legend('Eigenverb Matlab','CASS','Eigenverb C++','location','northeast') ;
%     set(gca, 'xlim', [0 7], 'ylim', [-4 4]) ;
%     grid on ;
    
else
    [bi, desc] = xlsread('bistatic.csv') ;
    [cass_bi, cass_desc] = xlsread('cass_bistatic.xls') ;

    figure ;
    plot( bi(:,1), bi(:,2), 'r-', ...
          cass_bi(:,1), cass_bi(:,6), 'b--' ) ;
    xlabel('Time (s)') ;
    ylabel('Reverberation Level (dB)') ;
    legend('eigenverb\_bistatic','cass','location','northeast')
    set(gca, 'ylim', [100 175], 'xlim', [0 10]) ;
    grid on ;
end

if check_paths
    size = length(bottom_verbs) ;
    surf = zeros(size,1) ;
    bot = zeros(size,1) ;
    inten = zeros(size,1) ;
    for i=1:size
        surf(i) = bottom_verbs(i).surface ;
        bot(i) = bottom_verbs(i).bottom ;
        inten(i) = bottom_verbs(i).intensity ;
    end
    n = find( surf == 0 & bot == 0 ) ;
    path1 = bottom_verbs(n) ;
    n = find( surf == 1 & bot == 1 ) ;
    path3 = bottom_verbs(n) ;
    n = find( surf == 2 & bot == 2 ) ;
    path5 = bottom_verbs(n) ;
    T0 = 0.25 ;             % pulse length
    time2 = 0:0.1:7.0 ;     % reverb time
    SL = 200.0 ;            % source level
    speed = 1500 ;
    reverb_path11 = compute_reverberation(path1,path1,time2,T0,1,1) ;
    reverb_path13 = compute_reverberation(path1,path3,time2,T0,1,3) ;
    reverb_path15 = compute_reverberation(path1,path5,time2,T0,1,5) ;
    reverb_path31 = compute_reverberation(path3,path1,time2,T0,3,1) ;
    reverb_path33 = compute_reverberation(path3,path3,time2,T0,3,3) ;
    reverb_path35 = compute_reverberation(path3,path5,time2,T0,3,5) ;
    reverb_path51 = compute_reverberation(path5,path1,time2,T0,5,1) ;
    reverb_path53 = compute_reverberation(path5,path3,time2,T0,5,3) ;
    reverb_path55 = compute_reverberation(path5,path5,time2,T0,5,5) ;
    reverb_total = reverb_path11 + reverb_path13 + reverb_path15 + ...
                   reverb_path31 + reverb_path33 + reverb_path35 + ...
                   reverb_path51 + reverb_path53 + reverb_path55 ;
    reverb_total = 10*log10(reverb_total) ;
    type = {'total' 's1r1' 's1r3' 's1r5' 's3r3' 's3r5' 's5r5'} ;

    figure ;
    h = plot( time2, SL+reverb_total, 'kd', ...
              time2, SL+10*log10(reverb_path11), 's', ...
              time2, SL+10*log10(reverb_path13), 's', ...
              time2, SL+10*log10(reverb_path15), 's', ...
              time2, SL+10*log10(reverb_path33), 's', ...
              time2, SL+10*log10(reverb_path35), 's', ...
              time2, SL+10*log10(reverb_path55), 's' ) ;
    xlabel('Time (s)') ;
    ylabel('Reverberation Level (dB)') ;
    title('C++ implementation (shape), Matlab implementation (line)')
    for i=1:length(h)
        set(h(i), 'linewidth', 2) ;
    end
    set(gca, 'ylim', [55 135], 'xlim', [0 7]) ;
    legend(type, 'location', 'northeast') ;
    grid on ;
    m = [ 1 2 3 5 6 9 ] ;   % unique entries
    hold on ;
    plot( demo.time2, demo.reverb(m,:), '-', demo.time2, demo.total_reverb, '-k', 'linewidth', 1.0) ;
    hold off ;

%     path_size = length(path1) ;
%     dist1 = zeros(path_size,1) ;
%     inten1 = zeros(path_size,1) ;
%     dist3 = zeros(path_size,1) ;
%     inten3 = zeros(path_size,1) ;
%     dist5 = zeros(path_size,1) ;
%     inten5 = zeros(path_size,1) ;
%     for i=1:path_size
%         dist1(i) = speed*path1(i).travel_time ;
%         inten1(i) = 10.0*log10(path1(i).intensity) ;
%         dist3(i) = speed*path3(i).travel_time ;
%         inten3(i) = 10.0*log10(path3(i).intensity) ;
%         dist5(i) = speed*path5(i).travel_time ;
%         inten5(i) = 10.0*log10(path5(i).intensity) ;
%     end
%     figure ;
%     plot( 1:3000, -20*log10(1:3000), 'k-', ...
%           dist1, inten1, '-', ...
%           dist3, inten3, '-', ...
%           dist5, inten5, '-' ) ;
%     xlabel('Range (m)') ;
%     ylabel('Transmission Loss (dB)') ;
%     set(gca, 'ylim', [-80 -30], 'xlim', [0 3000]) ;
%     grid on ;
end