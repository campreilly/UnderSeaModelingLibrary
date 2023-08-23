%%
% bp_test.m
% Plots all beam patterns exported from bp_test program.
% This allows the developer to perform a visual inspection of the USML
% results.  There is no automated errors checking build into this test.
%
clear; close all

% Plot omni-beampatter.  Level of 1 in every direction.

mesh_bp('bp_omni.csv', [], []);  title('USML omni');

% Display solid angle beam pattern. 
% Level of 1 for 45 deg around -20 DE and 90 deg around +1- deg AZ.
 
mesh_bp('bp_solid.csv', [], []);  title('USML solid'); view(0,0);

% Display of 4 lambda piston, baffled in the back.

mesh_bp('bp_piston.csv', [], []);  title('USML piston'); view(0,0);

% Cardioid steered to -20 deg DE and +10 deg AZ
% Level of one in that direction and zero in the direction opposite.

mesh_bp('bp_cardioid.csv', [], []);  title('USML cardioid');

% Plot USML sin and cos on a single plot.
%
% When see from above, there are 4 lobes in the combination of these two
% patterns (two lobes each). The lobes overlap near their 3 dB down points.

a = mesh_bp('bp_sin.csv', [], []);
mesh_bp('bp_cos.csv', a, []);   title('USML sin/cos');

% Compare USML and Matlab versions of evenly spaced horzontal line arrays.
%
% The ine array in this test has 5 elements along the forward/back
% direction, and it is steered toward the front of the array (aka forward
% endfire).  Because the 900 Hz signal frequency is slightly less than the
% 1000 Hz design frequency of the array, the aft facing lobes are all
% smaller than their forward facing counterparts.  At the design frequency,
% the forward and aft lobes would be symetric.

mesh_bp('bp_hla.csv', [], []); title('USML hla'); view(0,0);
bp_line(0.0,0.0,0.0,0.0,'hla'); title('matlab hla'); view(0,0);

% Compare USML and Matlab versions of evenly spaced vertic line arrays.
%
% The ine array in this test has 5 elements along the up/down direction,
% and it is steered down by 20 degrees. The signal frequency is 900 Hz and
% the design frequency is 1000 Hz.  The bp_multi test outputs a VLA pattern
% that should be identical to the ones for bp_vla in C++ and the b_line
% test in Matlab.

mesh_bp('bp_vla.csv', [], []);  title('USML vla'); view(0,0);
bp_line(0.0,0.0,0.0,-20.0,'vla'); title('matlab vla'); view(0,0);
mesh_bp('bp_multi.csv', [], []);  title('USML multi'); view(0,0);

% Compare USML and Matlab versions of unevenly spaced line arrays.
%
% All of these tests are performed on an unevenly spaced vertical line
% array with element locations at -3, -1, 0, +1, and +4 times the spacing
% for a design frequency of 1000 Hz.  The computed beam pattern is for a
% 900 Hz signal.  The bp_arb_weight is different than the others because
% two of the elements have been weighted to zero.

mesh_bp('bp_arb.csv', [], []);  title('USML arb'); view(0,0);
bp_arb(0.0,-20.0); title('matlab arb'); view(0,0);
mesh_bp('bp_arb_weight.csv', [], []);  title('USML arb weight'); view(0,0);

% Compare USML and Matlab versions of planar array.
%
% The planar array in this test has 7 elements in the horizontal and 5 in
% the vertical. The signal frequency of 900 Hz is slightly less than the
% 1000 Hz design frequency of the array. The main lobe is steered down 20
% deg and 10 deg to the right. The lobes are baffled in the back.

mesh_bp('bp_planar.csv', [], []);  title('USML planar'); view(0,0);
bp_planar(0.0, 0.0, 0.0, -20.0, 10.0, true); title('matlab planar'); view(0,0);
