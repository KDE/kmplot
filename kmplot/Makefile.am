bin_PROGRAMS = kmplot
kmplot_SOURCES = SwDlgData.ui SkalDlgData.ui RstDlgData.ui KoordDlgData.ui KOptDlgData.ui FktDlgData.ui BezWndData.ui xparser.cpp parser.cpp misc.cpp main.cpp diagr.cpp View.cpp SwDlg.cpp SkalDlg.cpp RstDlg.cpp MainDlg.cpp KoordDlg.cpp KOptDlg.cpp FktDlg.cpp BezWnd.cpp kprinterdlg.cpp settings.kcfgc settingspagecolor.ui settingspagecoords.ui settingspagefonts.ui settingspagescaling.ui qeditfunction.ui keditfunction.cpp
kmplot_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET) $(LIB_KDEPRINT)

EXTRA_DIST = kmplot.desktop BezWnd.cpp BezWnd.h FktDlg.cpp FktDlg.h KOptDlg.cpp KOptDlg.h KoordDlg.cpp KoordDlg.h MainDlg.cpp MainDlg.h RstDlg.cpp RstDlg.h SkalDlg.cpp SkalDlg.h SwDlg.cpp SwDlg.h View.cpp View.h diagr.cpp diagr.h main.cpp main.h misc.cpp misc.h parser.cpp parser.h xparser.cpp xparser.h BezWndData.ui FktDlgData.ui KOptDlgData.ui KoordDlgData.ui RstDlgData.ui SkalDlgData.ui SwDlgData.ui kprinterdlg.cpp kprinterdlg.h settingspagecolor.ui settingspagecoords.ui settingspagefonts.ui settingspagescaling.ui qeditfunction.ui keditfunction.h keditfunction.cpp

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

