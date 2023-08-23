######################################################################
# USML_TEST regression test suite

    # define standard input and output data files for USML regression tests

    set( USML_DATA_DIR ${PROJECT_SOURCE_DIR}/data
         CACHE PATH "directory for data used in testing" )
    add_compile_definitions( USML_DATA_DIR="${USML_DATA_DIR}" )

    set( USML_TEST_DIR ${PROJECT_SOURCE_DIR}
         CACHE PATH "root directory for testing outputs" )
    add_compile_definitions( USML_TEST_DIR="${USML_TEST_DIR}" )

    foreach( MOD ${PACKAGE_MODULES} )
        file(MAKE_DIRECTORY ${USML_TEST_DIR}/${MOD}/test)
    endforeach( MOD )

    # generated data files for bathymetry tests

    if ( EXISTS ${NETCDF_NCKS} )
        set( USML_DATA_BATHYMETRY ${USML_DATA_DIR}/bathymetry/ETOPO1_Ice_g_gmt4.grd )
        set( ETOPO_CMP ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp )
        add_custom_command(
            OUTPUT ${ETOPO_CMP}.nc ${ETOPO_CMP}.log
            COMMAND ${NETCDF_NCKS} -O --no-abc -d y,18.0,23.000001 -d x,-160.0,-154.0
                "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP}.nc"
            COMMAND ncdump "${ETOPO_CMP}.nc" > "${ETOPO_CMP}.log"
        )

        set( ETOPO_CMP2W ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp2w )
        add_custom_command(
            OUTPUT ${ETOPO_CMP2W}.nc ${ETOPO_CMP2W}.log
            COMMAND ${NETCDF_NCKS} -O --no-abc -d y,-1.0,2.000001 -d x,179.0,180.0
                "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP2W}.nc"
            COMMAND ncdump "${ETOPO_CMP2W}.nc" > "${ETOPO_CMP2W}.log"
        )

        set( ETOPO_CMP2E ${PROJECT_SOURCE_DIR}/netcdf/test/etopo_cmp2e )
        add_custom_command(
            OUTPUT ${ETOPO_CMP2E}.nc ${ETOPO_CMP2E}.log
            COMMAND ${NETCDF_NCKS} -O --no-abc -d y,-1.0,2.000001 -d x,-179.9834,-178.0
                "${USML_DATA_BATHYMETRY}" "${ETOPO_CMP2E}.nc"
            COMMAND ncdump "${ETOPO_CMP2E}.nc" > "${ETOPO_CMP2E}.log"
        )

        add_custom_target( ETOPO_TEST_DATA ALL DEPENDS
        ${ETOPO_CMP}.nc ${ETOPO_CMP2W}.nc ${ETOPO_CMP2E}.nc )
    endif()

    # generated data files for profile tests

    if ( EXISTS ${NETCDF_NCKS} )
        set( USML_DATA_TEMP_MONTH ${USML_DATA_DIR}/woa09/temperature_monthly_1deg.nc )
        set( WOA_CMP ${PROJECT_SOURCE_DIR}/netcdf/test/woa_cmp )
        add_custom_command(
            OUTPUT ${WOA_CMP}.nc ${WOA_CMP}.log
            COMMAND ${NETCDF_NCKS} -O --no-abc -v depth,time,lat,lon,t_an
                -d time,165.0,165.0 -d lat,18.0,23.0 -d lon,200.0,206.0
                "${USML_DATA_TEMP_MONTH}" "${WOA_CMP}.nc"
            COMMAND ncdump "${WOA_CMP}.nc" > "${WOA_CMP}.log"
        )

        set( WOA_CMP2 ${PROJECT_SOURCE_DIR}/netcdf/test/woa_cmp2 )
        add_custom_command(
            OUTPUT ${WOA_CMP2}.nc ${WOA_CMP2}.log
            COMMAND ${NETCDF_NCKS} -O --no-abc -v depth,time,lat,lon,t_an
                -d time,0.0,0.0 -d lat,0.5,0.5 -d lon,0.5,0.5
                "${USML_DATA_TEMP_MONTH}" "${WOA_CMP2}.nc"
            COMMAND ncdump "${WOA_CMP2}.nc" > "${WOA_CMP2}.log"
        )

        add_custom_target( WOA_TEST_DATA ALL DEPENDS ${WOA_CMP}.nc ${WOA_CMP2}.nc )
    endif()


    # build the USML regression tests
    unset( HEADERS )
    file( GLOB SOURCES usml_test.cc )
    source_group( main FILES ${SOURCES} )
    FIND_SOURCES( "${PACKAGE_MODULES}" "/test" )

    add_executable( usml_test ${HEADERS} ${SOURCES} )
    target_link_libraries( usml_test usml pthread )
