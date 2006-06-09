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

// Qt includes
#include <qbitmap.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <QApplication>
#include <QPicture>
#include <qslider.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <QPixmap>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <QTime>
#include <QMouseEvent>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kprogressbar.h>

// local includes
#include "functioneditor.h"
#include "kminmax.h"
#include "settings.h"
#include "ksliderwindow.h"
#include "MainDlg.h"

#include "View.h"
#include "viewadaptor.h"


// other includes
#include <assert.h>
#include <cmath>


//BEGIN class View
View * View::m_self = 0;

View::View( bool readOnly, bool & modified, KMenu * functionPopup, QWidget* parent, KActionCollection *ac )
	: QWidget( parent, Qt::WStaticContents ),
	  buffer( width(), height() ),
	  m_popupmenu( functionPopup ),
	  m_modified( modified ),
	  m_readonly( readOnly ),
	  m_ac(ac)
{
	assert( !m_self ); // this class should only be constructed once
	m_self = this;

	m_drawIntegral = false;
	m_width = m_height = 0.0;
	m_scaler = 0.0;
	m_haveRoot = false;
	tlgx = tlgy = drskalx = drskaly = 0.0;
	m_ymin = 0.0;
	m_ymax = 0.0;
	m_trace_x = 0.0;
	m_printHeaderTable = false;
	stop_calculating = false;
	m_minmax = 0;
	m_isDrawing = false;
	m_popupmenushown = 0;
	m_zoomMode = Normal;
	m_prevCursor = CursorArrow;

	m_mousePressTimer = new QTime();

    new ViewAdaptor(this);
    QDBus::sessionBus().registerObject("/view", this);

	XParser::self( & modified );
	init();
	getSettings();

	setMouseTracking( true );
	m_sliderWindow = 0;
	updateSliders();

	/// \todo fix title on popup menu (should display function name)
// 	m_popupmenu->addTitle( " " );
}

void View::setMinMaxDlg(KMinMax *minmaxdlg)
{
	m_minmax = minmaxdlg;
}

View::~View()
{
	delete XParser::self();
	delete m_mousePressTimer;
}


