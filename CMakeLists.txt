kde4_header()
project(kmplot)

add_subdirectory( kmplot ) 
add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kmplot icons
#
#EXTRA_DIST = KmPlot.kdevelop KmPlot.kdevelop.filelist TODO
#
#AUTOMAKE_OPTIONS = foreign
#
#include ../admin/Doxyfile.am
