%% run_cross_slope
% Compare analystic solution for transmission loss, as a function of range
% across the slope, using a simple 3-D wedge, to an equivalent
% solution for a 2-D environment of constant depth.
% Repeat this process for small and large slope angles, at low and high
% frequencies.
%
% ref: F. B. Jensen and C. M. Ferla, "Numerical solutions of 
% range?dependent benchmark problems in ocean acoustics," 
% J. Acoust. Soc. Am., vol. 87, no. 4, pp. 1499-1510, 1990.
%
clear ; close all

cmp_cross_slope(...  % low frequency, small angle from reference
    25, ...         % frequency (Hz)
    2.86, ...       % wedge angle (deg)
    4000.0, ...     % horizontal range of source from apex (m)
    100.0, ...      % depth of source from ocean surface (m)
    100:100:70000 ) ;  % horizontal range of receiver across slopw (m)

cmp_cross_slope(...  % low frequency, wide angle from reference
    25, ...       % frequency (Hz)
    21.0, ...       % wedge angle (deg)
    520.0, ...      % horizontal range of source from apex (m)
    100.0, ...      % depth of source from ocean surface (m)
    10:10:10000 ) ;  % horizontal range of receiver across slopw (m)

cmp_cross_slope(...  % low frequency, small angle from reference
    2000, ...       % frequency (Hz)
    2.86, ...       % wedge angle (deg)
    4000.0, ...     % horizontal range of source from apex (m)
    100.0, ...      % depth of source from ocean surface (m)
    100:100:70000 ) ;  % horizontal range of receiver across slopw (m)

cmp_cross_slope(...  % low frequency, wide angle from reference
    2000, ...       % frequency (Hz)
    21.0, ...       % wedge angle (deg)
    520.0, ...      % horizontal range of source from apex (m)
    100.0, ...      % depth of source from ocean surface (m)
    10:10:10000 ) ;  % horizontal range of receiver across slopw (m)

