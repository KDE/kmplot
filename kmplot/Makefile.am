bin_PROGRAMS = kmplot
kmplot_SOURCES = FktDlgData.ui BezWndData.ui xparser.cpp parser.cpp misc.cpp main.cpp diagr.cpp View.cpp  MainDlg.cpp FktDlg.cpp BezWnd.cpp kprinterdlg.cpp settings.kcfgc settingspagecolor.ui settingspagecoords.ui settingspagefonts.ui settingspagescaling.ui settingspageprecision.ui qeditfunction.ui keditfunction.cpp qeditparametric.ui keditparametric.cpp qeditpolar.ui keditpolar.cpp kmplotio.cpp
kmplot_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET) $(LIB_KDEPRINT)

EXTRA_DIST = kmplot.desktop BezWnd.h FktDlg.h MainDlg.cpp MainDlg.h View.h diagr.cpp diagr.h main.h misc.h parser.h xparser.h kprinterdlg.h qeditfunction.ui keditfunction.h keditparametric.h keditpolar.h kmplotio.h

xdg_apps_DATA = kmplot.desktop

kde_kcfg_DATA = kmplot.kcfg 

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

appmimedir = $(kde_mimedir)/application
appmime_DATA = x-kmplot.desktop

# the library search path. 
kmplot_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kmplot
rc_DATA = kmplotui.rc

messages: rc.cpp
	$(EXTRACTRC) *.ui >> rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmplot.pot; \
	fi