void View::draw( QPaintDevice * dev, PlotMedium medium )
{
// 	kDebug() << k_funcinfo << endl;

	double lx, ly;
	double sf;
	QRect rc;
	QPainter DC;    // our painter
	DC.begin(dev);    // start painting widget
	rc=DC.viewport();
	m_width = rc.width();
	m_height = rc.height();

	switch ( medium )
	{
		case Screen:
		{
			QPointF ref(120, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			DC.scale( m_height/(ly+2*ref.y()), m_height/(ly+2*ref.y()));
			if ( ( QPointF(lx+2*ref.x(), ly) * DC.matrix() ).x() > DC.viewport().right())
			{
				DC.resetMatrix();
				DC.scale( m_width/(lx+2*ref.x()), m_width/(lx+2*ref.x()));
			}
			wm = DC.matrix();
			m_scaler=( QPoint(1000, 0) * DC.matrix() ).x()/1000.;
			CDiagr::self()->Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			break;
		}

		case Printer:
		{
			sf=72./254.;        // 72dpi
			QPointF ref(100, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			DC.scale(sf, sf);
			m_scaler = 1.;
			m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1";
			drawHeaderTable( &DC );
			CDiagr::self()->Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			if ( ( (KPrinter* )dev )->option( "app-kmplot-printbackground" ) == "-1" )
				DC.fillRect( CDiagr::self()->frame(),  m_backgroundColor); //draw a colored background
			//DC.end();
			//((QPixmap *)dev)->fill(QColor("#FF00FF"));
			//DC.begin(dev);
			break;
		}

		case SVG:
		{
			QPointF ref(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			CDiagr::self()->Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			DC.translate(-CDiagr::self()->frame().left(), -CDiagr::self()->frame().top());
			m_scaler=1.;
			break;
		}

		case Pixmap:
		{
			sf=180./254.;								// 180dpi
			QPointF ref(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			CDiagr::self()->Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			DC.end();
			*((QPixmap *)dev) = QPixmap( (int)(CDiagr::self()->frame().width()*sf), (int)(CDiagr::self()->frame().height()*sf) );
			((QPixmap *)dev)->fill(m_backgroundColor);
			DC.begin(dev);
			DC.translate(-CDiagr::self()->frame().left()*sf, -CDiagr::self()->frame().top()*sf);
			DC.scale(sf, sf);
			m_scaler=1.;
			break;
		}
	}

	CDiagr::self()->updateSettings();
	CDiagr::self()->Skal( tlgx, tlgy );

	DC.setRenderHint( QPainter::Antialiasing, true );
	CDiagr::self()->Plot(&DC);

	area=DC.matrix().mapRect( CDiagr::self()->plotArea() );

	m_isDrawing=true;
	updateCursor();
	stop_calculating = false;

	// Antialiasing slows down rendering a lot, so turn it off if we are
	// sliding the view about
	DC.setRenderHint( QPainter::Antialiasing, m_zoomMode != Translating );
// 	DC.setRenderHint( QPainter::Antialiasing, false );
// 	if ( m_zoomMode != Translating )
// 		kDebug() << "##############################\n";
	DC.setClipping( true );
	DC.setClipRect( CDiagr::self()->plotArea() );
	foreach ( Function * ufkt, XParser::self()->m_ufkt )
	{
		if ( stop_calculating )
			break;

		plotFunction(ufkt, &DC);
	}
	DC.setClipping( false );

	m_isDrawing=false;
	updateCursor();
	DC.end();   // painting done
}


double View::value( const Plot & plot, int eq, double x, bool updateParameter )
{
	Function * function = plot.function();
	assert( function );

	double dx = (m_xmax-m_xmin)/area.width();

	if ( updateParameter )
		plot.updateFunctionParameter();

	Equation * equation = function->eq[ eq ];

	switch ( plot.plotMode )
	{
		case Function::Derivative0:
			return XParser::self()->fkt( equation, x );

		case Function::Derivative1:
			return XParser::self()->derivative( 1, equation, x, dx );

		case Function::Derivative2:
			return XParser::self()->derivative( 2, equation, x, dx );

		case Function::Integral:
			return XParser::self()->integral( equation, x, dx );
	}

	kWarning() << k_funcinfo << "Unknown mode!\n";
	return 0.0;
}


QPointF View::realValue( const Plot & plot, double x, bool updateParameter )
{
	Function * function = plot.function();
	assert( function );

	switch ( function->type() )
	{
		case Function::Cartesian:
		{
			double y = value( plot, 0, x, updateParameter );
			return QPointF( x, y );
		}

		case Function::Polar:
		{
			double y = value( plot, 0, x, updateParameter );
			return QPointF( y * cos(x), y * sin(x) );
		}

		case Function::Parametric:
		{
			double X = value( plot, 0, x, updateParameter );
			double Y = value( plot, 1, x, updateParameter );
			return QPointF( X, Y );
		}
	}

	kWarning() << k_funcinfo << "Unknown function type!\n";
	return QPointF();
}


double View::getXmin( Function * function )
{
	double min = function->dmin.value();

	if ( !function->usecustomxmin )
	{
		switch ( function->type() )
		{
			case Function::Polar:
				min = 0.0;
				break;

			case Function::Parametric:
				min = -M_PI;
				break;

			case Function::Cartesian:
				min = m_xmin;
				break;
		}
	}

	if ( (function->type() == Function::Cartesian) && (min < m_xmin) )
		min = m_xmin;

	return min;
}


double View::getXmax( Function * function )
{
	double max = function->dmax.value();

	if ( !function->usecustomxmax )
	{
		switch ( function->type() )
		{
			case Function::Polar:
				max = 2.0*M_PI;
				break;

			case Function::Parametric:
				max = M_PI;
				break;

			case Function::Cartesian:
				max = m_xmax;
				break;
		}
	}

	if ( (function->type() == Function::Cartesian) && (max > m_xmax) )
		max = m_xmax;

	return max;
}


void View::plotFunction(Function *ufkt, QPainter *pDC)
{
	double dmin = getXmin( ufkt );
	double dmax = getXmax( ufkt );

	double base_dx = (dmax-dmin)/area.width();
	if ( (ufkt->type() == Function::Parametric) || (ufkt->type() == Function::Polar) )
		base_dx *= 0.01;

	// Increase speed while translating the view
	bool quickDraw = ( m_zoomMode == Translating );
	if ( quickDraw )
		base_dx *= 4.0;

	double dx = base_dx;

	const QList< Plot > plots = ufkt->allPlots();
	foreach ( Plot plot, plots )
	{
		plot.updateFunctionParameter();

		pDC->setPen( penForPlot( plot, pDC->renderHints() & QPainter::Antialiasing ) );

		bool drawIntegral = m_drawIntegral && (m_integralDrawSettings.plot == plot);

		int mflg=2;
		double x = dmin;
		dx = base_dx;
		if ( plot.plotMode == Function::Integral )
		{
			if ( ufkt->integral_use_precision )
// 					if ( Settings::useRelativeStepWidth() )
				dx =  ufkt->integral_precision*(dmax-dmin)/area.width();
// 					else
// 						dx =  ufkt->integral_precision;
		}

		double totalLength = 0.0; // total pixel length; used for drawing dotted lines

		QPointF p1, p2;

		while ( x>=dmin && x<=dmax )
		{
			p2 = CDiagr::self()->toPixel( realValue( plot, x, false ), CDiagr::ClipInfinite );

			bool dxAtMinimum = (dx <= base_dx*(5e-5));
			bool dxAtMaximum = (dx >= base_dx*(5e+1));
			bool dxTooBig = false;
			bool dxTooSmall = false;

			if ( CDiagr::self()->xclipflg || CDiagr::self()->yclipflg )
			{
				p1=p2;
			}
			else
			{
				QPointF p1_pixel = p1 * pDC->matrix();
				QPointF p2_pixel = p2 * pDC->matrix();
				QRectF bound = QRectF( p1_pixel, QSizeF( (p2_pixel-p1_pixel).x(), (p2_pixel-p1_pixel).y() ) ).normalized();
				double length = QLineF( p1_pixel, p2_pixel ).length();
				totalLength += length;

				if ( (mflg<=1) && ((ufkt->type() == Function::Parametric) || (ufkt->type() == Function::Polar)) )
				{
					if ( QRectF( area ).intersects( bound ) )
					{
						dxTooBig = !dxAtMinimum && (length > (quickDraw ? 40.0 : 4.0));
						dxTooSmall = !dxAtMaximum && (length < (quickDraw ? 10.0 : 1.0));
					}
					else
						dxTooSmall = !dxAtMaximum;
				}

				if ( !dxTooBig )
				{
					if(mflg<=1)
					{
						if ( drawIntegral && (x >= m_integralDrawSettings.dmin) && (x <= m_integralDrawSettings.dmax) )
						{
							double y0 = CDiagr::self()->yToPixel( 0 );

							QPointF points[4];
							points[0] = QPointF( p1.x(), y0 );
							points[1] = QPointF( p2.x(), y0 );
							points[2] = QPointF( p2.x(), p2.y() );
							points[3] = QPointF( p1.x(), p1.y() );

							pDC->drawPolygon( points, 4 );
// 								pDC->drawRect( QRectF( p1, QSizeF( p2.x()-p1.x(), CDiagr::self()->yToPixel( 0 ) - p2.y() ) ) );
						}
						else
						{
							if ( penShouldDraw( totalLength, plot ) )
								pDC->drawLine( p1, p2 );
						}
					}
					p1=p2;
				}
				mflg=0;
			}

			if ( dxTooBig )
				dx *= 0.5;
			else
			{
				if ( dxTooSmall )
					dx *= 2.0;
				x=x+dx;
			}
		}

#if 0
		// Draw the stationary points if the user has requested them to be shown
		if ( ufkt->type() == Function::Cartesian )
		{
			pDC->setPen( QPen( Qt::black, 10 ) );

			QList<QPointF> stationaryPoints = findStationaryPoints( p_mode, ufkt->eq[0] );
			foreach ( QPointF realValue, stationaryPoints )
				pDC->drawPoint( CDiagr::self()->toPixel( realValue ) );
		}
#endif
	}
}


// does for real numbers what "%" does for integers
double realModulo( double x, double mod )
{
	x = qAbs(x);
	mod = qAbs(mod);
	return x - floor(x/mod)*mod;
}


bool View::penShouldDraw( double length, const Plot & plot )
{
	// Always use a solid line when translating the view
	if ( m_zoomMode == Translating )
		return true;

	Function * function = plot.function();

	Qt::PenStyle style = function->plotAppearance( plot.plotMode ).style;

	double sepBig = 8.0;	// separation distance between dashes
	double sepMid = 7.0;	// separation between a dash and a dot
	double sepSmall = 6.5;	// separation distance between dots
	double dash = 9.0;		// length of a dash
	double dot = 3.5;		// length of a dot

	switch ( style )
	{
		case Qt::NoPen:
			// *whatever*...
			return false;

		case Qt::SolidLine:
			return true;

		case Qt::DashLine:
			return realModulo( length, dash + sepBig ) < dash;

		case Qt::DotLine:
			return realModulo( length, dot + sepSmall ) < dot;

		case Qt::DashDotLine:
		{
			double at = realModulo( length, dash + sepMid + dot + sepMid );

			if ( at < dash )
				return true;
			if ( at < (dash + sepMid) )
				return false;
			if ( at < (dash + sepMid + dot) )
				return true;
			return false;
		}

		case Qt::DashDotDotLine:
		{
			double at = realModulo( length, dash + sepMid + dot + sepSmall + dot + sepMid );

			if ( at < dash )
				return true;
			if ( at < (dash + sepMid) )
				return false;
			if ( at < (dash + sepMid + dot) )
				return true;
			if ( at < (dash + sepMid + dot + sepSmall) )
				return false;
			if ( at < (dash + sepMid + dot + sepSmall + dot) )
				return true;
			return false;
		}

		case Qt::MPenStyle:
		case Qt::CustomDashLine:
		{
			assert( ! "Don't know how to handle this style!" );
			return true;
		}
	}

	assert( ! "Unknown pen style!" );
	return true;
}


QPen View::penForPlot( const Plot & plot, bool antialias ) const
{
	QPen pen;
	if ( m_zoomMode == Translating )
	{
		// plot style is always a solid line when translating the view
		pen.setCapStyle( Qt::FlatCap );
	}
	else
	{
		pen.setCapStyle( Qt::RoundCap );
		// (the style will be set back to FlatCap if the plot style is a solid line)
	}

	Function * ufkt = plot.function();

	double lineWidth_mm = ufkt->plotAppearance( plot.plotMode ).lineWidth;
	pen.setColor( ufkt->plotAppearance( plot.plotMode ).color );
	if ( ufkt->plotAppearance( plot.plotMode ).style == Qt::SolidLine )
		pen.setCapStyle( Qt::FlatCap );

	double width = mmToPenWidth( lineWidth_mm, antialias );
	if ( (width*m_scaler < 3) && !antialias )
	{
		// Plots in general are curvy lines - so if a plot is drawn with a QPen
		// of width 1 or 2, then we will get a patchy, twinkling line. So set
		// the width to zero (i.e. a cosmetic pen).
		width = 0;
	}

	pen.setWidthF( width );

	return pen;
}


double View::mmToPenWidth( double width_mm, bool antialias ) const
{
	/// \todo ensure that this line width setting works for mediums other than screen

	double w = width_mm*10.0;
	if ( !antialias )
	{
		// To avoid a twinkling, patchy line, have to adjust the pen width
		// so that after scaling by QPainter (by a factor of s), it is of an
		// integer width.
		w = std::floor(w*m_scaler)/m_scaler;
	}

	return w;
}


void View::drawHeaderTable(QPainter *pDC)
{
	QString alx, aly, atx, aty, dfx, dfy;

	if( m_printHeaderTable )
	{
		pDC->translate(250., 150.);
		pDC->setPen(QPen(Qt::black, (int)(5.*m_scaler)));
		pDC->setFont(QFont( Settings::headerTableFont(), 30) );
		puts( Settings::headerTableFont().toLatin1().data() );
		QString minStr = Settings::xMin();
		QString maxStr = Settings::xMax();
		getMinMax( Settings::xRange(), minStr, maxStr);
		alx="[ "+minStr+" | "+maxStr+" ]";
		minStr = Settings::yMin();
		maxStr = Settings::yMax();
		getMinMax( Settings::yRange(), minStr, maxStr);
		aly="[ "+minStr+" | "+maxStr+" ]";
		setpi(&alx);
		setpi(&aly);
		atx="1E  =  "+tlgxstr;
		setpi(&atx);
		aty="1E  =  "+tlgystr;
		setpi(&aty);
		dfx="1E  =  "+drskalxstr+" cm";
		setpi(&dfx);
		dfy="1E  =  "+drskalystr+" cm";
		setpi(&dfy);

		pDC->drawRect(0, 0, 1500, 230);
		pDC->Lineh(0, 100, 1500);
		pDC->Linev(300, 0, 230);
		pDC->Linev(700, 0, 230);
		pDC->Linev(1100, 0, 230);

		pDC->drawText(0, 0, 300, 100, Qt::AlignCenter, i18n("Parameters:"));
		pDC->drawText(300, 0, 400, 100, Qt::AlignCenter, i18n("Plotting Area"));
		pDC->drawText(700, 0, 400, 100, Qt::AlignCenter, i18n("Axes Division"));
		pDC->drawText(1100, 0, 400, 100, Qt::AlignCenter, i18n("Printing Format"));
		pDC->drawText(0, 100, 300, 65, Qt::AlignCenter, i18n("x-Axis:"));
		pDC->drawText(0, 165, 300, 65, Qt::AlignCenter, i18n("y-Axis:"));
		pDC->drawText(300, 100, 400, 65, Qt::AlignCenter, alx);
		pDC->drawText(300, 165, 400, 65, Qt::AlignCenter, aly);
		pDC->drawText(700, 100, 400, 65, Qt::AlignCenter, atx);
		pDC->drawText(700, 165, 400, 65, Qt::AlignCenter, aty);
		pDC->drawText(1100, 100, 400, 65, Qt::AlignCenter, dfx);
		pDC->drawText(1100, 165, 400, 65, Qt::AlignCenter, dfy);

		pDC->drawText(0, 300, i18n("Functions:"));
		pDC->Lineh(0, 320, 700);
		int ypos = 380;
		foreach ( Function * it, XParser::self()->m_ufkt )
		{
			for ( unsigned i = 0; i < 2; ++ i )
			{
				if ( stop_calculating )
					break;

				if ( !it->eq[i] )
					continue;

				QString fstr = it->eq[i]->fstr();
				if ( fstr.isEmpty() )
					continue;

				pDC->drawText( 100, ypos, fstr );
				ypos+=60;
			}
		}
		pDC->translate(-60., ypos+100.);
	}
	else  pDC->translate(150., 150.);
}


void View::getMinMax( int koord, QString &mini, QString &maxi )
{
	switch(koord)
	{
	case 0:
		mini="-8.0";
		maxi="8.0";
		break;
	case 1:
		mini="-5.0";
		maxi="5.0";
		break;
	case 2:
		mini="0.0";
		maxi="16.0";
		break;
	case 3:
		mini="0.0";
		maxi="10.0";
	}
}


void View::setpi(QString *s)
{
	int i;
	QChar c(960);

	while((i=s->indexOf('p')) != -1)
		s->replace(i, 2, &c, 1);
}


QList< QPointF > View::findStationaryPoints( const Plot & plot )
{
	Plot plot2 = plot;

	switch ( plot.plotMode )
	{
		case Function::Integral:
			plot2.plotMode = Function::Derivative0;
			break;

		case Function::Derivative0:
			plot2.plotMode = Function::Derivative1;
			break;

		case Function::Derivative1:
			plot2.plotMode = Function::Derivative2;
			break;

		case Function::Derivative2:
			kWarning() << k_funcinfo << "Can't handle this yet!\n";
			break;
	}

	QList< double > roots = findRoots( plot2 );

	plot.updateFunctionParameter();
	QList< QPointF > stationaryPoints;
	foreach ( double x, roots )
		stationaryPoints << realValue( plot, x, false );

	return stationaryPoints;
}


QList< double > View::findRoots( const Plot & plot )
{
	Equation * eq = plot.function()->eq[0];

	double min = getXmin( eq->parent() );
	double max = getXmax( eq->parent() );

	double dx = 20*(max-min)/area.width();

	QList< double > roots;

	// Use this to detect finding the same root. This assumes that the same root
	// will be converged to in unbroken x-intervals
	double prevX = 0.0;

	for ( double x = min; x < max; x += dx )
	{
		double x0 = x;
		bool found = findRoot( & x0, plot );

		bool differentRoot = (qAbs(x0-prevX) > (dx/2)) || roots.isEmpty();

		if ( found && differentRoot )
		{
			roots << x0;
			prevX = x0;
		}
	}

	return roots;
}


bool View::findRoot( double *x0, const Plot & plot )
{
	int k = 0; // iteration count
	int max_k = 200; // maximum number of iterations
	double max_y = 1e-14; // the largest value of y which is deemed a root found

	double y = m_crosshairPosition.y();
	bool tooBig = true;

// 	kDebug() << "Initial: ("<<*x0<<","<<y<<")\n";

	int n = 1;
	switch ( plot.plotMode )
	{
		case Function::Derivative0:
			n += 0;
			break;
		case Function::Derivative1:
			n += 1;
			break;
		case Function::Derivative2:
			n += 2;
			break;
		case Function::Integral:
			n += -1;
			break;
	}

	Equation * eq = plot.function()->eq[0];
	plot.updateFunctionParameter();

	double dx;
	do
	{
		double df = XParser::self()->derivative( n, eq, *x0, (m_xmax-m_xmin)*1e-5 );
		if ( qAbs(df) < 1e-20 )
			df = 1e-20 * ((df < 0) ? -1 : 1);

		dx = y / df;
		*x0 -= dx;
		y = value( plot, 0, *x0, false );

// 		kDebug() << "k="<<k<<": ("<<*x0<<","<<y<<")\n";

		tooBig = (qAbs(y) > max_y);
	}
	while ( (k++<max_k) && ( tooBig || (qAbs(dx) > ((m_xmax-m_xmin)*1e-10)) ) );

	// We continue calculating until |y| < max_y; this may result in k reaching
	// max_k. However, if |y| is reasonably small (even if reaching max_k),
	// we consider it a root.
	return ( qAbs(y) < 1e-6 );
}

void View::paintEvent(QPaintEvent *)
{
	// Note: it is important to have this function call before we begin painting
	// as updateCrosshairPosition may set the statusbar text
	updateCrosshairPosition();

	QPainter p;
	p.begin(this);

	p.drawPixmap( QPoint( 0, 0 ), buffer );

	// the current cursor position in widget coordinates
	QPoint mousePos = mapFromGlobal( QCursor::pos() );

	if ( (m_zoomMode == ZoomInDrawing) || (m_zoomMode == ZoomOutDrawing) )
	{
		QPalette palette;
		QColor highlightColor = palette.color( QPalette::Highlight );
		QColor backgroundColor = highlightColor;
		backgroundColor.setAlpha( 63 );

		p.setPen( highlightColor );
		p.setBrush( backgroundColor );

		p.setBackgroundMode (Qt::OpaqueMode);
		p.setBackground (Qt::blue);

		QRect rect( m_zoomRectangleStart, mousePos );
		p.drawRect( rect );
	}
	else if ( m_zoomMode == AnimatingZoom )
	{
		p.save();
		p.setMatrix( wm );
		QPointF tl( CDiagr::self()->xToPixel( m_animateZoomRect.left() ), CDiagr::self()->yToPixel( m_animateZoomRect.top() ) );
		QPointF br( CDiagr::self()->xToPixel( m_animateZoomRect.right() ), CDiagr::self()->yToPixel( m_animateZoomRect.bottom() ) );
		p.drawRect( QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) ) );
		p.restore();
	}
	else if ( shouldShowCrosshairs() )
	{
		Function * it = m_currentPlot.function();

			// Fadenkreuz zeichnen [draw the cross-hair]
		QPen pen;
		if ( !it )
			pen.setColor(m_invertedBackgroundColor);
		else
		{
			pen.setColor( it->plotAppearance( m_currentPlot.plotMode ).color );

			if ( pen.color() == m_backgroundColor) // if the "Fadenkreuz" [cross-hair] has the same color as the background, the "Fadenkreuz" [cross-hair] will have the inverted color of background so you can see it easier
				pen.setColor(m_invertedBackgroundColor);
		}
		p.setPen( pen );
		p.Lineh( area.left(), m_crosshairPixelCoords.y(), area.right() );
		p.Linev( m_crosshairPixelCoords.x(), area.bottom(), area.top());
	}

	p.end();
}

void View::resizeEvent(QResizeEvent *)
{
	if (m_isDrawing) //stop drawing integrals
	{
		stop_calculating = true; //stop drawing
		return;
	}
	buffer = QPixmap( size() );
	drawPlot();
}


void View::drawPlot()
{
	if( m_minmax->isVisible() )
		m_minmax->updateFunctions();
	buffer.fill(m_backgroundColor);
	draw(&buffer, Screen );
	update();
}


void View::focusOutEvent( QFocusEvent * )
{
	// Redraw ourself to get rid of the crosshair (if we had it)...
	QTimer::singleShot( 0, this, SLOT(update()) );
	QTimer::singleShot( 0, this, SLOT(updateCursor()) );
}


void View::focusInEvent( QFocusEvent * )
{
	// Redraw ourself to get the crosshair (if we should have it)...
	QTimer::singleShot( 0, this, SLOT(update()) );
	QTimer::singleShot( 0, this, SLOT(updateCursor()) );
}


bool View::crosshairPositionValid( Function * plot ) const
{
	if ( !plot )
		return false;

	// only relevant for cartesian plots - assume true for none
	if ( plot->type() != Function::Cartesian )
		return true;

	bool lowerOk = ((!plot->usecustomxmin) || (plot->usecustomxmin && m_crosshairPosition.x()>plot->dmin.value()));
	bool upperOk = ((!plot->usecustomxmax) || (plot->usecustomxmax && m_crosshairPosition.x()<plot->dmax.value()));

	return lowerOk && upperOk;
}


void View::mousePressEvent(QMouseEvent *e)
{
	m_mousePressTimer->start();

	// In general, we want to update the view
	update();

	if ( m_popupmenushown>0)
		return;

	if (m_isDrawing)
	{
		stop_calculating = true; //stop drawing
		return;
	}

	if ( m_zoomMode != Normal )
	{
		// If the user clicked with the right mouse button will zooming in or out, then cancel it
		if ( (m_zoomMode == ZoomInDrawing) ||
					(m_zoomMode == ZoomOutDrawing) )
		{
			m_zoomMode = Normal;
		}
		updateCursor();
		return;
	}

	m_haveRoot = false;

	bool hadFunction = (m_currentPlot.functionID() != -1 );

	updateCrosshairPosition();

	if( !m_readonly && e->button()==Qt::RightButton) //clicking with the right mouse button
	{
		getPlotUnderMouse();
		Function * function = m_currentPlot.function();
		if ( function )
		{
			QString popupTitle( function->prettyName( m_currentPlot.plotMode ) );

			if ( hadFunction )
				m_popupmenushown = 2;
			else
				m_popupmenushown = 1;

			m_popupmenu->setTitle( popupTitle );
			m_popupmenu->exec( QCursor::pos() );
		}
		return;
	}

	if(e->button()!=Qt::LeftButton)
		return;

	if ( m_currentPlot.functionID() >= 0 ) //disable trace mode if trace mode is enable
	{
		m_currentPlot.setFunctionID( -1 );
		setStatusBar("",3);
		setStatusBar("",4);
		mouseMoveEvent(e);
		return;
	}

	QPointF closestPoint = getPlotUnderMouse();
	Function * function = m_currentPlot.function();
	if ( function )
	{
		QPointF ptd( CDiagr::self()->toPixel( closestPoint ) );
		QPoint globalPos = mapToGlobal( (ptd * wm).toPoint() );
		QCursor::setPos( globalPos );

		m_minmax->selectItem();
		setStatusBar( function->prettyName( m_currentPlot.plotMode ), 4 );

		return;
	}

	// user didn't click on a plot; so we prepare to enter translation mode
	m_currentPlot.setFunctionID( -1 );
	m_zoomMode = AboutToTranslate;
	m_prevDragMousePos = e->pos();
	updateCursor();
}


QPointF View::getPlotUnderMouse()
{
	m_currentPlot.setFunctionID( -1 );
	m_trace_x = 0.0;

	Plot bestPlot;

	int best_id = -1;
	double best_distance = 1e30; // a nice large number
	QPointF best_cspos;

	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		const QList< Plot > plots = it->allPlots();
		foreach ( Plot plot, plots )
		{
			plot.updateFunctionParameter();

			double best_x = getClosestPoint( m_crosshairPosition, plot );
			double distance = pixelDistance( m_crosshairPosition, plot, best_x, false );

			if ( distance < best_distance )
			{
				best_distance = distance;
				bestPlot = plot;
				m_trace_x = best_x;
				best_cspos = realValue( plot, best_x, false );
			}
		}
	}

	if ( best_distance < 30.0 )
	{
		m_currentPlot = bestPlot;
		m_crosshairPosition = best_cspos;
		return m_crosshairPosition;
	}
	else
		return QPointF();
}


double View::getClosestPoint( const QPointF & pos, const Plot & plot )
{
	plot.updateFunctionParameter();

	double best_x = 0.0;

	Function * function = plot.function();

	if ( function->type() == Function::Cartesian )
	{
		double best_pixel_x = 0.0;

		QPointF pixelPos = CDiagr::self()->toPixel( pos, CDiagr::ClipInfinite );

		double dmin = getXmin( function );
		double dmax = getXmax( function );

		double stepSize = (m_xmax-m_xmin)/area.width();

		// Algorithm in use here: Work out the shortest distance between the
		// line joining (x0,y0) to (x1,y1) and the given point (real_x,real_y)

		double x = dmin;
		double y0 = value( plot, 0, x, false );

		double best_distance = 1e20; // a large distance

		while ( x <= dmax )
		{
			x += stepSize;

			double y1 = XParser::self()->fkt( function->eq[0], x );

			double _x0 = CDiagr::self()->xToPixel( x-stepSize, CDiagr::ClipInfinite );
			double _x1 = CDiagr::self()->xToPixel( x, CDiagr::ClipInfinite );

			double _y0 = CDiagr::self()->yToPixel( y0, CDiagr::ClipInfinite );
			double _y1 = CDiagr::self()->yToPixel( y1, CDiagr::ClipInfinite );

			double k = (_y1-_y0)/(_x1-_x0);

			double closest_x;
			if ( k == 0 )
				closest_x = _x0;
			else
				closest_x = (pixelPos.y() + pixelPos.x()/k + k*_x0 - _y0) / (k + 1.0/k);

			double closest_y = CDiagr::self()->yToPixel( value( plot, 0, CDiagr::self()->xToReal( closest_x ), false ), CDiagr::ClipInfinite );

			double dfx = qAbs( closest_x - pixelPos.x() );
			double dfy = qAbs( closest_y - pixelPos.y() );

			double distance = sqrt( dfx*dfx + dfy*dfy );
			if ( distance < best_distance )
			{
				best_distance = distance;
				best_pixel_x = closest_x;
			}
		}

		best_x = CDiagr::self()->xToReal( best_pixel_x );
	}
	else
	{
		double minX = getXmin( function );
		double maxX = getXmax( function );
		double stepSize = 0.01;

		while ( stepSize > 0.0000009 )
		{
			double best_distance = 1e20; // a large distance

			double x = minX;
			while ( x <= maxX )
			{
				double distance = pixelDistance( pos, plot, x, false );
				if ( distance < best_distance )
				{
					best_distance = distance;
					best_x = x;
				}

				x += stepSize;
			}

			minX = best_x - stepSize;
			maxX = best_x + stepSize;

			stepSize *= 0.1;
		}
	}

	return best_x;
}


double View::pixelDistance( const QPointF & pos, const Plot & plot, double x, bool updateFunctionParameter )
{
	QPointF f = realValue( plot, x, updateFunctionParameter );
	QPointF df = CDiagr::self()->toPixel( pos, CDiagr::ClipInfinite ) - CDiagr::self()->toPixel( f, CDiagr::ClipInfinite );

	return std::sqrt( df.x()*df.x() + df.y()*df.y() );
}


QString View::posToString( double x, double delta, PositionFormatting format, QColor color  ) const
{
	assert( delta != 0.0 );

	QString numberText;

	int decimalPlaces = 1-int(log(delta)/log(10.0));

	switch ( format )
	{
		case ScientificFormat:
		{
			int accuracy = 1 + decimalPlaces + int(log(qAbs(x))/log(10.0));
			if ( accuracy < 2 )
			{
				// Ensure a minimum of two significant digits
				accuracy = 2;
			}

			QString number = QString::number( x, 'g', accuracy );
			if ( number.contains( 'e' ) )
			{
				number.remove( "+0" );
				number.remove( "+" );
				number.replace( "-0", "-" );

				number.replace( 'e', QChar(215) + QString("10<sup>") );
				number.append( "</sup>" );
			}
			if ( x > 0.0 )
				number.prepend('+');

			numberText = QString("<html><body><span style=\"color:%1;\">").arg( color.name() ) + number + "</span></body></html>";

			break;
		}

		case DecimalFormat:
		{
			if ( decimalPlaces >= 0 )
				numberText = QString::number( x, 'f', decimalPlaces );
			else
				numberText = QString::number( x/(pow(10.0,decimalPlaces)), 'f', 0 ) + QString( -decimalPlaces, '0' );

			if ( x > 0.0 )
				numberText.prepend('+');

			break;
		}
	}

	return numberText;
}


void View::mouseMoveEvent(QMouseEvent *e)
{
	if ( m_isDrawing )
		return;

	bool inBounds = updateCrosshairPosition();
	if ( !m_haveRoot )
		setStatusBar("", 3);

	QString sx, sy;

	if ( inBounds )
	{
		sx = "x = " + posToString( m_crosshairPosition.x(), (m_xmax-m_xmin)/area.width(), View::DecimalFormat );
		sy = "y = " + posToString( m_crosshairPosition.y(), (m_ymax-m_ymin)/area.width(), View::DecimalFormat );
	}
	else
		sx = sy = "";

	setStatusBar(sx, 1);
	setStatusBar(sy, 2);

	if ( e->buttons() & Qt::LeftButton )
	{
		if ( m_zoomMode == ZoomIn )
		{
			m_zoomMode = ZoomInDrawing;
			m_zoomRectangleStart = e->pos();
		}
		else if ( m_zoomMode == ZoomOut )
		{
			m_zoomMode = ZoomOutDrawing;
			m_zoomRectangleStart = e->pos();
		}
		else if ( ((m_zoomMode == AboutToTranslate) || (m_zoomMode == Translating)) &&
						  (e->pos() != m_prevDragMousePos) )
		{
			m_zoomMode = Translating;
			QPoint d = m_prevDragMousePos - e->pos();
			m_prevDragMousePos = e->pos();
			translateView( d.x(), d.y() );
		}
	}

	if ( (m_zoomMode == Normal) &&
			 (m_popupmenushown > 0) &&
			 !m_popupmenu->isVisible() )
	{
		if ( m_popupmenushown==1)
			m_currentPlot.setFunctionID( -1 );
		m_popupmenushown = 0;
	}

	update();
	updateCursor();
}


bool View::updateCrosshairPosition()
{
	QPoint mousePos = mapFromGlobal( QCursor::pos() );

	bool out_of_bounds = false; // for the ypos

	QPointF ptl = mousePos * wm.inverted();
	m_crosshairPosition = CDiagr::self()->toReal( ptl );

	m_currentPlot.updateFunctionParameter();
	Function * it = m_currentPlot.function();

	if ( it && crosshairPositionValid( it ) )
	{
		// The user currently has a plot selected, with the mouse in a valid position

		if ( (it->type() == Function::Parametric) ||
					(it->type() == Function::Polar) )
		{

			// Should we increase or decrease t to get closer to the mouse?
			double dx[2] = { -0.00001, +0.00001 };
			double d[] = { 0.0, 0.0 };
			for ( int i = 0; i < 2; ++ i )
				d[i] = pixelDistance( m_crosshairPosition, m_currentPlot, m_trace_x + dx[i], false );

			double prev_best = pixelDistance( m_crosshairPosition, m_currentPlot, m_trace_x, false );
			double current_dx = dx[(d[0] < d[1]) ? 0 : 1]*1e3;

			while ( true )
			{
				double new_distance = pixelDistance( m_crosshairPosition, m_currentPlot, m_trace_x + current_dx, false );
				if ( new_distance < prev_best )
				{
					prev_best = new_distance;
					m_trace_x += current_dx;
				}
				else
				{
					if ( qAbs(current_dx) > 9e-10 )
						current_dx *= 0.1;
					else
						break;
				}
			}

			double min = getXmin( it );
			double max = getXmax( it );

			if ( m_trace_x > max )
				m_trace_x  = max;

			else if ( m_trace_x < min )
				m_trace_x = min;

			m_crosshairPosition = realValue( m_currentPlot, m_trace_x, false );
		}
		else
		{
			// cartesian plot

			m_crosshairPosition.setY( value( m_currentPlot, 0, m_crosshairPosition.x(), false ) );
			ptl.setY(CDiagr::self()->yToPixel( m_crosshairPosition.y() ));

			if ( m_crosshairPosition.y()<m_ymin || m_crosshairPosition.y()>m_ymax) //the ypoint is not visible
			{
				out_of_bounds = true;
			}
			else if(fabs(CDiagr::self()->yToReal(ptl.y())) < (m_ymax-m_ymin)/80)
			{
				double x0 = m_crosshairPosition.x();
				if ( !m_haveRoot && findRoot( &x0, m_currentPlot ) )
				{
					QString str="  ";
					str+=i18n("root");
					setStatusBar(str+QString().sprintf(":  x0 = %+.5f", x0), 3);
					m_haveRoot=true;
				}
			}
			else
				m_haveRoot=false;
		}

		// For Cartesian plots, only adjust the cursor position if it is not at the ends of the view
		if ( (it->type() != Function::Cartesian) || area.contains( mousePos ) )
		{
			ptl = CDiagr::self()->toPixel( m_crosshairPosition );
			QPoint globalPos = mapToGlobal( (ptl * wm).toPoint() );
			QCursor::setPos( globalPos );
		}
	}

	m_crosshairPixelCoords = ptl * wm;

	return !out_of_bounds && area.contains( mousePos );
}


void View::mouseReleaseEvent ( QMouseEvent * e )
{
	bool doDrawPlot = false;

	// avoid zooming in if the zoom rectangle is very small and the mouse was
	// just pressed, which suggests that the user dragged the mouse accidently
	QRect zoomRect = QRect( m_zoomRectangleStart, e->pos() ).normalized();
	int area = zoomRect.width() * zoomRect.height();

	if ( (area <= 500) && (m_mousePressTimer->elapsed() < 100) )
	{
		if ( m_zoomMode == ZoomInDrawing )
			m_zoomMode = ZoomIn;
		else if ( m_zoomMode == ZoomOutDrawing )
			m_zoomMode = ZoomOut;
	}

	switch ( m_zoomMode )
	{
		case Normal:
		case AnimatingZoom:
		case AboutToTranslate:
			break;

		case Translating:
			doDrawPlot = true;
			break;

		case ZoomIn:
			zoomIn( e->pos(), double(Settings::zoomInStep())/100.0 );
			break;

		case ZoomOut:
			zoomIn( e->pos(), (double(Settings::zoomOutStep())/100.0) + 1.0 );
			break;

		case ZoomInDrawing:
			zoomIn( zoomRect );
			break;

		case ZoomOutDrawing:
			zoomOut( zoomRect );
			break;
	}

	m_zoomMode = Normal;

	if ( doDrawPlot )
		drawPlot();
	else
		update();

	updateCursor();
}


void View::zoomIn( const QPoint & mousePos, double zoomFactor )
{
	double realx = CDiagr::self()->xToReal((mousePos * wm.inverted()).x());
	double realy = CDiagr::self()->yToReal((mousePos * wm.inverted()).y());

	double diffx = (m_xmax-m_xmin)*zoomFactor;
	double diffy = (m_ymax-m_ymin)*zoomFactor;

	if ( diffx < 1e-8 || diffy < 1e-8 )
		return;

	animateZoom( QRectF( realx-diffx, realy-diffy, 2.0*diffx, 2.0*diffy ) );
}


void View::zoomIn( const QRect & zoomRect )
{
	QRect rect = wm.inverted().mapRect( zoomRect );

	QPoint p = rect.topLeft();
	double real1x = CDiagr::self()->xToReal(p.x() );
	double real1y = CDiagr::self()->yToReal(p.y() );
	p = rect.bottomRight();
	double real2x = CDiagr::self()->xToReal(p.x() );
	double real2y = CDiagr::self()->yToReal(p.y() );

	if ( real1x > real2x )
		qSwap( real1x, real2x );
	if ( real1y > real2y )
		qSwap( real1y, real2y );

	//setting new x-boundaries
	if ( real2x - real1x < 1e-8 )
		return;
	if ( real2y - real1y < 1e-8 )
		return;

	animateZoom( QRectF( QPointF( real1x, real1y ), QSizeF( real2x-real1x, real2y-real1y ) ) );
}


void View::zoomOut( const QRect & zoomRect )
{
	QRect rect = wm.inverted().mapRect( zoomRect );

	QPoint p = rect.topLeft();
	double _real1x = CDiagr::self()->xToReal(p.x() );
	double _real1y = CDiagr::self()->yToReal(p.y() );
	p = rect.bottomRight();
	double _real2x = CDiagr::self()->xToReal(p.x() );
	double _real2y = CDiagr::self()->yToReal(p.y() );

	double kx = (_real1x-_real2x)/(m_xmin-m_xmax);
	double lx = _real1x - (kx * m_xmin);

	double ky = (_real1y-_real2y)/(m_ymax-m_ymin);
	double ly = _real1y - (ky * m_ymax);

	double real1x = (m_xmin-lx)/kx;
	double real2x = (m_xmax-lx)/kx;

	double real1y = (m_ymax-ly)/ky;
	double real2y = (m_ymin-ly)/ky;

	animateZoom( QRectF( QPointF( real1x, real1y ), QSizeF( real2x-real1x, real2y-real1y ) ) );
}


void View::animateZoom( const QRectF & _newCoords )
{
	QRectF oldCoords( m_xmin, m_ymin, m_xmax-m_xmin, m_ymax-m_ymin );
	QRectF newCoords( _newCoords.normalized() );

	if ( oldCoords == newCoords )
		return;

	m_zoomMode = AnimatingZoom;

	double oldCoordsArea = oldCoords.width() * oldCoords.height();
	double newCoordsArea = newCoords.width() * newCoords.height();

	QPointF beginTL, beginBR, endTL, endBR;

	if ( oldCoordsArea > newCoordsArea )
	{
		// zooming in
		beginTL = newCoords.topLeft();
		beginBR = newCoords.bottomRight();
		endTL = oldCoords.topLeft();
		endBR = oldCoords.bottomRight();
	}
	else
	{
		// zooming out
		beginTL = oldCoords.topLeft();
		beginBR = oldCoords.bottomRight();

		double kx = ( oldCoords.left() - oldCoords.right() ) / ( newCoords.left() - newCoords.right() );
		double ky = ( oldCoords.top() - oldCoords.bottom() ) / ( newCoords.top() - newCoords.bottom() );

		double lx = oldCoords.left() - (kx * newCoords.left());
		double ly = oldCoords.top() - (ky * newCoords.top());

		endTL = QPointF( (kx * oldCoords.left()) + lx, (ky * oldCoords.top()) + ly );
		endBR = QPointF( (kx * oldCoords.right()) + lx, (ky * oldCoords.bottom()) + ly );
	}

	double MAX = 10;
	double ms = MAX*16; // milliseconds to animate for

	for ( int i = 0; i <= MAX; ++i )
	{
		QTime t;
		t.start();

		QPointF tl = (( i*endTL) + ((MAX-i)*beginTL)) / MAX;
		QPointF br = (( i*endBR) + ((MAX-i)*beginBR)) / MAX;

		m_animateZoomRect = QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) );

		repaint();

		if ( i == MAX )
			break;
		else while ( t.elapsed() < (ms/MAX) )
			; // do nothing
	}

	m_xmin = newCoords.left();
	m_xmax = newCoords.right();
	m_ymin = newCoords.top();
	m_ymax = newCoords.bottom();

	Settings::setXMin( Parser::number( m_xmin ) );
	Settings::setXMax( Parser::number( m_xmax ) );
	Settings::setYMin( Parser::number( m_ymin ) );
	Settings::setYMax( Parser::number( m_ymax ) );

	Settings::setXRange(4); //custom x-range
	Settings::setYRange(4); //custom y-range

	setScaling();

	drawPlot(); //update all graphs

	m_zoomMode = Normal;
}


