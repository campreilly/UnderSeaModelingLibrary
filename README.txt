The Under Sea Modeling Library (USML) is a collection of 
C++ software development modules for sonar modeling and simulation. 
The Wavefront Queue 3D (WaveQ3D) model is the component of USML 
that computes acoustic transmission loss in the ocean using 
Hybrid Gaussian Beams in Spherical/Time Coordinates. At this time, 
most of the other modules provide support to WaveQ3D. 

Instructions for building and testing this library are provided
in the usml_install.html file.  Documentation for the users of
this model is created from the source code using Doxygen.

Warnings:
- The big data files distributed with USML require you to install
  the git large file system (lfs) before they can be correctly
  downloaded from GitHub.  This is a GitHub requirement.
- The code must be checked out into a directory called "usml"
  in order to build properly.  The build process treate the parent
  of the "usml" as the root of the project.

