#ifndef MainDlg_included
#define MainDlg_included

// Qt includes
#include <qwidget.h>
#include <qpdevmet.h>
#include <qpopmenu.h>
#include "qcolor.h"
#include <qkeycode.h>
#include <qdstream.h>
#include <qfile.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qpicture.h>

// KDE includes
#include <kapplication.h>
#include <kaction.h>
#include "kconfig.h"
#include <klocale.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include "kfiledialog.h"
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include "kmessagebox.h"
#include "kurl.h"
#undef  GrayScale
#include <kprinter.h>

// local includes
#include "FktDlg.h"
#include "KoordDlg.h"
#include "RstDlg.h"
#include "SkalDlg.h"
#include "SwDlg.h"
#include "BezWnd.h"
#include "misc.h"
#include "View.h"


class MainDlg : public KMainWindow
{
    Q_OBJECT

public:
    MainDlg( const char* name = NULL );
    virtual ~MainDlg();
    friend class FktDlg;
	friend class BezWnd;

public slots:
    void neu();
    void save();
    void saveas();
    void load();
    void print();
    void bezeichnungen();
    // void quit();
    void stbmode();
    void tbmode();
    void funktionen();
    void skalierung();
    void schrittw();
    void raster();
    void achsen();
    void onachsen1();
    void onachsen2();
    void onachsen3();
    // void about();
    void hilfe();
	void slotSettings();

private:
    void setupActions();
    void doSave();
    void setupStatusBar();
	void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );

	void parseAxes( const QDomElement &n );
	void parseGrid( const QDomElement &n );
	void parseScale( const QDomElement &n );
	void parseStep( const QDomElement &n );
	void parseFunction( const QDomElement &n );
    int tbid,
    stbid;
    KStatusBar *stbar;
    KToggleAction *viewToolBar;
    KToggleAction *viewStatusBar;
    FktDlg *fdlg;
	BezWnd *bez;
	KToggleAction *view_bezeichnungen;
    View *view;
};
#endif // MainDlg_included