void View::translateView( int dx, int dy )
{
	double rdx = CDiagr::self()->xToReal( dx / m_scaler ) - CDiagr::self()->xToReal( 0.0 );
	double rdy = CDiagr::self()->yToReal( dy / m_scaler ) - CDiagr::self()->yToReal( 0.0 );

	m_xmin += rdx;
	m_xmax += rdx;
	m_ymin += rdy;
	m_ymax += rdy;

	Settings::setXMin( Parser::number( m_xmin ) );
	Settings::setXMax( Parser::number( m_xmax ) );
	Settings::setYMin( Parser::number( m_ymin ) );
	Settings::setYMax( Parser::number( m_ymax ) );

	Settings::setXRange(4); //custom x-range
	Settings::setYRange(4); //custom y-range

	setScaling();

	drawPlot(); //update all graphs
}


void View::coordToMinMax( const int koord, const QString &minStr, const QString &maxStr,
                          double &min, double &max )
{
	switch ( koord )
	{
	case 0:
		min = -8.0;
		max = 8.0;
		break;
	case 1:
		min = -5.0;
		max = 5.0;
		break;
	case 2:
		min = 0.0;
		max = 16.0;
		break;
	case 3:
		min = 0.0;
		max = 10.0;
		break;
	case 4:
		min = XParser::self()->eval( minStr );
		max = XParser::self()->eval( maxStr );
	}
}

