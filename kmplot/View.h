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

class KMinMax;
class KSliderWindow;
class MainDlg;
class XParser;
class QTime;

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
	View(bool, bool &, KMenu *, QWidget* parent, KActionCollection *ac, MainDlg * mainDlg );
	void setMinMaxDlg(KMinMax *);
	virtual ~View();
	
	/// There is only one view.
	static View * self() { return m_self; }

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
	void areaUnderGraph(Ufkt *, Ufkt::PMode, double &, double &, const QString &, QPainter* );
	/// the calculation was cancelled by the user
	bool isCalculationStopped();

	/// Returns a pointer to the private parser instance m_parser.
	/// @see m_parser
	XParser* parser();
	/// Returns a pointer to the MainDlg
	MainDlg * mainDlg() const { return m_mainDlg; }

	/// Slider controlling parameter values
	QPointer<KSliderWindow> m_sliderWindow;
	/// Menu actions for the sliders
	KToggleAction * m_menuSliderAction;
	void updateSliders(); /// show only needed sliders
	
	/// Convert a width in mm to a suitable QPen width for drawing
	double mmToPenWidth( double width_mm, bool antialias ) const;

	/** Current plot range endge. */
	static double xmin;
	static double xmax;

	/// trace mode stuff, must be accessible in KMinMax
	int csmode, csparam;
	Ufkt::PMode cstype;

public slots:
	/// Called when the user want to cancel the drawing
	void stopDrawing();
	/// A slider window has been closed
	void slidersWindowClosed();

	/// Called when the graph should be updated
	void drawPlot();
	///Slots for the three first items in popup menu
	void mnuHide_clicked();
	void mnuRemove_clicked();
	void mnuEdit_clicked();
	///Slots for the zoom menu
	void mnuZoomIn_clicked();
	void mnuZoomOut_clicked();
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
	/// Restore the mouse cursor when a drawing is finished
	void updateCursor();
	/**
	 * Updates csxpos and csypos from the current mouse position.
	 * @return whether the crosshair is within the bounds of the diagram.
	 */
	bool updateCrosshairPosition();

signals:
	void setStatusBarText(const QString &);
	
protected:
	/// called when focus is lost
	virtual void focusOutEvent( QFocusEvent * );
	/// called when focus is gained
	virtual void focusInEvent( QFocusEvent * );

private:
	/// Print out table with additional information.
	/// Only for printing.
	void drawHeaderTable(QPainter *);
	/// Draw the function plots.
	void plotfkt(Ufkt *ufkt, QPainter*);
	/// @return an appropriate pen for drawing the plot
	QPen penForPlot( Ufkt * ufkt, Ufkt::PMode, bool antialias ) const;
	/// Gets the greek pi symbol.
	void setpi(QString *);
	/// in trace mode checks, if the function is (near by) zero
	bool root(double *, Ufkt *);
	///return the inverted color
	void invertColor(QColor &, QColor &);
	/// Changes the text in the statusbar
	void setStatusBar(const QString &text, const int id);
	/// Functions for the progressbar
	bool stopProgressBar();
	void startProgressBar(int);
	void increaseProgressBar();
	/// @return whether the crosshairs should be shown for the current mouse position
	bool shouldShowCrosshairs() const;
	/**
	 * Zooms in by amount \p zoomFactor (which will zooming out if less than 1)
	 * from clicking at \p mousePos (in widget coordinates).
	 */
	void zoomIn( const QPoint & mousePos, double zoomFactor );
	/// zooms in from having drawn \p zoomRect (which is in widget coordinates)
	void zoomIn( const QRect & zoomRect );
	/// zooms out from havoutg drawn \p zoomRect (which is out widget coordoutates)
	void zoomOut( const QRect & zoomRect );
	/// translates the view by \p dx, \p dy (in widget coordinates)
	void translateView( int dx, int dy );
	/// animates zooming from the current zoom rect to the one given (in real coordinates)
	void animateZoom( const QRectF & newCoords );
	/**
	 * Finds the plot (if any) under the last mouse pos as recorded by
	 * updateCrosshairPosition(). This sets csmode, cstype, csparam. If no plot
	 * was found, then csmode is set to -1.
	 */
	void getPlotUnderMouse();
	/**
	 * Finds the closest point to \p real_x and \p real_y to the given polar or
	 * parametric function.
	 * \return the parametization (angle or t) that gives the closest point.
	 */
	double getClosestPoint( double real_x, double real_y, Ufkt * function1, Ufkt * function2 );
	/**
	 * Calculates the pixel distance from \p real_x and \p real_y to the display
	 * point of the given polar or parametric parametric function at \p x.
	 */
	double pixelDistance( double real_x, double real_y, Ufkt * function1, Ufkt * function2, double x );
	

	/// for areadrawing when printing
	bool areaDraw;
	Ufkt * areaUfkt;
	Ufkt::PMode areaPMode;
	double areaMin, areaMax;
	QString areaParameter;

	/// The central parser instance.
	/// @see parser()
	XParser *m_parser;

	int w, h;
	float s;
	
	QPointF m_crosshairPixelCoords;
	float csxpos;	///< y-position of the cross hair (real coordinates)
	float csypos;	///< x-position of the cross hair (real coordinates)
	
	/// The t- or x- (angle) coordinate of the traced curve
	double m_trace_x;
	
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
		Normal,				///< no zooming
		AnimatingZoom,		///< animating a current zooming
		ZoomIn,				///< zoom in
		ZoomOut,			///< zoom out
		ZoomInDrawing,		///< drawing a rectangle for zooming in
		ZoomOutDrawing,		///< drawing a rectangle for zooming out
		AboutToTranslate,	///< user has clicked on an empty spot, but hasn't moved the mouse yet
		Translating,		///< dragging the view with the mouse
	};
		
	/// The current editing status
	ZoomMode m_zoomMode;
	/// for zoom-mode
	QPoint m_zoomRectangleStart;
	/// for animating zoom; contains the rectangle (in real coordinates) to draw
	QRectF m_animateZoomRect;
	/// for translating the view via dragging
	QPoint m_prevDragMousePos;
	/// timer that is started when the mouse is pressed
	QTime * m_mousePressTimer;

	DCOPClient *m_dcop_client;
	QString m_statusbartext1;
	QString m_statusbartext2;
	QString m_statusbartext3;
	QString m_statusbartext4;
	KActionCollection *m_ac;
	
	enum Cursor { CursorWait, CursorBlank, CursorArrow, CursorCross, CursorMagnify, CursorLessen, CursorMove };
	Cursor m_prevCursor;
	
	MainDlg * m_mainDlg;
	static View * m_self;
};

#endif // View_included
