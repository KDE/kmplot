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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef View_included
#define View_included
#undef	 GrayScale

// Qt includes
#include <tqpixmap.h>

// KDE includes
#include <dcopclient.h>
#include <kactionclasses.h>
#include <kdebug.h>
#include <kpopupmenu.h>
#include <kprinter.h>
#include <kprogress.h>
#include <kpushbutton.h>


// local includes
#include "diagr.h"
#include "Viewiface.h"
#include "xparser.h"

class XParser;
class KMinMax;
class KSliderWindow;

enum ZoomMode
{
    Z_Normal = 0,
    Z_Rectangular = 1,
    Z_ZoomIn = 2,
    Z_ZoomOut = 3,
    Z_Center = 4
};

/**
 * @short This class tqcontains the plots. 
 *
 * It is the central widget of MainDlg.
 * @see MainDlg, MainDlg::view
 */
class View : public TQWidget, virtual public ViewIface
{
	Q_OBJECT
public:
	/// Contructor sets up the parser, too.
  View(bool, bool &, KPopupMenu *, TQWidget* parent=NULL, const char* name=NULL );
	void setMinMaxDlg(KMinMax *);
	virtual ~View();

	/// Reimplemented to draw all stuff to the view.
	void draw(TQPaintDevice *, int const);
	/// Getting all relevant settings using KConfig XT class Settings.
	void getSettings();
	/// Clears all functions in the parser and gets default settings.
	/// @see getSettings
	void init();
	/// Finding the minimum or maximum value
	void findMinMaxValue(Ufkt *, char, bool, double &, double &,const TQString &);
	/// get a y-value from a x-value
	void getYValue(Ufkt * , char, double , double &,const TQString &);
	/// draw and calculate the area between the graph and the x-axis.
	void areaUnderGraph(Ufkt *, char const, double &, double &, const TQString &, TQPainter* );
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
	TQString areaParameter;

	/// Slider controlling parameter values
	KSliderWindow* sliders[ SLIDER_COUNT ];
	/// Menu actions for the sliders
	KToggleAction *mnuSliders[ SLIDER_COUNT ];
	void updateSliders(); /// show only needed sliders

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
	void paintEvent(TQPaintEvent *);
	void resizeEvent(TQResizeEvent *);
	/// Updating the cross hair.
	void mouseMoveEvent(TQMouseEvent *);
	/// Toggles the trace mode if the cursor is near to a plot.
	void mousePressEvent(TQMouseEvent *);
	/// when a key is pressed and the graph widget has focus
	void keyPressEvent(TQKeyEvent * );
	/// called when a mouse key is released
	void mouseReleaseEvent ( TQMouseEvent * e );
	/// Is needed to be reimplement so that the user can stop a preview-drawing
	bool event( TQEvent * e );

signals:
	void setStatusBarText(const TQString &);
    void resetZoom();

private:
	/// Print out table with additional information.
	/// Only for printing.
	void drawHeaderTable(TQPainter *);
	/// Draw the function plots.
	void plotfkt(Ufkt *ufkt, TQPainter*);
	/// Gets the greek pi symbol.
	void setpi(TQString *);
	/// in trace mode checks, if the function is (near by) zero
	bool root(double *, Ufkt *);
	///return the inverted color
	void invertColor(TQColor &, TQColor &);
	/// Restore the mouse cursor when a drawing is finished
	void restoreCursor();
	/// Changes the text in the statusbar
	void setStatusBar(const TQString &text, const int id);
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
	int fcx;	///< x-position of the crosshais (pixel)
	int fcy;	///< y-position of the crosshais (pixel)
	float csxpos;	///< y-position of the cross hair (real coordinates)
	float csypos;	///< x-position of the cross hair (real coordinates)
	//@}
	/// trace mode stuff
	int csflg;
	bool rootflg;

	CDiagr dgr;	///< Coordinate system
	TQPoint ref;
	TQRect area,
	PlotArea;
	TQPixmap hline,
	vline;
	TQWMatrix wm;

	double tlgx, tlgy, drskalx, drskaly;
	TQString tlgxstr, tlgystr, drskalxstr, drskalystr;
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
	void getMinMax(int koord, TQString &mini, TQString &maxi);
	/** Handle predefiend axes ranges.
	*
	* @p koord can have the values 0 to 4 which have the following meanings: 
	*
	* In the last case @p minstr and @p maxstr are evaluated.
	*/
	void coordToMinMax( const int koord, const TQString &minStr, const TQString &maxStr,
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
	TQColor backgroundcolor;
	/// the inverted background color used by the "Fadenkreuz"
	TQColor inverted_backgroundcolor;
	/// pointer to KMinMax
	KMinMax *m_minmax;
	///buffer the current window so all functions don't need to be re-drawed
	TQPixmap buffer;
	/// the popup menu
	KPopupMenu *m_popupmenu;
	/// is set to true if an integral is calculated
	bool isDrawing;
	///status of the popup menu
	char m_popupmenushown; /// 0==no popup 1==popup 2==popup+trace mode before
	/// for zoom-mode
	TQPoint rectangle_point;
	/*char zoom_mode;*/ ///0=normal 1=rectangular zoom, 2=zoom in, 3=zoom out ,4=drawing a rectangle, 5=center
    ZoomMode zoom_mode;
	/// true == modifications not saved
	bool &m_modified;
	/// False if KmPlot is started as a program, otherwise true
	bool const m_readonly;

	DCOPClient *m_dcop_client;
	TQString m_statusbartext1;
	TQString m_statusbartext2;
	TQString m_statusbartext3;
	TQString m_statusbartext4;
};

#endif // View_included