void View::setScaling()
{
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };

	assert( (Settings::xScaling() >= 0) && (Settings::xScaling() < 9) );
	assert( (Settings::yScaling() >= 0) && (Settings::yScaling() < 9) );

	if( Settings::xScaling() == 8) //automatic x-scaling
    {
		tlgx = double(m_xmax-m_xmin)/16;
        tlgxstr = units[ Settings::xScaling() ];
    }
	else
	{
		tlgxstr = units[ Settings::xScaling() ];
		tlgx = XParser::self()->eval( tlgxstr );
	}

	if( Settings::yScaling() == 8)  //automatic y-scaling
    {
		tlgy = double(m_ymax-m_ymin)/16;
        tlgystr = units[ Settings::yScaling() ];
    }
	else
	{
		tlgystr = units[ Settings::yScaling() ];
		tlgy = XParser::self()->eval( tlgystr );
	}

	drskalxstr = units[ Settings::xPrinting() ];
	drskalx = XParser::self()->eval( drskalxstr );
	drskalystr = units[ Settings::yPrinting() ];
	drskaly = XParser::self()->eval( drskalystr );
}

void View::getSettings()
{
	coordToMinMax( Settings::xRange(), Settings::xMin(), Settings::xMax(), m_xmin, m_xmax );
	coordToMinMax( Settings::yRange(), Settings::yMin(), Settings::yMax(), m_ymin, m_ymax );
	setScaling();

	XParser::self()->setAngleMode( (Parser::AngleMode)Settings::anglemode() );

	m_backgroundColor = Settings::backgroundcolor();
	if ( !m_backgroundColor.isValid() )
		m_backgroundColor = Qt::white;

	invertColor(m_backgroundColor,m_invertedBackgroundColor);

// 	setBackgroundColor(m_backgroundColor);
	QPalette palette;
	palette.setColor( backgroundRole(), m_backgroundColor );
	setPalette(palette);
}

