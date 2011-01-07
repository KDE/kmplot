# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

# these are the headers for your project
noinst_HEADERS = kmplot.h kmplotprogress.h MainDlg.h

METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) *.ui >> rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmplot.pot; \
	fi
# messages: rc.cpp
# $(XGETTEXT) *.cpp -o $(podir)/kmplot.pot


#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kmplot

# the application source, library search path, and link libraries
kmplot_SOURCES = main.cpp kmplot.cpp kmplotprogress.cpp kmplotIface.skel
kmplot_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kmplot_LDADD   = $(LIB_KPARTS)

# this is where the desktop file will go
appmimedir = $(kde_mimedir)/application
appmime_DATA = x-kmplot.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kmplot
shellrc_DATA = kmplot_shell.rc


#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkmplotpart.la

# the Part's source, library search path, and link libraries
libkmplotpart_la_SOURCES = FktDlgData.ui parser.cpp xparser.cpp  diagr.cpp View.cpp MainDlg.cpp FktDlg.cpp kprinterdlg.cpp settings.kcfgc settingspagecolor.ui kconstanteditor.cpp qparametereditor.ui kparametereditor.cpp settingspagecoords.ui settingspagefonts.ui settingspagescaling.ui settingspageprecision.ui qeditparametric.ui keditparametric.cpp qeditpolar.ui keditpolar.cpp editfunctionpage.ui editderivativespage.ui editintegralpage.ui editfunction.cpp qeditconstant.ui qminmax.ui kminmax.cpp qconstanteditor.ui keditconstant.cpp kmplotio.cpp sliderwindow.ui ksliderwindow.cpp MainDlgIface.skel parseriface.skel Viewiface.skel coordsconfigdialog.cpp
libkmplotpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkmplotpart_la_LIBADD  = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET) $(LIB_KDEPRINT)

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kmplot_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kmplot
partrc_DATA = kmplot_part.rc kmplot_part_readonly.rc

EXTRA_DIST = kmplot.desktop FktDlg.h MainDlg.cpp MainDlg.h View.h diagr.cpp diagr.h  xparser.h parser.h kprinterdlg.h keditparametric.h keditpolar.h ksliderwindow.h keditconstant.h kconstanteditor.h kparametereditor.h kmplotio.h kminmax.h coordsconfigdialog.h

xdg_apps_DATA = kmplot.desktop
kde_kcfg_DATA = kmplot.kcfg
