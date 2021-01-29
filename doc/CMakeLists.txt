########### install files ###############
#

kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kmplot)
kdoctools_create_manpage(man-kmplot.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
