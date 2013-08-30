This directory consists of the following scripts:

wedge_test_2d - Computes complex pressure in the Deane/Buckingham wedge 
solution for a 2-D slice in the plane of of the source.  The scenario 
is based on Figure 6 in the March 1993 paper.  The model is structured 
as follows:

    - wedge_spectrum_2d() computes the wave number spectrum for one image.
      This calculation uses reflection() to model bottom loss.
    - wedge_integ_2d() uses simpson() to integrate the wave number spectrum 
      along the contour 0 to pi/2-Inf.
    - wedge_pressure_2d() coherently sums the contributions from all images.
      This calculation uses spherical_add() to compute the location
      of targets relative to each image source.

reflection_test - Tests reflection.m using the scenario defined in 
Figure 1.22 of Jensen, Kuperman, Porter, Schmidt, Computational Ocean 
Acoustics, 1st Edition, 1994.  Computes reflection loss as a function of
sound speed, attenuation, density, and shear speed.

To-do items:

general: Have we oriented theta and zeta correctly relative to each other?
simpson: Needs to be tested, including tests for complex integrands
wedge_pressure_2d: Need to check the source_zeta terms in spherical_add()!!!
wedge_integ_2d: Error: complex portion of spectrum goes to Inf for large X!!! Should be decaying.
wedge_model_2d: Expand calculation to multiple ranges

Notes:

1) zeta was oriented from the x-axis as opposed to the z-axis. Corrected zeta by adding pi/2 to the values and now have corrected values.
2) corrected the equation in wedge_spectrum_2d so that the imaginary parts go to 0 at large values by including the appropriate negative sign in the equation.
