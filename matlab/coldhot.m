%COLDHOT    Hue-saturation-value color map
%   COLDHOT(M) returns an M-by-3 matrix containing an COLDHOT colormap.
%   COLDHOT, by itself, is the same length as the current figure's
%   colormap. If no figure exists, MATLAB creates one.
%
%   The COLDHOT colormap consists of black, violet, blue, green, red,
%   yellow, and white.  The color order is designed to provide more 
%   contrast between regions that the standard JET colormap.
%
%   For example, to reset the colormap of the current figure:
%
%             colormap(COLDHOT)
%
%   See also GRAY, HOT, COOL, BONE, COPPER, PINK, FLAG, PRISM, JET,
%   COLORMAP, RGBPLOT, HSV2RGB, RGB2HSV.
function map = coldhot(m)

if nargin < 1, m = size(get(gcf,'colormap'),1); end
h = (0:m-1)'/max(m,1);
if isempty(h)
    map = [];
else
    color = [
        0.0 0.0 0.0   % black
        1.0 0.0 1.0   % violet
        0.0 0.0 1.0   % blue
        0.0 1.0 1.0   % cyan
        0.0 1.0 0.0   % green
        1.0 0.0 0.0   % red
        1.0 1.0 0.0   % yellow
        1.0 1.0 1.0   % white
        ] ;
    [n,m]=size(color) ;
    x = (0:(n-1))'/(n-1);
    map = interp1( x, color, h ) ;
end
