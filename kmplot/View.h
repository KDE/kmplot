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
#undef	 GrayScale 

// Qt includes
#include <qpixmap.h>

// KDE includes
#include <kstatusbar.h>
#include <kprinter.h>

// local includes
#include "diagr.h"
#include "errno.h"

class XParser;


/**
 * @short This class contains the plots. 
 *
 * It is the central widget of MainDlg.
 * @see MainDlg, MainDlg::view
 */
class View : public QWidget
{
	Q_OBJECT
public:
	/// Contructor sets up the parser, too.
	View( QWidget* parent=NULL, const char* name=NULL );
	
	virtual ~View();

	/// Reimplemented to draw all stuff to the view.
	void draw(QPaintDevice *, int);
	/// Getting all relevant settings using KConfig XT class Settings.
	void getSettings();
	/// Clears all functions in the parser and gets default settings.
	/// @see getSettings
	void init();
	
	/// Returns a pointer to the private parser instance m_parser.
	/// @see m_parser
	XParser* parser();

	/// Points to the status bar.
	KStatusBar *stbar;
    
protected slots:
	void paintEvent(QPaintEvent *);
	/// Updating the cross hair.
	void mouseMoveEvent(QMouseEvent *);
	/// Toggles the trace mode if the cursor is near to a plot.
	void mousePressEvent(QMouseEvent *);

private:
	/// Print out table with additional information.
	/// Only for printing.
	void drawHeaderTable(QPainter *);
	/// Draw the function plots.
	void plotfkt(int, QPainter*);
	/// Gets the greek pi symbol.
	void setpi(QString *);
	/// in trace mode checks, if the function is (near by) zero
	bool root(double *);
	
	/// The central parser instance.
	/// @see parser()
	XParser *m_parser;
	
	int csflg,
	    csmode,
	    rootflg;
	int w, h;
	
	/// @name Crosshair
	/// Crosshair support.
	//@{
	int fcx;	///< x-position of the crosshais (pixel)
	int fcy;	///< y-position of the crosshais (pixel)
	float csxpos;	///< y-position of the cross hair (real coordinates) @add
	float csypos;	///< x-position of the cross hair (real coordinates)
	//@}
	float s;
	
	CDiagr dgr;	///< Coordinate system 
	QPoint ref;
	QRect area,
		  PlotArea;
	QPixmap hline,
			vline;
	QWMatrix wm;
	
	double tlgx, tlgy, drskalx, drskaly;
	QString tlgxstr, tlgystr, drskalxstr, drskalystr;
	double relativeStepWidth;  ///< Precision relativly to the size.
	double stepWidth; ///< Absolute step width 
	
	/** @name Plotrange
	 * There are 4 predefined plot ranges:
	 * @li 0: -8..8
	 * @li 1: -5..5
	 * @li 2: 0..16
	 * @li 3: 0..10
	 * @li 4: custom
	 */
	//@{
	///Convert axes range predefinition index to boundaries.
	void getMinMax(int koord, QString &mini, QString &maxi);
	/** Handle predefiend axes ranges.
	*
	* @p koord can have the values 0 to 4 which have the following meanings: 
	*
	* In the last case @p minstr and @p maxstr are evaluated.
	*/ 
	void coordToMinMax( const int koord, const QString minStr, const QString maxStr, 
		double &min, double &max );
	/// Sets the plot range from Settings
	void setPlotRange();
	//@{
	/** Current plot range endge. */
	double xmin; 
	double xmax;
	double ymin;
	double ymax;
	//@}
	//@}
	
	void setScaling();
	/// represents the KPrinter option app-kmplot-printtable.
	/// @see KPrinterDlg
	bool m_printHeaderTable;
};

#endif // View_included
