kde4_header()

add_subdirectory( actions ) 




########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = actions
#KDE_ICON=AUTO