void View::init()
{
	QList<int> functionIDs = XParser::self()->m_ufkt.keys();
	foreach ( int id, functionIDs )
		XParser::self()->removeFunction( id );
	getSettings();
}


void View::stopDrawing()
{
	if (m_isDrawing)
		stop_calculating = true;
}

QPointF View::findMinMaxValue( const Plot & plot, ExtremaType type, double dmin, double dmax )
{
	Function * ufkt = plot.function();
	assert( ufkt->type() == Function::Cartesian );

	double x = 0;
	double y = 0;
	double result_x = 0;
	double result_y = 0;
	bool start = true;

	double dx;
	if ( plot.plotMode == Function::Integral )
	{
		if ( ufkt->integral_use_precision )
		{
			if ( ufkt->integral_use_precision )
				dx = ufkt->integral_precision*(dmax-dmin)/area.width();
			else
				dx = ufkt->integral_precision;
		}
		else
		{
			if ( ufkt->integral_use_precision )
				dx = (dmax-dmin)/area.width();
			else
				dx = 1.0;
		}
	}
	else
		dx = (dmax-dmin)/area.width();

	x=dmin;

	plot.updateFunctionParameter();

	while ( (x>=dmin && x<=dmax) )
	{
		y = value( plot, 0, x, false );

		if ( !isnan(x) && !isnan(y) )
		{
			kDebug() << "x " << x << endl;
			kDebug() << "y " << y << endl;
			if (x>=dmin && x<=dmax)
			{
				if ( start )
				{
					result_x = x;
					result_y = y;
					start=false;
				}
				else switch ( type )
				{
					case Minimum:
					{
						if ( y <= result_y )
						{
							result_x = x;
							result_y = y;
						}
						break;
					}

					case Maximum:
					{
						if ( y >= result_y )
						{
							result_x = x;
							result_y = y;
						}
						break;
					}
				}
			}
		}

		x += dx;
	}

	return QPointF( result_x, result_y );
}


