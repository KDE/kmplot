/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
*                     2006 David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef View_included
#define View_included
#undef	 GrayScale

// Qt includes
#include <qpixmap.h>
#include <QPointer>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>

// KDE includes
#include <dcopclient.h>
#include <kactionclasses.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kprinter.h>
#include <kprogressbar.h>
#include <kpushbutton.h>


// local includes
#include "diagr.h"
#include "Viewiface.h"
#include "xparser.h"

class XParser;
class KMinMax;
class KSliderWindow;

/**
 * @short This class contains the plots. 
 *
 * It is the central widget of MainDlg.
 * @see MainDlg, MainDlg::view
 */
class View : public QWidget, virtual public ViewIface
{
	Q_OBJECT
public:
	/// Contructor sets up the parser, too.
  View(bool, bool &, KMenu *, QWidget* parent, KActionCollection *ac, const char* name=NULL );
	void setMinMaxDlg(KMinMax *);
	virtual ~View();

	/// Reimplemented to draw all stuff to the view.
	void draw(QPaintDevice *, int const);
	/// Getting all relevant settings using KConfig XT class Settings.
	void getSettings();
	/// Clears all functions in the parser and gets default settings.
	/// @see getSettings
	void init();
	/// Finding the minimum or maximum value
	void findMinMaxValue(Ufkt *, char, bool, double &, double &,const QString &);
	/// get a y-value from a x-value
	void getYValue(Ufkt * , char, double , double &,const QString &);
	/// draw and calculate the area between the graph and the x-axis.
	void areaUnderGraph(Ufkt *, char const, double &, double &, const QString &, QPainter* );
	/// the calculation was cancelled by the user
	bool isCalculationStopped();

	/// Returns a pointer to the private parser instance m_parser.
	/// @see m_parser
	XParser* parser();

	/** Current plot range endge. */
	static double xmin;
	static double xmax;

	/// trace mode stuff, must be accessible in KMinMax
	int csmode, csparam;
	char cstype;

	/// for areadrawing when printing
	bool areaDraw;
	Ufkt * areaUfkt;
	char areaPMode;
	double areaMin, areaMax;
	QString areaParameter;

	/// Slider controlling parameter values
	QPointer<KSliderWindow> sliders[ SLIDER_COUNT ];
	/// Menu actions for the sliders
	KToggleAction *mnuSliders[ SLIDER_COUNT ];
	void updateSliders(); /// show only needed sliders
	
	/// Convert a width in mm to a suitable QPen width for drawing
	double mmToPenWidth( double width_mm, bool antialias ) const;

public slots:
	/// Called when the user want to cancel the drawing
	void stopDrawing();
	/// A slider window has been closed
	void sliderWindowClosed(int);

	/// Called when the graph should be updated
	void drawPlot();
	///Slots for the three first items in popup menu
	void mnuCopy_clicked();
	void mnuMove_clicked();
	void mnuHide_clicked();
	void mnuRemove_clicked();
	void mnuEdit_clicked();
	///Slots for the zoom menu
	void mnuNoZoom_clicked();
	void mnuRectangular_clicked();
	void mnuZoomIn_clicked();
	void mnuZoomOut_clicked();
	void mnuCenter_clicked();
	void mnuTrig_clicked();

protected slots:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
	/// Updating the cross hair.
	void mouseMoveEvent(QMouseEvent *);
	/// Toggles the trace mode if the cursor is near to a plot.
	void mousePressEvent(QMouseEvent *);
	/// when a key is pressed and the graph widget has focus
	void keyPressEvent(QKeyEvent * );
	/// called when a mouse key is released
	void mouseReleaseEvent ( QMouseEvent * e );
	/// Is needed to be reimplement so that the user can stop a preview-drawing
	bool event( QEvent * e );

signals:
	void setStatusBarText(const QString &);

private:
	/// Print out table with additional information.
	/// Only for printing.
	void drawHeaderTable(QPainter *);
	/// Draw the function plots.
	void plotfkt(Ufkt *ufkt, QPainter*);
	/// @return an appropriate pen for drawing the plot
	QPen penForPlot( Ufkt * ufkt, int p_mode, bool antialias ) const;
	/// Gets the greek pi symbol.
	void setpi(QString *);
	/// in trace mode checks, if the function is (near by) zero
	bool root(double *, Ufkt *);
	///return the inverted color
	void invertColor(QColor &, QColor &);
	/// Restore the mouse cursor when a drawing is finished
	void restoreCursor();
	/// Changes the text in the statusbar
	void setStatusBar(const QString &text, const int id);
	/// Functions for the progressbar
	bool stopProgressBar();
	void startProgressBar(int);
	void increaseProgressBar();

	/// The central parser instance.
	/// @see parser()
	XParser *m_parser;

	int w, h;
	float s;

	/// @name Crosshair
	/// Crosshair support.
	//@{
	float fcx;	///< x-position of the crosshais (pixel)
	float fcy;	///< y-position of the crosshais (pixel)
	float csxpos;	///< y-position of the cross hair (real coordinates)
	float csypos;	///< x-position of the cross hair (real coordinates)
	//@}
	/// trace mode stuff
	bool rootflg;

	/// @return whether csxpos is in the range of the view or in the custom range for the given \p plot
	bool csxposValid( Ufkt * plot ) const;
	
	CDiagr dgr;	///< Coordinate system
	QPoint ref;
	QRect area,
	PlotArea;
	QMatrix wm;

	double tlgx, tlgy, drskalx, drskaly;
	QString tlgxstr, tlgystr, drskalxstr, drskalystr;
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
	void coordToMinMax( const int koord, const QString &minStr, const QString &maxStr,
	                    double &min, double &max );
	/// Sets the plot range from Settings
	void setPlotRange();
	//@{
	/** Current plot range endge. */
	double ymin;
	double ymax;
	//@}
	//@}

	void setScaling();
	/// represents the KPrinter option app-kmplot-printtable.
	/// @see KPrinterDlg
	bool m_printHeaderTable;
	/// if stop_calculating is true, the user has canceled drawing of an integral graph
	bool stop_calculating;
	/// the background color of the graph
	QColor backgroundcolor;
	/// the inverted background color used by the "Fadenkreuz"
	QColor inverted_backgroundcolor;
	/// pointer to KMinMax
	KMinMax *m_minmax;
	///buffer the current window so all functions don't need to be re-drawed
	QPixmap buffer;
	/// the popup menu
	KMenu *m_popupmenu;
	/// is set to true if an integral is calculated
	bool isDrawing;
	///status of the popup menu
	char m_popupmenushown; /// 0==no popup 1==popup 2==popup+trace mode before
	/// true == modifications not saved
	bool &m_modified;
	/// False if KmPlot is started as a program, otherwise true
	bool const m_readonly;
	
	enum ZoomMode
	{
		Normal=0,			///< normal
		Rectangular=1,		///< rectangular zoom (in)
		ZoomIn=2,			///< zoom in
		ZoomOut=3,			///< zoom out
		DrawingRectangle=4,	///< drawing a rectangle
		Center=5			///< centering a point
	};
		
	ZoomMode zoom_mode;
	/// for zoom-mode
	QPoint rectangle_point;

	DCOPClient *m_dcop_client;
	QString m_statusbartext1;
	QString m_statusbartext2;
	QString m_statusbartext3;
	QString m_statusbartext4;
	KActionCollection *m_ac;
};

#endif // View_included
