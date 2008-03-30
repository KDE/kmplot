project(kmplot)

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmplot.h)

add_subdirectory( kmplot ) 
add_subdirectory( icons ) 

