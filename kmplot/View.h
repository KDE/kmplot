#ifndef View_included
#define View_included

#include "misc.h"
#include "diagr.h"
#include "xparser.h"
#include <errno.h>
#include <kapplication.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <qwidget.h>
#include <qpdevmet.h>
#include <qpainter.h>
#include <qpoint.h>
#undef	 GrayScale 
//#include <qprinter.h>
#include <qevent.h>
#include <qstring.h>
#include <qpixmap.h>
#include <kprinter.h>


class View : public QWidget
{
	Q_OBJECT

public:

	View(QWidget* parent=NULL, const char* name=NULL);
	virtual ~View();

	void draw(QPaintDevice *, int);
	void tabelle(QPainter *);

	KStatusBar *stbar;

    
protected slots:

	void paintEvent(QPaintEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);


private:
	void plotfkt(int, QPainter*);
	void getMinMax(int koord, QString &mini, QString &maxi);
	void setpi(QString *);
    bool root(double *);

	int csflg,
	    csmode,
        rootflg,
	    fcx, fcy,       		// Position des Fadenkreuzes
	    w, h;

	float s,
	      csxpos, csypos;	    // Position des Fadenkreuzes
    CDiagr dgr;
	QPoint ref;
	QRect area,
		  PlotArea;
	QPixmap hline,
			vline;
	QWMatrix wm;

};

#endif // View_included
