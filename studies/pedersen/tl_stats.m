%%
% TL_STATS  Analyzes the differences between two transmission loss models.
%
% [bias,err,detcoef,lag] = tl_stats(range1,level1,range2,level2,threshold) 
%
% The model to be tested is level1, in dB, as a function of range1. The
% model to be tested against is level2, in dB, as a function of range2.
% This implementation interpolates level1 to estimates its values on the
% range2 grid. The range of values can be easily limited by limiting the
% scope of the range2 input. This implementation also clips the minimum
% level of both models to the threshold. This clipping prevents us from
% basing most of the statistics on the differences between very quiet
% sounds.  If no threshold is provided, a default value of -100 dB is used.
%
% The bias measures the average amount that the level1 differs from level2
% in dB.  Since the difference of the mean is equal to the mean of the
% differences (dL).
%
%       bias = mean( dL ) 
%
% The deviation (dev) is a measure of the difference in flucuations of level3
% compared to level1.  Computed indirectly from the mean of dL^2.
%
%       dev = sqrt( mean( dL^2 ) - bias^2 ) 
%
% The coefficient of determination (detcoef) is a measure of the fraction
% of level2 that is predicts level1.  To allow for the fact that the best
% correlation may have a range offset, the coefficient of determination is
% computed from the maximum covariance as a function of lag.  To use
% matlab's xcov() and cov() functions for this calculation, we must make
% sure that they all use the normalization.
%
%                 [ xcov(L1,L2) / (N-1) ]^2
%       detcoef = ------------------------- * 100%
%                       cov(L1) cov(L2)
%
% The lag indicates the offset, in the same units as range2, of how far
% level1 is ahead of level2.
%
function [ bias, dev, detcoef, lag ] = tl_stats( ...
    range1, level1, range2, level2, threshold ) 

N = length(range2) ;
dr = range2(2)-range2(1) ;
if ( nargin < 5 ), threshold = -100 ; end ;

%% put both datasets on the same grid and apply threshold

range2 = range2(:) ;    
level2 = level2(:) ;
level1 = interp1( range1, level1(:), range2, 'pchip' ) ;
level1( level1 < threshold ) = threshold ;
level2( level2 < threshold ) = threshold ;
dL = level1 - level2 ;

%% compute statistics

bias = mean( dL ) ;
dev = sqrt( mean( dL.^2 ) - bias^2 ) ;
detcoef = 100.0 * xcov(level2,level1).^2 ...
        / ( cov(level2) * cov(level1) * (N-1).^2 ) ;
[detcoef,lag] = max( detcoef ) ;
lag = -(lag-N) * dr ;

end