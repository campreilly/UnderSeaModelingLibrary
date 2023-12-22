######################################################################
# USML studies

    set( USML_STUDIES_DIR ${PROJECT_SOURCE_DIR}/studies
         CACHE PATH "directory for data used in testing" )
    add_compile_definitions( USML_STUDIES_DIR="${USML_STUDIES_DIR}" )
    
    add_executable( ray_speed studies/ray_speed/ray_speed.cc )
    target_link_libraries( ray_speed usml )
    
    add_executable( cmp_speed studies/cmp_speed/cmp_speed.cc )
    target_link_libraries( cmp_speed usml )
    
    add_executable( pedersen_test studies/pedersen/pedersen_test.cc )
    target_link_libraries( pedersen_test usml )
    
    # add_executable( reverb_extra_test studies/reverberation/reverb_extra_test.cc )
    # target_link_libraries( reverb_extra_test usml )

    # add_executable( reverb_analytic_test studies/reverberation/reverb_analytic_test.cc )
    # target_link_libraries( reverb_analytic_test usml )

    add_executable( dead_reckon_test studies/dead_reckoning/dead_reckon_test.cc )
    target_link_libraries( dead_reckon_test usml )
    
    add_executable( simple_wedge studies/simple_wedge/simple_wedge.cc )
    target_link_libraries( simple_wedge usml )
    
