% width_test.m - tests the normalization of Gaussian beams
%
% Builds a series of adjacent boxes, each of which has a
% constant "energy" spread over a changing set of "width" values.
% Approximate each box using a Gaussian beam of the same width and
% energy.  Sum the the beams across all values of "x" and see if
% the peak levels approximate those of the boxes.
%
% Gaussian beams can be made smoother by spreading the energy
% over a wider area with the same total energy.  Allow the user
% to select an overlap ratio to tune this smoothing.
%
clear all ; close all ;
fprintf('\n*** Tests Gaussian Beam Normalization ***\n\n') ;

% full width of each box
width = [ 2 6 9 5 7 2 9 5 5 5 5 7 1 8 7 2 5 3 5 4 4 2 3 ] ; 
% width = [ 10 10 10 10 10 10 10 10 10 10 ] ;

overlap = 1.1 ;             % gaussian width / box width 
s = input('overlap as gaussian width / box width (default=1.0): ');
if ( ~ isempty(s) ) overlap = s ; end

energy = 10.0 ;             % total energy in each box
N = length(width) ;         % total number of boxes
boundary = cumsum(width);   % defines the edge of each box

x=0:0.1:(boundary(length(boundary))-1); % values to plot
x0=boundary-(width/2) ;                 % center of each box

% define a series of boxes at these widthhs for comparison

boxcar = ones(1,length(x)) * energy / width(N) ;
for n=(N-1):-1:1
    m = find( x < boundary(n) ) ;
    boxcar(m) = ones(1,length(m)) * energy / width(n) ;
end

% define an equivalent Gaussian beam for each box
% sum the Gaussian beams across boxes

gaussian = zeros(1,length(x)) ;
for n=1:N
    w = overlap * 0.5 * width(n) ;      % beam half width at 1/2 max
    gaussian = gaussian ...
        + energy * exp( -(x-x0(n)).^2 / (2*w*w) ) / (w*sqrt(2*pi)) ;
end

plot( x, boxcar, x, gaussian ) ;
