/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef View_included
#define View_included

#include "diagr.h"
#include <errno.h>
#include <kstatusbar.h>
#undef	 GrayScale 
#include <kprinter.h>

/**
 * This class contains the plots.
 */
class View : public QWidget
{
	Q_OBJECT

public:

	View(QWidget* parent=NULL, const char* name=NULL);
	virtual ~View();

	void draw(QPaintDevice *, int);
	void drawHeaderTable(QPainter *);

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
	
	/// current plot range
	double xmin, xmax, ymin, ymax;
	/** Handle predefiend axes ranges.
	*
	* @p koord can have the values 0 to 4 which have the following meanings: 
	* @li 0: -8..8
	* @li 1: -5..5
	* @li 2: 0..16
	* @li 3: 0..10
	* @li 4: custom
	*
	* In the last case @p minstr and @p maxstr are evaluated.
	*/ 
	void coordToMinMax( const int koord, const QString minStr, const QString maxStr, 
		double &min, double &max );
	void setPlotRange();
};

#endif // View_included