void View::keyPressEvent( QKeyEvent * e )
{
	// if a zoom operation is in progress, assume that the key press is to cancel it
	if ( m_zoomMode != Normal )
	{
		m_zoomMode = Normal;
		update();
		updateCursor();
		return;
	}

	if (m_isDrawing)
	{
		stop_calculating=true;
		return;
	}

	if ( m_currentPlot.functionID() == -1 )
		return;

	QMouseEvent * event = 0;
	if (e->key() == Qt::Key_Left )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() - QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Right )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() + QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) //switch graph in trace mode
	{
		/// \todo reimplement moving between plots
#if 0
		QMap<int, Function*>::iterator it = XParser::self()->m_ufkt.find( m_currentPlot.functionID );
		int const ke=(*it)->parameters.count();
		if (ke>0)
		{
			m_currentFunctionParameter++;
			if (m_currentFunctionParameter >= ke)
				m_currentFunctionParameter=0;
		}
		if (m_currentFunctionParameter==0)
		{
			int const old_m_currentPlot.functionID=m_currentPlot.functionID;
			Function::PMode const old_m_currentPlot.plotMode = m_currentPlot.plotMode;
			bool start = true;
			bool found = false;
			while ( 1 )
			{
				if ( old_m_currentPlot.functionID==m_currentPlot.functionID && !start)
				{
					m_currentPlot.plotMode=old_m_currentPlot.plotMode;
					break;
				}
				kDebug() << "m_currentPlot.functionID: " << m_currentPlot.functionID << endl;
				switch ( (*it)->type() )
				{
					case Function::Parametric:
				case Function::Polar:
					break;
				default:
				{
					//going through the function, the first and the second derivative
					for ( m_currentPlot.plotMode = (Function::PMode)0; m_currentPlot.plotMode < 3; m_currentPlot.plotMode = (Function::PMode)(m_currentPlot.plotMode+1) )
// 					for (m_currentPlot.plotMode=0;m_currentPlot.plotMode<3;m_currentPlot.plotMode++)
					{
							if (start)
							{
								if ( m_currentPlot.plotMode==Function::Derivative2)
									m_currentPlot.plotMode=Function::Derivative0;
								else
									m_currentPlot.plotMode = (Function::PMode)(old_m_currentPlot.plotMode+1);
								start=false;
							}
						kDebug() << "   m_currentPlot.plotMode: " << (int)m_currentPlot.plotMode << endl;

						if ( (*it)->plotAppearance( m_currentPlot.plotMode ).visible )
							found = true;

							if (found)
								break;
						}
						break;
					}
				}
				if (found)
					break;

				if ( ++it == XParser::self()->m_ufkt.end())
					it = XParser::self()->m_ufkt.begin();
				m_currentPlot.functionID = (*it)->id;
			}
		}

		kDebug() << "************************" << endl;
		kDebug() << "m_currentPlot.functionID: " << (int)m_currentPlot.functionID << endl;
		kDebug() << "m_currentPlot.plotMode: " << (int)m_currentPlot.plotMode << endl;
		kDebug() << "m_currentFunctionParameter: " << m_currentFunctionParameter << endl;

		setStatusBar( (*it)->prettyName( m_currentPlot.plotMode ), 4 );

		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint(), Qt::LeftButton, Qt::LeftButton, 0 );
