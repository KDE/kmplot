



########### next target ###############

set(kmplotpart_PART_SRCS 
   plotstylewidget.cpp
   initialconditionseditor.cpp
   functioneditor.cpp
   parser.cpp 
   xparser.cpp 
   equationedit.cpp
   function.cpp
   view.cpp 
   maindlg.cpp 
   kprinterdlg.cpp 
   kconstanteditor.cpp 
   kparametereditor.cpp 
   kminmax.cpp 
   kmplotio.cpp 
   ksliderwindow.cpp 
   parameterswidget.cpp
   coordsconfigdialog.cpp
   parameteranimator.cpp
   vector.cpp
   kgradientdialog.cpp
   )

qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.MainDlg.xml maindlg.h MainDlg)
qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.Parser.xml xparser.h XParser)
qt4_add_dbus_adaptor( kmplotpart_PART_SRCS org.kde.kmplot.View.xml view.h View)
kde4_automoc(${kmplotpart_PART_SRCS})

kde4_add_ui_files(kmplotpart_PART_SRCS
	plotstylewidget.ui
	initialconditionswidget.ui
	functioneditorwidget.ui
	editcoords.ui
	editscaling.ui
	qminmax.ui
	qparametereditor.ui
	settingspagecolor.ui
	settingspageconstants.ui
	settingspagefonts.ui
	settingspagegeneral.ui
	sliderwindow.ui
	parameterswidget.ui
	parameteranimator.ui
	equationeditor.ui
	)

kde4_add_kcfg_files(kmplotpart_PART_SRCS settings.kcfgc )

kde4_add_plugin(kmplotpart WITH_PREFIX ${kmplotpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmplotpart )

target_link_libraries(kmplotpart  ${KDE4_KDE3SUPPORT_LIBS} kdeprint ${QT_QTXML_LIBRARY} )

install(TARGETS kmplotpart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############   

set(kmplot_SRCS main.cpp kmplot.cpp kmplotprogress.cpp)
qt4_add_dbus_adaptor(kmplot_SRCS org.kde.kmplot.KmPlot.xml kmplot.h KmPlot)
kde4_automoc(${kmplot_SRCS})

# FIXME: it's wrong, but otherwise it does not compile
kde4_add_kcfg_files(kmplot_SRCS settings.kcfgc )

kde4_add_executable(kmplot ${kmplot_SRCS})

target_link_libraries(kmplot  ${KDE4_KPARTS_LIBS} kdeprint )

install(TARGETS kmplot  DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES kmplot.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kmplot.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kmplot_part.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES x-kmplot.desktop  DESTINATION  ${MIME_INSTALL_DIR}/application )
install( FILES kmplot_part.rc kmplot_part_readonly.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmplot )
install( FILES kmplot_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmplot )




#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = kmplot.h MainDlg.h
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) *.ui >> rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/kmplot.pot; \
#	fi
## messages: rc.cpp
## $(XGETTEXT) *.cpp -o $(podir)/kmplot.pot
#
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kmplot
#
## the application source, library search path, and link libraries
#kmplot_SOURCES = main.cpp kmplot.cpp kmplotprogress.cpp kmplotIface.skel
#kmplot_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kmplot_LDADD   = $(LIB_KPARTS)
#
## this is where the desktop file will go
#appmimedir = $(kde_mimedir)/application
#appmime_DATA = x-kmplot.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/kmplot
#shellrc_DATA = kmplot_shell.rc
#
#
##########################################################################
## KPART SECTION
##########################################################################
#kde_module_LTLIBRARIES = libkmplotpart.la
#
## the Part's source, library search path, and link libraries
#libkmplotpart_la_SOURCES = FktDlgData.ui parser.cpp xparser.cpp  diagr.cpp View.cpp MainDlg.cpp FktDlg.cpp kprinterdlg.cpp settings.kcfgc settingspagecolor.ui kconstanteditor.cpp qparametereditor.ui kparametereditor.cpp settingspagecoords.ui settingspagefonts.ui settingspagescaling.ui settingspageprecision.ui qeditparametric.ui keditparametric.cpp qeditpolar.ui keditpolar.cpp editfunctionpage.ui editderivativespage.ui editintegralpage.ui editfunction.cpp qminmax.ui kminmax.cpp qconstanteditor.ui kmplotio.cpp sliderwindow.ui ksliderwindow.cpp MainDlgIface.skel parseriface.skel Viewiface.skel coordsconfigdialog.cpp
#libkmplotpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libkmplotpart_la_LIBADD  = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET) $(LIB_KDEPRINT)
#
## this is where the desktop file will go
#partdesktopdir   = $(kde_servicesdir)
#partdesktop_DATA = kmplot_part.desktop
#
## this is where the part's XML-GUI resource file goes
#partrcdir   = $(kde_datadir)/kmplot
#partrc_DATA = kmplot_part.rc kmplot_part_readonly.rc
#
#EXTRA_DIST = kmplot.desktop FktDlg.h MainDlg.cpp MainDlg.h View.h diagr.cpp diagr.h  xparser.h parser.h kprinterdlg.h keditparametric.h keditpolar.h ksliderwindow.h kconstanteditor.h kparametereditor.h kmplotio.h kminmax.h coordsconfigdialog.h
#
#xdg_apps_DATA = kmplot.desktop
#kde_kcfg_DATA = kmplot.kcfg
