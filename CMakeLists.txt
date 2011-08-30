project(kmplot)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(CheckIncludeFiles)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

check_include_files(ieeefp.h HAVE_IEEEFP_H)
configure_file(config-kmplot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmplot/config-kmplot.h)

add_subdirectory( doc )
add_subdirectory( kmplot ) 
add_subdirectory( icons ) 