#endif
	}
	else if ( e->key() == Qt::Key_Space  )
	{
		event = new QMouseEvent( QEvent::MouseButtonPress, QCursor::pos(), Qt::RightButton, Qt::RightButton, 0 );
		mousePressEvent(event);
		delete event;
		return;
	}
	else
	{
		event = new QMouseEvent( QEvent::MouseButtonPress, m_crosshairPixelCoords.toPoint(), Qt::LeftButton, Qt::LeftButton, 0 );
		mousePressEvent(event);
		delete event;
		return;
	}
	mouseMoveEvent(event);
	delete event;
}


double View::areaUnderGraph( IntegralDrawSettings s )
{
	assert( s.dmin < s.dmax );

	Function * ufkt = s.plot.function();
	assert( ufkt );

	double dx;
	if ( (s.plot.plotMode == Function::Integral) && ufkt->integral_use_precision )
		dx = ufkt->integral_precision*(s.dmax-s.dmin)/area.width();
	else
		dx = (s.dmax-s.dmin)/area.width();

	// Make sure that we calculate the exact area (instead of missing out a
	// vertical slither at the end) by making sure dx tiles the x-range
	// a whole number of times
	int intervals = qRound( (s.dmax-s.dmin)/dx );
	dx = (s.dmax-s.dmin) / intervals;

	double calculated_area=0;
	double x = s.dmin;

	s.plot.updateFunctionParameter();

	for ( int i = 0; i <= intervals; ++i )
	{
		double y = value( s.plot, 0, x, false );

		// Trapezoid rule for integrals: only add on half for the first and last value
		if ( (i == 0) || (i == intervals) )
			calculated_area += 0.5*dx*y;
		else
			calculated_area += dx*y;

		x=x+dx;
	}

	m_integralDrawSettings = s;
	m_drawIntegral = true;
	drawPlot();
	m_drawIntegral = false;
	return calculated_area;
}

