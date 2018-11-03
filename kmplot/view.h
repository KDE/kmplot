/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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

#include "function.h"

// Qt includes
#include <qpixmap.h>
#include <QPointer>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <ktoggleaction.h>

// KDE includes
#include <QDebug>

class KSliderWindow;
class MainDlg;
class QPaintEvent;
class QTextDocument;
class QTextEdit;
class QTime;
class QMenu;

//@{
/// Some abbreviations for horizontal and vertical lines.
#define Line drawLine
#define Lineh(x1, y, x2) drawLine( QPointF(x1, y), QPointF(x2, y) )
#define Linev(x, y1, y2) drawLine( QPointF(x, y1), QPointF(x, y2) )
//@}


/**
 * For drawing the area of a (Cartesian) plot.
 */
 class IntegralDrawSettings
{
	public:
		IntegralDrawSettings();
		
		Plot plot;
		double dmin, dmax;
		/// Set to true when calculating the area under the graph
		bool draw;
};


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
		/// Constructor
		View( bool readOnly, QMenu * functionPopup, QWidget* parent );
		virtual ~View();
	
		/// There is only one view.
		static View * self() { return m_self; }

		enum PlotMedium
		{
			Screen,
			Printer,
			SVG,
			Pixmap
		};
		/**
		 * Draw the plot to \p dev, which is of the given \p medium.
		 */
		void draw( QPaintDevice * dev, PlotMedium medium );
		
		enum ExtremaType { Minimum, Maximum };
		/**
		* Finding the minimum or maximum value.
		* \return The (x,y) coordinates of the extrema point.
		*/
		QPointF findMinMaxValue( const Plot & plot, ExtremaType type, double dmin, double dmax );
		/**
		* Calculates the area between the given plot and the x-axis
		* (from x = \p dmin to x = \p dmax). The area will also be colored in.
		* \return the area.
		*/
		double areaUnderGraph( IntegralDrawSettings settings );
		/**
		 * \return if the calculation was cancelled by the user.
		 */
		bool isCalculationStopped();
		/**
		 * Used in posToString for requesting how the position string is to be
		 * created.
		 */
		enum PositionFormatting
		{
			DecimalFormat,		///< Plain text, using no scientific notation; just decimal expansion.
			ScientificFormat	///< Rich text possibly using scientific notation (mult x 10 ^ exp).
		};
		/**
		 * @return a string for displaying the x or y coordinate in the statusbar.
		 * \param x The number to convert to a string.
		 * \param delta is the amount by which the value varies over one pixel in
		 * the view. This is for choosing an appropriate number of decimals so that
		 * moving the cursor shows a nice change in the string.
		 * \param format How the number should be represented as a string.
		 * \param color If using scientific mode, the color to format the text.
		 */
		QString posToString( double x, double delta, PositionFormatting format, const QColor &color = Qt::black ) const;

		/// Slider controlling parameter values
		QPointer<KSliderWindow> m_sliderWindow;
		/// Menu actions for the sliders
		KToggleAction * m_menuSliderAction;
		void updateSliders(); /// show only needed sliders
		
		/**
		 * Convert \p width_mm (millimeters) to the equivalent length when
		 * drawing using \p painter.
		 */
		double millimetersToPixels( double width_mm, QPaintDevice * device ) const;
		/**
		 * The inverse of millimetersToPixels().
		 */
		double pixelsToMillimeters( double width_pixels, QPaintDevice * device ) const;

		/** Current plot x-range. */
		double m_xmin;
		double m_xmax;
		/** Current plot y-range. */
		double m_ymin;
		double m_ymax;

		/// trace mode stuff, must be accessible in FunctionTools
		Plot m_currentPlot;
		/**
		 * Convenience function for calculating the value of \p eq using the
		 * given \p mode
		 */
		double value( const Plot & plot, int eq, double x, bool updateFunction );
		/**
		 * \return the real position of the function (similar to calling
		 * value(), but returns both coordinates).
		 */
		QPointF realValue( const Plot & plot, double x, bool updateFunction );
		/**
		 * \return the (signed) curvature (in screen coordinates) of the plot
		 * at \p x (and \p y for implicit functions).
		 */
		double pixelCurvature( const Plot & plot, double x, double y = 0 );
		/**
		 * \return the angle of the normal (in radians) of the plot when viewed
		 * on the screen.
		 */
		double pixelNormal( const Plot & plot, double x, double y = 0 );
		/**
		 * Animates zooming from the current zoom rect to the one given (in real
		 * coordinates)
		 */
		void animateZoom( const QRectF & newCoords );

		///Methods for the Print Dialog to set options for drawing
		void setPrintHeaderTable( bool status );
		void setPrintBackground( bool status );
		void setPrintWidth( double width );
		void setPrintHeight( double height );
                
		/**
		 * Which part of the status bar.
		 */
		enum StatusBarSection
		{
			XSection		= 0,
			YSection		= 1,
			RootSection		= 2,
			FunctionSection		= 3,
			SectionCount		= 4
		};

	public slots:
		/// Called when the user want to cancel the drawing
		void stopDrawing();
	
		/// Called when the graph should be updated
		void drawPlot();
		/// Called when a function is deleted
		void functionRemoved( int id );
		///Slots for the three first items in popup menu
		void hideCurrentFunction();
		void removeCurrentPlot();
		void editCurrentPlot();
		void animateFunction();
		///Slots for the zoom menu
		void zoomIn();
		void zoomOut();
		void zoomToTrigonometric();

	protected slots:
		void sliderWindowClosed();
		/// Restore the mouse cursor when a drawing is finished
		void updateCursor();
	
	signals:
		void setStatusBarText(const QString &);
	
	protected:
		/// called when focus is lost
		void focusOutEvent( QFocusEvent * ) Q_DECL_OVERRIDE;
		/// called when focus is gained
		void focusInEvent( QFocusEvent * ) Q_DECL_OVERRIDE;
		void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
		void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
		/// Updating the cross hair.
		void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
		/// Clearing the cross hair.
		void leaveEvent(QEvent *) Q_DECL_OVERRIDE;
		/// Toggles the trace mode if the cursor is near to a plot.
		void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
		/// when a key is pressed and the graph widget has focus
		void keyPressEvent(QKeyEvent * ) Q_DECL_OVERRIDE;
		/// called when a mouse key is released
		void mouseReleaseEvent ( QMouseEvent * e ) Q_DECL_OVERRIDE;
		/// Is needed to be reimplement so that the user can stop a preview-drawing
		bool event( QEvent * e ) Q_DECL_OVERRIDE;
		/**
		 * Updates csxpos and csypos from the current mouse position.
		 * @return whether the crosshair is within the bounds of the diagram.
		 */
		bool updateCrosshairPosition();
	
	private:
		/**
		 * Fills the popup menu according to the currently selected plot.
		 */
		void fillPopupMenu();
		/**
		 * For using in automatic tic spacing. Given \p range (e.g. x_max-x_min)
		 * and the \p length_mm (in millimeters), it aims to find a "nice"
		 * spacing distance that is visually pleasing and also fits the base 10
		 * number system in use (i.e. is a decimal multiple of 1, 2 or 5).
		 */
		double niceTicSpacing( double length_mm, double range );
		/**
		 * For using in manual tic spacing. Given a size in screen \p pixels for
		 * the given \p range (e.g. x_max-x_min), make sure the \p spacing distance
		 * is at least \p minPixels large. If zooming out too far, the spacing is
		 * doubled until this conditions is met, effectively omitting tics. If zooming in
		 * too far, the spacing is halved until at least two tics are visible, effectively
		 * inserting additional tics.
		*/
		double validatedTicSpacing( double spacing, double range, double pixels, double minPixels );
		/**
		 * When zoomed in on part of a circle, it looks nearly straight. KmPlot
		 * uses this to quickly draw curves that are mostly straight. Given the
		 * curvature, this function returns the maximum length of line that can
		 * be used to draw a part of a curve with the given curvature without
		 * the curve starting to look jagged.
		 */
		static double maxSegmentLength( double curvature );
		/**
		 * \return an appropriate value to use in numerical differentiation.
		 */
		double h( const Plot & plot ) const;
		/**
		 * Initializes the size, scaling, etc variables from the given paint
		 * device.
		 */
		void initDrawing( QPaintDevice * device, PlotMedium medium );
		/**
		* Print out table with additional information. Only for printing.
		*/
		void drawHeaderTable(QPainter *);
		/// Draw the grid.
		void drawGrid( QPainter* );
		/**
		 * Draw the axes.
		 */
		void drawAxes( QPainter *painter );
		/**
		 * Draw the axes' labels.
		 */
		void drawLabels( QPainter *painter );
		/**
		 * Draw the labels for the x-axis (this function is called from
		 * drawLabels).
		 * \a painter The QPainter to draw the labels with
		 * \a endLabelWidth_mm the distance of the "x" label from the edge.
		 */
		void drawXAxisLabels( QPainter *painter, double endLabelWidth_mm );
		/**
		 * Draw the labels for the y-axis (this function is called from
		 * drawLabels).
		 */
		void drawYAxisLabels( QPainter *painter );
		/**
		 * Draw a non-implicit function.
		 */
		void drawFunction( Function * function, QPainter * painter );
		/**
		 * Draw the function plots (other than implicit).
		 */
		void drawPlot( const Plot & plot, QPainter*);
		/**
		 * Draw the tangent field (for a differential function).
		 */
		void drawTangentField( const Plot & plot, QPainter * painter );
		/**
		 * Draw an implicit function.
		 */
		void drawImplicit( Function * function, QPainter * );
		/**
		 * Draw the extrema points, function names, etc. This needs to be done
		 * after the functions have all been drawn so that the label positioning
		 * knows where the plots have been drawn.
		 */
		void drawFunctionInfo( QPainter * painter );
		/**
		 * Initializes for the drawLabel function, called before drawing has
		 * started.
		 */
		void initDrawLabels();
		/**
		 * Draw text (e.g. showing the value of an extrema point or a function
		 * name) at the given (real) position.
		 */
		void drawLabel( QPainter * painter, const QColor & color, const QPointF & realPos, const QString & text );
		/**
		 * Used by plotImplicit to draw the plot in the square associated with
		 * the given point.
		 */
		void drawImplicitInSquare( const Plot & plot, QPainter *, double x, double y, Qt::Orientations orientation, QList<QPointF> * singular );
		/**
		* \return whether should draw the pixel from the given line length,
		* according to the given pen style (used in plotfkt).
		*/
		bool penShouldDraw( double totalLength, const Plot & plot );
		/**
		* \return An appropriate pen for drawing the plot.
		*/
		QPen penForPlot( const Plot & plot, QPainter * painter ) const;
		/**
		 * Used in findRoot.
		 */
		enum RootAccuracy
		{
			PreciseRoot,	///< Will potential spend a long time finding a root to a high degree of accuracy
			RoughRoot		///< Won't spend too long making a root accurate, giving up quickly if failed to find root
		};
		/**
		 * Used in trace mode. Attempts to find the root of equation \p eq near
		 * \p x (which is then set to the exact root if found).
		 * \returns whether a root was found.
		 */
		bool findRoot( double * x, const Plot & plot, RootAccuracy accuracy );
		/**
		 * Equivalent function as above for implicit functions.
		 */
		bool findRoot( double * x, double * y, const Plot & plot, RootAccuracy accuracy );
		/**
		 * For use in the findRoot functions.
		 * \p max_k maximum number of iterations
		 * \p max_f the largest value of y which is deemed a root found
		 */
		void setupFindRoot( const Plot & plot, RootAccuracy accuracy, double * max_k, double * max_f, int * n );
		/**
		 * Finds the list of points (in function coordinates) at which the
		 * derivative of the given plot is zero in the range of the currently
		 * viewable segment of the plot.
		 */
		QList<QPointF> findStationaryPoints( const Plot & plot );
		/**
		 * Find all roots (at which the given plot is zero) in the range
		 * [min,max].
		 */
		QList<double> findRoots( const Plot & plot, double min, double max, RootAccuracy accuracy );
		/**
		 * Changes the text in the statusbar.
		 */
		void setStatusBar( const QString &text, StatusBarSection section );
		/**
		* \return whether the crosshairs should be shown for the current mouse
		* position, zoom mode, etc.
		*/
		bool shouldShowCrosshairs() const;
		/**
		* Zooms in by amount \p zoomFactor (which will zooming out if less than 1)
		* from clicking at \p mousePos (in widget coordinates).
		*/
		void zoomIn( const QPoint & mousePos, double zoomFactor );
		/**
		* Zooms in from having drawn \p zoomRect (which is in widget coordinates).
		*/
		void zoomIn( const QRectF & zoomRect );
		/**
		* Zooms out from havoutg drawn \p zoomRect (which is out widget
		* coordinates).
		*/
		void zoomOut( const QRectF & zoomRect );
		/**
		* Translates the view by \p dx, \p dy (in widget coordinates).
		*/
		void translateView( int dx, int dy );
		/**
		* Finds the plot (if any) under the last mouse pos as recorded by
		* updateCrosshairPosition(). This sets csmode, cstype, csparam. If no plot
		* was found, then csmode is set to -1.
		 * \return the function position of the closest plot if one was found.
		*/
		QPointF getPlotUnderMouse();
		/**
		* Finds the closest point to \p pos (which is in real coordinates) to
		 * the given function.
		* \return the parametization (angle, x or t) that gives the closest
		 * point.
		*/
		double getClosestPoint( const QPointF & pos, const Plot & plot );
		/**
		* Calculates the pixel distance from \p pos to the display point of the
		* given function at \p x.
		*/
		double pixelDistance( const QPointF & pos, const Plot & plot, double x, bool updateFunction );
		/**
		 * \param overlapEdge whether to give values that are slightly either
		 * side of the view; this is useful for thick pens
		 * \return an appropriate xmin value for the given function
		 * plotting.
		 */
		double getXmin( Function * function, bool overlapEdge = false );
		/**
		 * \param overlapEdge whether to give values that are slightly either
		 * side of the view; this is useful for thick pens
		 * \return an appropriate xmax value for the given function for
		 * plotting.
		 */
		double getXmax( Function * function, bool overlapEdge = false );
		
		/**
		 * How to behave in the *ToPixel functions.
		 */
		enum ClipBehaviour
		{
			ClipAll,		///< Clips any points going over the edge of the diagram
			ClipInfinite	///< Clips only infinite and NaN points going over the edge
		};
		/**
		 * @name Transformations
		 * These functions convert real coordinates to pixel coordinates and vice
		 * versa.
		 */
		double xToPixel( double x, ClipBehaviour clipBehaviour = ClipAll, double xIfNaN = 0 );
		double yToPixel( double y, ClipBehaviour clipBehaviour = ClipAll, double yIfNaN = 0 );
		QPointF toPixel( const QPointF & real, ClipBehaviour clipBehaviour = ClipAll, const QPointF & pixelIfNaN = QPointF() );
		double xToReal( double x );
		double yToReal( double y );
		QPointF toReal( const QPointF & pixel );
		bool xclipflg;	///< clipflg is set to 1 if the plot is out of the plot area.
		bool yclipflg;	///< clipflg is set to 1 if the plot is out of the plot area.
		/**
		 * Contains the settings for drawing the area under a graph (when
		 * calculating the area from function tools.
		 */
		IntegralDrawSettings m_integralDrawSettings;
		/**
		 * Separation distance between the grid lines.
		 */
		Value ticSepX, ticSepY;
		/**
		 * Positions of the first grid line.
		 */
		double ticStartX, ticStartY;
	
		QPointF m_crosshairPixelCoords;
		QPointF m_crosshairPosition;	///< in real coordinates
	
		/**
		 * The t- or x- (angle) coordinate of the traced curve - when tracing a
		 * polar or parametric curve.
		 */
		double m_trace_x;
		/**
		 * When tracing a Cartesian plot and the trace position nears the
		 * x-axis, an attempt to find a root will be found. If found, this will
		 * be set to true, and no further attempts will be made at finding a
		 * root. Once the plot position moves away from the x-axis again, this
		 * will be set to false.
		 */
		bool m_haveRoot;

		/// @return whether cspos is in the range of the view or in the custom range for the given \p plot
		bool crosshairPositionValid( Function * plot ) const;
		
		/// represents the Printer options set by user in the Print Dialog
		/// @see KPrinterDlg
		bool m_printHeaderTable;
		bool m_printBackground;
		double m_printWidth;
		double m_printHeight;
		/// if stop_calculating is true, the user has canceled drawing of an integral graph
		bool m_stopCalculating;
		/// the background color of the graph
		QColor m_backgroundColor;
		///buffer the current window so all functions don't need to be re-drawed
		QPixmap buffer;
		/// the popup menu
		QMenu *m_popupMenu;
		/// The pointer to the popup menu's title
		QAction *m_popupMenuTitle;
		/// is set to true if an integral is calculated
		bool m_isDrawing;
		/**
		 * Describes the state of the popup menu.
		 */
		enum PopupStatus
		{
			NoPopup,
			Popup,
			PopupDuringTrace
		};
		///status of the popup menu
		PopupStatus m_popupMenuStatus;
		/// False if KmPlot is started as a program, otherwise true
		bool const m_readonly;
		/// For drawing diagram labels
		QFont m_labelFont;
		/**
		 * The resolution of label positioning.
		 */
		static const int LabelGridSize = 50;
		/**
		 * Indicate which parts of the diagram have content (e.g. axis or
		 * plots), so that they can be avoided when drawing diagram labels
		 */
		bool m_usedDiagramArea[LabelGridSize][LabelGridSize];
		/**
		 * Marks the given diagram rectangle (in screen coords) as 'used'.
		 */
		void markDiagramAreaUsed( const QRectF & rect );
		/**
		 * Marks the given diagram point (in screen coords) as 'used'.
		 */
		void markDiagramPointUsed( const QPointF & point );
		/**
		 * \return the m_usedDiagramArea coords for the screen rect.
		 */
		QRect usedDiagramRect( const QRectF & rect ) const;
		/**
		 * \return the cost of occupying the given rectangle (as in whether it
		 * overlaps other diagram content, etc).
		 */
		int rectCost( QRectF rect ) const;
	
		enum ZoomMode
		{
			Normal,				///< no zooming
			AnimatingZoom,		///< animating a current zooming
			ZoomIn,				///< zoom in
			ZoomOut,			///< zoom out
			ZoomInDrawing,		///< drawing a rectangle for zooming in
			ZoomOutDrawing,		///< drawing a rectangle for zooming out
			AboutToTranslate,	///< user has clicked on an empty spot, but hasn't moved the mouse yet
			Translating			///< dragging the view with the mouse
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
		
		/**
		 * The rectangle (in painter, and hence pixel, coordinates) that the
		 * plots must be in. This is also the size of the image being drawn to,
		 * since the painter remains untransformed.
		 */
		QRect m_clipRect;
		/**
		 * This matrix transforms from real coordinates to painter coordinates.
		 * (Note that the painter does not have any transformation applied).
		 */
		QMatrix m_realToPixel;
		/**
		 * The inverse matrix of m_realToPixel; it maps from pixel coordinates
		 * to real X-Y coordinates.
		 */
		QMatrix m_pixelToReal;
	
		QString m_statusBarText[4];
		
		enum Cursor { CursorWait, CursorBlank, CursorArrow, CursorCross, CursorMagnify, CursorLessen, CursorMove };
		Cursor m_prevCursor;
		
		static View * m_self;
		
		QTextEdit * m_textEdit; ///< Contains m_textDocument
		QTextDocument * m_textDocument; ///< Used for layout of axis labels
};

#endif // View_included