bool View::isCalculationStopped()
{
	if ( stop_calculating)
	{
		stop_calculating = false;
		return true;
	}
	else
		return false;
}

void View::updateSliders()
{
	if ( m_sliderWindow )
	{
		m_sliderWindow->hide();
		m_menuSliderAction->setChecked( false ); //uncheck the slider-item in the menu
	}

	// do we need to show any sliders?
// 	for(QVector<Function>::iterator it=XParser::self()->ufkt.begin(); it!=XParser::self()->ufkt.end(); ++it)
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->m_parameters.useSlider && !it->allPlotsAreHidden() )
		{
			if ( !m_sliderWindow )
			{
				m_sliderWindow = new KSliderWindow( this, m_ac );
				connect( m_sliderWindow, SIGNAL( valueChanged() ), this, SLOT( drawPlot() ) );
				connect( m_sliderWindow, SIGNAL( windowClosed() ), this, SLOT( sliderWindowClosed() ) );
			}
			m_sliderWindow->show();
			m_menuSliderAction->setChecked( false );  //set the slider-item in the menu
		}
	}
}

void View::mnuHide_clicked()
{
	if ( m_currentPlot.functionID() == -1 )
      return;

	Function * ufkt = m_currentPlot.function();
	ufkt->plotAppearance( m_currentPlot.plotMode ).visible = false;

	MainDlg::self()->functionEditor()->functionsChanged();
	drawPlot();
	m_modified = true;
	updateSliders();
	if ( m_currentPlot.functionID() == -1 )
		return;
	if ( ufkt->allPlotsAreHidden() )
	{
		m_currentPlot.setFunctionID( -1 );
		QMouseEvent *event = new QMouseEvent( QMouseEvent::KeyPress, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, 0 );
		mousePressEvent(event); //leave trace mode
		delete event;
		return;
	}
	else
	{
		QKeyEvent *event = new QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Up, 0 );
		keyPressEvent(event); //change selected graph
		delete event;
		return;
	}
}
void View::mnuRemove_clicked()
{
	if ( m_currentPlot.functionID() == -1 )
      return;

	Function * ufkt = m_currentPlot.function();
	Function::Type function_type = ufkt->type();
	if (!XParser::self()->removeFunction( ufkt ))
		return;

	if ( m_currentPlot.functionID() != -1 ) // if trace mode is enabled
	{
		m_currentPlot.setFunctionID( -1 );
		QMouseEvent *event = new QMouseEvent( QMouseEvent::KeyPress, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, 0 );
		mousePressEvent(event); //leave trace mode
		delete event;
	}

	drawPlot();
	if ( function_type == Function::Cartesian )
		updateSliders();
	m_modified = true;
}
void View::mnuEdit_clicked()
{
	MainDlg::self()->functionEditor()->setCurrentFunction( m_currentPlot.functionID() );
}


void View::mnuZoomIn_clicked()
{
	m_zoomMode = ZoomIn;
	updateCursor();
}


void View::mnuZoomOut_clicked()
{
	m_zoomMode = ZoomOut;
	updateCursor();
}


void View::mnuTrig_clicked()
{
	if ( Settings::anglemode() == 0 )
	{
		//radians
		animateZoom( QRectF( -(47.0/24.0)*M_PI, -4.0, (47.0/12.0)*M_PI, 8.0 ) );
	}
	else
	{
		//degrees
		animateZoom( QRectF( 352.5, -4.0, 705.0, 8.0 ) );
	}
}


void View::invertColor(QColor &org, QColor &inv)
{
	int r = org.red()-255;
	if ( r<0) r=r*-1;
	int g = org.green()-255;
	if ( g<0) g=g*-1;
	int b = org.blue()-255;
	if ( b<0) b=b*-1;

	inv.setRgb(r,g,b);
}


void View::updateCursor()
{
	Cursor newCursor = m_prevCursor;

	if ( m_isDrawing && (m_zoomMode != Translating) )
		newCursor = CursorWait;

	else switch (m_zoomMode)
	{
		case AnimatingZoom:
			newCursor = CursorArrow;
			break;

		case Normal:
			if ( shouldShowCrosshairs() )
			{
				// Don't show any cursor if we're tracing a function or the crosshairs should be shown
				newCursor = CursorBlank;
			}
			else
				newCursor = CursorArrow;
			break;

		case ZoomIn:
		case ZoomInDrawing:
			newCursor = CursorMagnify;
			break;

		case ZoomOut:
		case ZoomOutDrawing:
			newCursor = CursorLessen;
			break;

		case AboutToTranslate:
		case Translating:
			newCursor = CursorMove;
			break;
	}

	if ( newCursor == m_prevCursor )
		return;
	m_prevCursor = newCursor;

	switch ( newCursor )
	{
		case CursorWait:
			setCursor( Qt::WaitCursor );
			break;
		case CursorBlank:
			setCursor( Qt::BlankCursor );
			break;
		case CursorArrow:
			setCursor( Qt::ArrowCursor );
			break;
		case CursorCross:
			setCursor( Qt::CrossCursor );
			break;
		case CursorMagnify:
			setCursor( QCursor( SmallIcon( "magnify", 32), 10, 10 ) );
			break;
		case CursorLessen:
			setCursor( QCursor( SmallIcon( "lessen", 32), 10, 10 ) );
			break;
		case CursorMove:
			setCursor( Qt::SizeAllCursor );

	}
}


bool View::shouldShowCrosshairs() const
{
	switch ( m_zoomMode )
	{
		case Normal:
		case ZoomIn:
		case ZoomOut:
			break;

		case AnimatingZoom:
		case ZoomInDrawing:
		case ZoomOutDrawing:
		case AboutToTranslate:
		case Translating:
			return false;
	}

	Function * it = m_currentPlot.function();

	QPoint mousePos = mapFromGlobal( QCursor::pos() );

	return ( underMouse() && area.contains( mousePos ) && (!it || crosshairPositionValid( it )) );
}


bool View::event( QEvent * e )
{
	if ( e->type() == QEvent::WindowDeactivate && m_isDrawing)
	{
		stop_calculating = true;
		return true;
	}
	return QWidget::event(e); //send the information further
}


void View::setStatusBar(const QString &text, const int id)
{
	if ( m_readonly) //if KmPlot is shown as a KPart with e.g Konqueror, it is only possible to change the status bar in one way: to call setStatusBarText
	{
		switch (id)
		{
		case 1:
			m_statusbartext1 = text;
			break;
		case 2:
			m_statusbartext2 = text;
			break;
		case 3:
			m_statusbartext3 = text;
			break;
		case 4:
			m_statusbartext4 = text;
			break;
		default:
			return;
		}
		QString statusbartext = m_statusbartext1;
		if ( !m_statusbartext1.isEmpty() && !m_statusbartext2.isEmpty() )
			statusbartext.append("   |   ");
		statusbartext.append(m_statusbartext2);
		if ( !m_statusbartext2.isEmpty() && !m_statusbartext3.isEmpty() )
			statusbartext.append("   |   ");
		statusbartext.append(m_statusbartext3);
		if ( (!m_statusbartext2.isEmpty() || !m_statusbartext3.isEmpty() ) && !m_statusbartext4.isEmpty() )
			statusbartext.append("   |   ");
		statusbartext.append(m_statusbartext4);
		emit setStatusBarText(statusbartext);
	}
	else
	{
        QDBusReply<void> reply = QDBusInterfacePtr( QDBus::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.Kmplot" )->call( QDBusAbstractInterface::NoWaitForReply, "setStatusBarText", text, id );
	}
}


void View::slidersWindowClosed()
{
	m_menuSliderAction->setChecked(false);
}
//END class View



//BEGIN class IntegralDrawSettings
IntegralDrawSettings::IntegralDrawSettings()
{
	dmin = dmax = 0.0;
}
//END class IntegralDrawSettings

#include "View.moc"
