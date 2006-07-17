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
#include <QAbstractTextDocumentLayout>
#include <qbitmap.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <QApplication>
#include <QPainter>
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
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
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
#include "functiontools.h"
#include "settings.h"
#include "ksliderwindow.h"
#include "maindlg.h"
#include "parameteranimator.h"
#include "view.h"
#include "viewadaptor.h"
#include "xparser.h"


// other includes
#include <assert.h>
#include <cmath>


//BEGIN nan & inf
#ifdef __osf__
#include <nan.h>
#define isnan(x) IsNAN(x)
#define isinf(x) IsINF(X)
#endif

#ifdef USE_SOLARIS
#include <ieeefp.h>
int isinf(double x)
{
	return !finite(x) && x==x;
}
#endif
//END nan & inf



//BEGIN class View
View * View::m_self = 0;

View::View( bool readOnly, bool & modified, KMenu * functionPopup, QWidget* parent )
	: QWidget( parent, Qt::WStaticContents ),
	  buffer( width(), height() ),
	  m_popupMenu( functionPopup ),
	  m_modified( modified ),
	  m_readonly( readOnly )
{
	assert( !m_self ); // this class should only be constructed once
	m_self = this;
	
	m_haveRoot = false;
	m_xmin = m_xmax = m_ymin = m_ymax = 0.0;
	m_printHeaderTable = false;
	m_stopCalculating = false;
	m_isDrawing = false;
	m_popupMenuStatus = NoPopup;
	m_zoomMode = Normal;
	m_prevCursor = CursorArrow;
	
	m_textEdit = new QTextEdit;
	m_textEdit->setWordWrapMode( QTextOption::NoWrap );
	m_textEdit->setLineWrapMode( QTextEdit::NoWrap );
	m_textDocument = m_textEdit->document();

	m_mousePressTimer = new QTime();

    new ViewAdaptor(this);
    QDBus::sessionBus().registerObject("/view", this);

	XParser::self( & modified );
	init();
	getSettings();

	setMouseTracking( true );
	m_sliderWindow = 0;
	updateSliders();
	
	m_popupMenuTitle = m_popupMenu->addTitle( "" );
}


View::~View()
{
	m_textEdit->deleteLater();
	delete XParser::self();
	delete m_mousePressTimer;
}


void View::initDrawLabels()
{
	m_labelFont = QFont( Settings::labelFont(), Settings::labelFontSize() );
	
	for ( int i = 0; i < LabelGridSize; ++i )
		for ( int j = 0; j < LabelGridSize; ++j )
			m_usedDiagramArea[i][j] = false;
	
	// Add the axis
	double x = xToPixel( 0 );
	double y = yToPixel( 0 );
	
	double x0 = xToPixel( m_xmin );
	double x1 = xToPixel( m_xmax );
	double y0 = yToPixel( m_ymin );
	double y1 = yToPixel( m_ymax );
	
	// x-axis
	markDiagramAreaUsed( QRectF( x-20, y0, 40, y1-y0 ) );
	
	// y-axis
	markDiagramAreaUsed( QRectF( x0, y-20, x1-x0, 40 ) );
}


void View::draw( QPaintDevice * dev, PlotMedium medium )
{
	if ( m_isDrawing )
		return;
	m_isDrawing=true;
	
	getSettings();
	updateCursor();
	initDrawLabels();
	
	QPainter painter( dev );

	switch ( medium )
	{
		case SVG:
		case Screen:
		{
			m_clipRect = QRect( 0, 0, width(), height() );
			break;
		}

		case Printer:
		{
			// Pipesmoker: I hope I haven't damanged any printing stuff while rearranging the
			// diagram appearance! (I promise to fix it once KPrinter works again if so :P).
			// I intend to have the user specify the height and width of the output image,
			// instead of specifying the distance between tic marks - hopefully this is
			// more intuitive.
			// - David
			
			KPrinter * printer = static_cast<KPrinter*>(dev);
			
			/// \todo get width, height in cm
			double width_cm = 12;
			double height_cm = 12;
			
			double pixelsPerCm = (254.0/100.0) * printer->resolution();
			
			m_clipRect = QRect( 0, 0, int(width_cm * pixelsPerCm), int(height_cm * pixelsPerCm) );
			
			m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1";
			drawHeaderTable( &painter );
			if ( printer->option( "app-kmplot-printbackground" ) == "-1" )
				painter.fillRect( m_clipRect,  m_backgroundColor); //draw a colored background
			break;
		}
		
		case Pixmap:
		{
			QPixmap * pic = static_cast<QPixmap*>(dev);
			m_clipRect = pic->rect();
			pic->fill(m_backgroundColor);
			break;
		}
	}
	
	//BEGIN setup drawing values
	m_realToPixel.reset();
	m_realToPixel.scale( m_clipRect.width()/(m_xmax-m_xmin), m_clipRect.height()/(m_ymin-m_ymax) );
	m_realToPixel.translate( -m_xmin, -m_ymax );
	
	m_pixelToReal = m_realToPixel.inverted();
	
	ticStartX = ceil(m_xmin/ticSepX.value())*ticSepX.value();
	ticStartY = ceil(m_ymin/ticSepY.value())*ticSepY.value();
	//END setup drawing values

	
	//BEGIN draw diagram background stuff
	painter.setRenderHint( QPainter::Antialiasing, true );
	
	drawGrid( &painter );
	if ( Settings::showAxes() )
		drawAxes( &painter );
	if( Settings::showLabel() )
		drawLabels( &painter );
	//END draw diagram background stuff
	
	
	//BEGIN draw the functions
	m_stopCalculating = false;

	// Antialiasing slows down rendering a lot, so turn it off if we are
	// sliding the view about
	painter.setRenderHint( QPainter::Antialiasing, m_zoomMode != Translating );
	
	painter.setClipping( true );
	painter.setClipRect( m_clipRect );
	double numPlots = XParser::self()->m_ufkt.size();
	double at = -1;
	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		at += 1;
		
		if ( m_stopCalculating )
			break;
		
// 		QDBusInterface( QDBus::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.KmPlot" ).call( QDBus::NoBlock, "setDrawProgress", at/numPlots );

		if ( function->type() == Function::Implicit )
			drawImplicit( function, & painter );
		else
			drawFunction( function, & painter );
	}
// 	QDBusInterface( QDBus::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.KmPlot" ).call( QDBus::NoBlock, "setDrawProgress", 1.0 );
	
	drawFunctionInfo( & painter );
	painter.setClipping( false );
	
	m_isDrawing=false;
	//END draw the functions

	updateCursor();
}


//BEGIN coordinate mapping functions
QPointF View::toPixel( const QPointF & real, ClipBehaviour clipBehaviour )
{
	xclipflg = false;
	yclipflg = false;
	
	QPointF pixel = m_realToPixel.map( real );
	double x = pixel.x();
	double y = pixel.y();
	
	if ( clipBehaviour == ClipAll )
	{
		if ( isnan(x) || x<0 )
		{
			xclipflg = true;
			x = 0;
		}
		else if ( x > m_clipRect.right() )
		{
			xclipflg = true;
			x = m_clipRect.right();
		}
	}
	else
	{
		if ( isnan(x) )
		{
			xclipflg = true;
			x = 0;
		}
		else if ( isinf(x) == -1 )
		{
			x = 0;
		}
		else if ( isinf(x) == 1 )
		{
			x = m_clipRect.right();
		}	
	}
	
	if ( clipBehaviour == ClipAll )
	{
		if ( isnan(y) || y<0 )
		{
			yclipflg = true;
			y = 0;
		}
		else if ( y > m_clipRect.bottom() )
		{
			yclipflg = true;
			y = m_clipRect.bottom();
		}
	}
	else
	{
		if ( isnan(y) )
		{
			yclipflg = true;
			y = 0;
		}
		else if ( isinf(y) == -1 )
		{
			y = 0;
		}
		else if ( isinf(y) == 1 )
		{
			y = m_clipRect.bottom();
		}	
	}
	
	return QPointF( x, y );
}

double View::xToPixel( double x, ClipBehaviour clipBehaviour )
{
	return toPixel( QPointF( x, 0 ), clipBehaviour ).x();
}

double View::yToPixel( double y, ClipBehaviour clipBehaviour )
{
	return toPixel( QPointF( 0, y ), clipBehaviour ).y();
}


QPointF View::toReal( const QPointF & pixel )
{
	return m_pixelToReal.map( pixel );
}

double View::xToReal( double x )
{
	return toReal( QPointF( x, 0 ) ).x();
}

double View::yToReal( double y )
{
	return toReal( QPointF( 0, y ) ).y();
}
//END coordinate mapping functions


void View::drawAxes( QPainter* painter )
{
	double axesLineWidth = mmToPenWidth( Settings::axesLineWidth(), painter );
	double ticWidth = mmToPenWidth( Settings::ticWidth(), painter );
	double ticLength = mmToPenWidth( Settings::ticLength(), painter );
	QColor axesColor = Settings::axesColor();
	
	painter->save();
		
	int const dx=7;
	int const dy=4;
	
	painter->setPen( QPen( axesColor, axesLineWidth ) );
	painter->setBrush( axesColor );
	
	//BEGIN draw x axis
	double a = m_clipRect.right()-ticLength;
	double b = yToPixel(0.);
	
	double b_max = m_clipRect.bottom() - ticLength;
	if ( b < ticLength )
		b = ticLength;
	else if ( b > b_max )
		b = b_max;
	
	// horizontal line
	painter->Lineh(ticLength, b, a);
	
	// arrow head
	if ( Settings::showArrows())
	{
		QPolygonF p(3);
		p[0] = QPointF( a, b );
		p[1] = QPointF( a-dx, b+dy );
		p[2] = QPointF( a-dx, b-dy );
		painter->drawPolygon( p );
	}
	//END draw x axis

	
	//BEGIN draw y axis
	a = xToPixel(0.);
	b = ticLength;
	
	double a_max = m_clipRect.right() - ticLength;
	if ( a < ticLength )
		a = ticLength;
	else if ( a > a_max )
		a = a_max;
	
	// vertical line
	painter->Linev(a, m_clipRect.bottom()-ticLength, b);
	
	// arrow head
	if ( Settings::showArrows() )
	{
		QPolygonF p(3);
		p[0] = QPointF( a, b );
		p[1] = QPointF( a-dy, b+dx );
		p[2] = QPointF( a+dy, b+dx );
		painter->drawPolygon( p );
	}
	//END draw y axis
	
	painter->restore();
	
	painter->setPen( QPen( axesColor, ticWidth ) );
	
	double da = yToPixel(0)-ticLength;
	double db = yToPixel(0)+ticLength;
	double d = ticStartX;
	if (da<0)
	{
		a = 0;
		b = 2*ticLength;
	}
	else if(db>(double)m_clipRect.bottom())
	{
		b = m_clipRect.bottom();
		a = m_clipRect.bottom()-2*ticLength;
	}
	else
	{
		a = da;
		b = db;
	}

	while(d<m_xmax-ticSepX.value()/2.)
	{
		double d_pixel = xToPixel(d);
		if ( d_pixel > ticLength )
			painter->Linev(xToPixel(d), a, b);
		d+=ticSepX.value();
	}

	da = xToPixel(0)-ticLength;
	db = xToPixel(0)+ticLength;
	d = ticStartY;
	if (da<0)
	{
		a = 0;
		b = 2*ticLength;
	}
	else if(db>(double)m_clipRect.right())
	{
		b = m_clipRect.right();
		a = m_clipRect.right()-2*ticLength;
	}
	else
	{
		a = da;
		b = db;
	}

	while(d<m_ymax-ticSepY.value()/2.)
	{
		double d_pixel = yToPixel(d);
		if ( d_pixel < m_clipRect.bottom()-ticLength )
			painter->Lineh(a, d_pixel, b);
		d+=ticSepY.value();
	}
}


void View::drawGrid( QPainter* painter )
{
	QColor gridColor = Settings::gridColor();
	
	double gridLineWidth = mmToPenWidth( Settings::gridLineWidth(), painter );
	QPen pen( gridColor, gridLineWidth );

	painter->setPen(pen);
	
	enum GridStyle
	{
		GridNone,
		GridLines,
		GridCrosses,
		GridPolar
	};
	GridStyle gridMode = (GridStyle)Settings::gridStyle();
	
	switch ( gridMode )
	{
		case GridNone:
			break;
			
		case GridLines:
		{
			for ( double d = ticStartX; d <= m_xmax; d += ticSepX.value() )
				painter->Linev(xToPixel(d), m_clipRect.bottom(), 0);
			
			for ( double d = ticStartY; d <= m_ymax; d += ticSepY.value() )
				painter->Lineh(0, yToPixel(d), m_clipRect.right());
			
			break;
		}
		
		case GridCrosses:
		{
			int const dx = 5;
			int const dy = 5;

			for( double x = ticStartX; x<m_xmax; x+=ticSepX.value() )
			{
				double a = xToPixel(x);
				for( double y=ticStartY; y<m_ymax; y+=ticSepY.value())
				{
					double b = yToPixel(y);
					painter->Lineh(a-dx, b, a+dx);
					painter->Linev(a, b-dy, b+dy);
				}
			}
			
			break;
		}
		
		case GridPolar:
		{
			// Note: 1.42 \approx sqrt(2)
			
			painter->setClipRect( m_clipRect );
			
			double xMax = qMax( qAbs(m_xmin), qAbs(m_xmax) ) * 1.42;
			double yMax = qMax( qAbs(m_ymin), qAbs(m_ymax) ) * 1.42;
			
			// The furthest pixel away from the origin
			double pixelMax = qMax( xMax*m_realToPixel.m11(), yMax*m_realToPixel.m22() );
			
			double x = ticSepX.value();
			double y = ticSepY.value();
			
			while ( x <= xMax || y <= yMax )
			{
				QRectF r;
				r.setTopLeft(		toPixel( QPointF( -x, y ), ClipInfinite ) ); 
				r.setBottomRight(	toPixel( QPointF( x, -y ), ClipInfinite ) );
				
				painter->drawEllipse( r );
				
				x += ticSepX.value();
				y += ticSepY.value();
			}
			
			for ( double theta = 0; theta < 2.0*M_PI; theta += M_PI/12.0 )
			{
				QPointF start = toPixel( QPointF( 0, 0 ), ClipInfinite );
				QPointF end = start + QPointF( pixelMax * cos(theta), pixelMax * sin(theta) );
				
				painter->drawLine( start, end );
			}
			
			painter->setClipping( false );
			break;
		}
	}
}


void View::drawLabels(QPainter* pDC)
{
	QColor axesColor = Settings::axesColor();
	int const dx=10;
	int const dy=15;
	QFont const font=QFont( Settings::axesFont(), Settings::axesFontSize() );
	pDC->setFont(font);
	m_textDocument->setDefaultFont( font );
	
	double const x=xToPixel(0.);
	double const y=yToPixel(0.);
	double d;
	int n;

	char draw_next=0;
	QFontMetrics const test(font);
	int swidth=0;

	for(d=ticStartX, n=(int)ceil(m_xmin/ticSepX.value()); d<m_xmax; d+=ticSepX.value(), ++n)
	{
		if(n==0 || fabs(d-m_xmax)<=1.5*ticSepX.value())
			continue;
		
		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ticSepX.value()-M_PI/frac[i])> 1e-3 )
				continue;
			
			s = (n<0) ? '-' : '+';
			
			found = true;
			if(n==-1 || n==1)
				s += QChar(960) + QString("/%1").arg(frac[i]);
			else if(n%frac[i] == 0)
			{
				if ( n == -frac[i] || n == frac[i])
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/frac[i]);
					s+=QChar(960);
				}
			}
			
			// Don't allow just plus or minus
			if ( s == "+" || s == "-" )
				s = QString();
			
			break;
		}
		
		if ( !found && (n%5==0 || n==1 || n==-1 || draw_next))
		{
			s = posToString( n*ticSepX.value(), (m_xmax-m_xmin)/4, View::ScientificFormat, axesColor );
		}
		
		if ( !s.isEmpty() )
		{
			swidth = test.width(s);
			if (  xToPixel(d)-x<swidth && xToPixel(d)-x>-swidth && draw_next==0)
			{
				draw_next=1;
				continue;
			}
			if (draw_next>0)
			{
				if (draw_next==1)
				{
					draw_next++;
					continue;
				}
				else
					draw_next=0;
			}
			
			if ( xclipflg )
				continue;
			
			m_textDocument->setHtml( s );
			QRectF br = m_textDocument->documentLayout()->frameBoundingRect( m_textDocument->rootFrame() );
			
			double x_pos = xToPixel(d)-(br.width()/2);
			double y_pos;
			if ( m_ymin < 0 )
			{
				y_pos = y+dy-(br.height()/2);
				
				double over = m_clipRect.bottom() - (y_pos + br.height());
				if ( over < 0 )
					y_pos += over;
			}
			else
				y_pos = y-dy-(br.height()/2);
				
			QPointF drawPoint( x_pos, y_pos );
			
			pDC->translate( drawPoint );
			m_textDocument->documentLayout()->draw( pDC, QAbstractTextDocumentLayout::PaintContext() ); 
			pDC->translate( -drawPoint );
		}
	}

	QRectF drawRect;
	
	if ( m_ymax<0 && m_xmax<0 )
		drawRect = QRectF( xToPixel(m_xmax)-(3*dx), y+dy, 0, 0 );
	else
		drawRect = QRectF( xToPixel(m_xmax)-dx, y+dy, 0, 0 );
	pDC->drawText( drawRect, Qt::AlignCenter|Qt::TextDontClip, "x" );

	for(d=ticStartY, n=(int)ceil(m_ymin/ticSepY.value()); d<m_ymax; d+=ticSepY.value(), ++n)
	{
		if(n==0 || fabs(d-m_ymax)<=1.5*ticSepY.value())
			continue;

		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ticSepY.value()-M_PI/frac[i])> 1e-3 )
				continue;
			
			s = (n<0) ? '-' : '+';
			
			found = true;
			if(n==-1 || n==1)
				s += QChar(960) + QString("/%1").arg(frac[i]);
			else if(n%frac[i] == 0)
			{
				if ( n == -frac[i] || n == frac[i])
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/frac[i]);
					s+=QChar(960);
				}
			}
			
			// Don't allow just plus or minus
			if ( s == "+" || s == "-" )
				s = QString();
			
			break;
		}
		if( !found && (n%5==0 || n==1 || n==-1))
		{
			s = posToString( n*ticSepY.value(), (m_ymax-m_ymin)/4, View::ScientificFormat, axesColor );
		}
		
		if ( !s.isEmpty() )
		{
			m_textDocument->setHtml( s );
			
			QRectF br = m_textDocument->documentLayout()->frameBoundingRect( m_textDocument->rootFrame() );
			
			QPointF drawPoint( 0, yToPixel(d)-(br.height()/2) );
			
			if (m_xmin>=0)
			{
				drawPoint.setX( x+dx );
			}
			else
			{
				drawPoint.setX( x-dx-br.width() );
				
				if ( drawPoint.x() < 0 )
				{
					// Don't draw off the left edge of the screen
					drawPoint.setX( 0 );
				}
			}
			
			if ( yclipflg )
				continue;
			
			pDC->translate( drawPoint );
			m_textDocument->documentLayout()->draw( pDC, QAbstractTextDocumentLayout::PaintContext() );
			pDC->translate( -drawPoint );
		}
	}

	
	if(m_ymax<0 && m_xmax<0)
		drawRect = QRectF( x-dx, yToPixel(m_ymax)+(2*dy), 0, 0 );
	else if (m_xmin>0)
		drawRect = QRectF( x-(2*dx), yToPixel(m_ymax)+dy, 0, 0 );
	else
		drawRect = QRectF( x-dx, yToPixel(m_ymax)+dy, 0, 0 );
	pDC->drawText( drawRect, Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, "y" );
}


double View::h( const Plot & plot ) const
{
	if ( (plot.plotMode == Function::Integral) || (plot.function()->type() == Function::Differential) )
		return plot.function()->eq[0]->differentialStates.step().value();
	else
		return qMin( (m_xmax-m_xmin)/m_clipRect.width(), (m_ymax-m_ymin)/m_clipRect.height() );
}


double View::value( const Plot & plot, int eq, double x, bool updateFunction )
{
	Function * function = plot.function();
	assert( function );
	
	if ( updateFunction )
		plot.updateFunction();
	
	Equation * equation = function->eq[eq];
	
	double dx = h( plot );
	
	DifferentialState * state = plot.state();
	
	return XParser::self()->derivative( plot.derivativeNumber(), equation, state, x, dx );
}


QPointF View::realValue( const Plot & plot, double x, bool updateFunction )
{
	Function * function = plot.function();
	assert( function );

	switch ( function->type() )
	{
		case Function::Differential:
		case Function::Cartesian:
		{
			double y = value( plot, 0, x, updateFunction );
			return QPointF( x, y );
		}

		case Function::Polar:
		{
			double y = value( plot, 0, x, updateFunction );
			return QPointF( y * lcos(x), y * lsin(x) );
		}

		case Function::Parametric:
		{
			double X = value( plot, 0, x, updateFunction );
			double Y = value( plot, 1, x, updateFunction );
			return QPointF( X, Y );
		}
		
		case Function::Implicit:
		{
			// Can only calculate the value when either x or y is fixed.
			assert( function->m_implicitMode != Function::UnfixedXY );
			
			double val = value( plot, 0, x, updateFunction );
			
			if ( function->m_implicitMode == Function::FixedX )
				return QPointF( function->x, val );
			else
				return QPointF( val, function->y );
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
			case Function::Parametric:
			case Function::Polar:
				min = 0.0;
				break;
				
			case Function::Implicit:
				kWarning() << "Probably don't want to do this!\n";
				// no break
				
			case Function::Differential:
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
			case Function::Parametric:
			case Function::Polar:
				max = 2.0*M_PI;
				break;
				
			case Function::Implicit:
				kWarning() << "Probably don't want to do this!\n";
				// no break
				
			case Function::Differential:
			case Function::Cartesian:
				max = m_xmax;
				break;
		}
	}

	if ( (function->type() == Function::Cartesian) && (max > m_xmax) )
		max = m_xmax;

	return max;
}

// #define DEBUG_IMPLICIT

#ifdef DEBUG_IMPLICIT
// Used in profiling root finding
int root_find_iterations;
int root_find_requests;
#endif


/**
 * For comparing points where two points close together are considered equal.
 */
class FuzzyPoint
{
	public:
		FuzzyPoint( const QPointF & point )
		{
			x = point.x();
			y = point.y();
		}
		
		
		FuzzyPoint( double x, double y )
		{
			FuzzyPoint::x = x;
			FuzzyPoint::y = y;
		}
		
		
		bool operator < ( const FuzzyPoint & other ) const
		{
			double du = qAbs(other.x - x) / dx;
			double dv = qAbs(other.y - y) / dy;
			
			bool x_eq = (du < 1); // Whether the x coordinates are considered equal
			bool y_eq = (dv < 1); // Whether the y coordinates are considered equal
			
			if ( x_eq && y_eq )
			{
				// Points are close together.
				return false;
			}
			
			bool x_lt = !x_eq && (x < other.x);
			bool y_lt = !y_eq && (y < other.y);
			
			return ( x_lt || (x_eq && y_lt) );
		}
		
		double x;
		double y;
		
		static double dx;
		static double dy;
};
typedef QMap< FuzzyPoint, QPointF > FuzzyPointMap;

double FuzzyPoint::dx = 0;
double FuzzyPoint::dy = 0;


double SegmentMin = 0.1;
double SegmentMax = 6.0;


// The viewable area is divided up into square*squares squares, and the curve
// is traced around in each square.
// NOTE: it is generally a good idea to make this number prime
int squares = 19;

void View::drawImplicit( Function * function, QPainter * painter )
{
	assert( function->type() == Function::Implicit );
	
#ifdef DEBUG_IMPLICIT
	QTime t;
	t.start();
	
	painter->setPen( Qt::black );
	
	for ( double i = 0; i <= squares; ++i )
	{
		double x = m_xmin + i * (m_xmax-m_xmin)/squares;
		double y = m_ymin + i * (m_ymax-m_ymin)/squares;
		
		painter->drawLine( toPixel( QPointF( m_xmin, y ), ClipInfinite ), toPixel( QPointF( m_xmax, y ), ClipInfinite ) );
		painter->drawLine( toPixel( QPointF( x, m_ymin ), ClipInfinite ), toPixel( QPointF( x, m_ymax ), ClipInfinite ) );
	}
	
	root_find_iterations = 0;
	root_find_requests = 0;
#endif
	
	// Need another function for investigating singular points
	Plot circular;
	QString fname( "f(x)=0" );
	XParser::self()->fixFunctionName( fname, Equation::Cartesian, -1 );
	circular.setFunctionID( XParser::self()->Parser::addFunction( fname, 0, Function::Cartesian ) );
	assert( circular.function() );
	
	const QList< Plot > plots = function->plots();
	foreach ( Plot plot, plots )
	{
		bool setAliased = false;
		if ( plot.parameter.type() == Parameter::Animated )
		{
			// Don't use antialiasing, so that rendering is speeded up
			if ( painter->renderHints() & QPainter::Antialiasing )
			{
				setAliased = true;
				painter->setRenderHint( QPainter::Antialiasing, false );
			}
		}
		
		painter->setPen( penForPlot( plot, painter ) );
		
		QList<QPointF> singular;
		
		for ( int i = 0; i <= squares; ++i )
		{
			double y = m_ymin + i*(m_ymax-m_ymin)/double(squares);
			
			function->y = y;
			function->m_implicitMode = Function::FixedY;
			QList<double> roots = findRoots( plot, m_xmin, m_xmax, RoughRoot );
			
			foreach ( double x, roots )
			{
#ifdef DEBUG_IMPLICIT
				painter->setPen( QPen( Qt::red, painter->pen().width() ) );
#endif
				drawImplicitInSquare( plot, painter, x, y, Qt::Horizontal, & singular );
			}
			
			
			
			double x = m_xmin + i*(m_xmax-m_xmin)/double(squares);
			
			function->x = x;
			function->m_implicitMode = Function::FixedX;
			roots = findRoots( plot, m_ymin, m_ymax, RoughRoot );
			
			foreach ( double y, roots )
			{	
#ifdef DEBUG_IMPLICIT
				painter->setPen( QPen( Qt::blue, painter->pen().width() ) );
#endif
				drawImplicitInSquare( plot, painter, x, y, Qt::Vertical, & singular );
			}
		}
		
		// Sort out the imlpicit points
		FuzzyPointMap singularSorted;
		FuzzyPoint::dx = (m_xmax-m_xmin) * SegmentMin * 0.1 / m_clipRect.width();
		FuzzyPoint::dy = (m_ymax-m_ymin) * SegmentMin * 0.1 / m_clipRect.height();
		foreach ( QPointF point, singular )
			singularSorted.insert( point, point );
		singular = singularSorted.values();
		
		foreach ( QPointF point, singular )
		{
			// radius of circle around singular point
			double epsilon = qMin( FuzzyPoint::dx, FuzzyPoint::dy );
			
			QString fstr;
			fstr = QString("%1(x)=%2(%3+%6*cos(x),%4+%6*sin(x)%5)")
					.arg( circular.function()->eq[0]->name() )
					.arg( function->eq[0]->name() )
					.arg( XParser::self()->number( point.x() ) )
					.arg( XParser::self()->number( point.y() ) )
					.arg( function->eq[0]->usesParameter() ? "," + XParser::self()->number( function->k ) : QString() )
					.arg( XParser::self()->number( epsilon ) );
			
			bool setFstrOk = circular.function()->eq[0]->setFstr( fstr );
			assert( setFstrOk );
			
			QList<double> roots = findRoots( circular, 0, 2*M_PI / XParser::self()->radiansPerAngleUnit(), PreciseRoot );
			
#ifdef DEBUG_IMPLICIT
			kDebug() << "Singular point at (x,y)=("<<point.x()<<","<<point.y()<<")\n";
			kDebug() << "fstr is    " << fstr << endl;
			kDebug() << "Found " << roots.size() << " roots.\n";
#endif
			
			foreach ( double t, roots )
			{	
#ifdef DEBUG_IMPLICIT
				painter->setPen( QPen( Qt::green, painter->pen().width() ) );
#endif
				double x = point.x() + epsilon * lcos(t);
				double y = point.y() + epsilon * lsin(t);
				drawImplicitInSquare( plot, painter, x, y, 0, & singular );
			}
		}
		
		
		if ( setAliased )
			painter->setRenderHint( QPainter::Antialiasing, true );
	}
	
#ifdef DEBUG_IMPLICIT
	if ( root_find_requests != 0 )
		kDebug() << "Average iterations in root finding was " << root_find_iterations/root_find_requests << endl;
	kDebug() << "Time taken was " << t.elapsed() << endl;
#endif
	
	XParser::self()->removeFunction( circular.functionID() );
}


// static
double View::maxSegmentLength( double curvature )
{
	// Use a circle angle of 4 degrees to determine the maximum segment length
	// Also, limit the length to be between 0.1 and 6 pixels.
	
	double arc = 4 * (M_PI / 180);
	
	if ( curvature < 0 )
		curvature = -curvature;
	
	if ( curvature < 1e-20 )
		return SegmentMax; // very large circle
	
	double radius = 1.0/curvature;
	
	double segment = arc * radius;
	if ( segment < SegmentMin )
		segment = SegmentMin;
	else if ( segment > SegmentMax )
		segment = SegmentMax;
	 
	return segment;
}


// does for real numbers what "%" does for integers
double realModulo( double x, double mod )
{
	x = qAbs(x);
	mod = qAbs(mod);
	return x - floor(x/mod)*mod;
}


void View::drawImplicitInSquare( const Plot & plot, QPainter * painter, double x, double y, Qt::Orientations orientation, QList<QPointF> * singular )
{
	plot.updateFunction();
	Plot diff1 = plot;
	diff1.differentiate();
	Plot diff2 = diff1;
	diff2.differentiate();
	
#ifdef DEBUG_IMPLICIT
	painter->save();
	painter->setPen( QPen( Qt::black, painter->pen().width() ) );
	QPointF tl = toPixel( QPointF( x, y ), ClipInfinite ) - QPoint( 2, 2 );
	painter->drawRect( QRectF( tl, QSizeF( 4, 4 ) ) );
	painter->restore();
#endif
	
	double x_side = (m_xmax-m_xmin)/squares;
	double y_side = (m_ymax-m_ymin)/squares;
	
	// Use a square around the root to bound the tracing
	// To start with, assume that tracing will go up,right. But this
	// might not be so, so the upper/lower boundaries may be adjusted depending
	// on where the tracing ends up
	double x_lower, x_upper, y_lower, y_upper;
	if ( orientation & Qt::Vertical )
	{
		x_lower = x;
		x_upper = x + x_side;
	}
	else
	{
		double x_prop = (x-m_xmin)/(m_xmax-m_xmin);
		x_lower = std::floor( x_prop * squares ) * x_side + m_xmin;
		x_upper = x_lower + x_side;
	}
	if ( orientation & Qt::Horizontal )
	{
		y_lower = y;
		y_upper = y + y_side;
	}
	else
	{
		double y_prop = (y-m_ymin)/(m_ymax-m_ymin);
		y_lower = std::floor( y_prop * squares ) * y_side + m_ymin;
		y_upper = y_lower + y_side;
	}
	
	// If during tracing, the root could not be found, then this will be set to true,
	// the route will be retraced using a smaller step size and it will attempt to find
	// a root again. If it fails for a second time, then tracing is finished.
	bool foundRootPreviously = true;
	
	// Used for focal points.
	double prevAngle = 0;
	int switchCount = 0;
	
	// This is so that the algorithm can "look ahead" to see what is coming up,
	// before drawing or committing itself to anything potentially bad
	QPointF prev2 = toPixel( QPointF( x, y ), ClipInfinite );
	QPointF prev1 = prev2;
	
	// Allow us to doubly retrace
	double prev_diff_x = 0;
	double prev_diff_y = 0;
	
	for ( int i = 0; i < 500; ++i ) // allow a maximum of 500 traces (to prevent possibly infinite loop)
	{
		if ( i == 500 - 1 )
		{
			kDebug() << "Implicit: got to last iteration!\n";
		}
		
		// (dx, dy) is perpendicular to curve
		
		plot.function()->x = x;
		plot.function()->y = y;
		
		plot.function()->m_implicitMode = Function::FixedY;
		double dx = value( diff1, 0, x, false );
		
		plot.function()->m_implicitMode = Function::FixedX;
		double dy = value( diff1, 0, y, false );
		
		double k = pixelCurvature( plot, x, y );
		double segment_step = maxSegmentLength( k ) * pow( 0.5, switchCount );
		
		// If we couldn't find a root in the previous iteration, it was possibly
		// because we were using too large a step size. So reduce the step size
		// and try again.
		if ( !foundRootPreviously )
			segment_step = qMin( segment_step/4, SegmentMin );
		
// 		kDebug() << "k="<<k<<" segment_step="<<segment_step<<endl;
		
		QPointF p1 = toPixel( QPointF( x, y ),			ClipInfinite ) * painter->matrix();
		QPointF p2 = toPixel( QPointF( x+dx, y+dy ),	ClipInfinite ) * painter->matrix();
		double l = QLineF( p1, p2 ).length() / segment_step;
		
		if ( l == 0 )
		{
			kDebug() << "length is zero!\n";
			break;
		}
		
		// (tx, ty) is tangent to the curve in the direction that we are tracing
		double tx = -dy/l;
		double ty = dx/l;
		
		double angle = atan(ty/tx) + ((tx<0) ? M_PI : 0);
		double diff = realModulo( angle-prevAngle, 2*M_PI );
		
		bool switchedDirection = (i > 0) && (diff > (3./4.)*M_PI) && (diff < (5./4.)*M_PI);
		if ( switchedDirection )
		{
			// Why do I care about suddenly changing the direction?
			// Because the chances are, a attracting or repelling point has been reached.
			// Even if not, it suggests that a smaller step size is needed. If we have
			// switched direction and are already at the smallest step size, then note
			// the dodgy point for further investigation and give up for now
			
// 			kDebug() << "Switched direction: x="<<x<<" switchCount="<<switchCount<<" segment_step="<<segment_step<<" i="<<i<<endl;
			
			// Use a step size much smaller than segment min to obtain good accuracy,
			// needed for investigating the point further
			if ( segment_step <= SegmentMin * 0.01 )
			{
				// Give up. Tell our parent function to investigate the point further
				*singular << QPointF( x, y );
				break;
			}
			
			// Rewind the last tangent addition as well
			x -= prev_diff_x;
			y -= prev_diff_y;
			
			prev_diff_x = 0;
			prev_diff_y = 0;
			
			switchCount += 2;
			continue;
		}
		else
		{
			// Reset the stepping adjustment
			switchCount = qMax( 0, switchCount-1 );
			prevAngle = angle;
// 			kDebug() << "Didn't switch - x="<<x<<" segment_step="<<segment_step<<endl;
		}
		
		if ( i == 0 )
		{
			// First trace; does the bounding square need adjusting?
			
			if ( (tx < 0) && (orientation & Qt::Vertical) )
			{
				x_lower -= x_side;
				x_upper -= x_side;
			}
			
			if ( (ty < 0) && (orientation & Qt::Horizontal) )
			{
				y_lower -= y_side;
				y_upper -= y_side;
			}
		}
		
		
		// The maximum tangent length before we end up outside our bounding square
		double max_tx, max_ty;
		if ( tx > 0 )
			max_tx = x_upper - x;
		else
			max_tx = x - x_lower;
		if ( ty > 0 )
			max_ty = y_upper - y;
		else
			max_ty = y - y_lower;
		
		
		// Does (tx,ty) need to be scaled to make sure the tangent stays inside the square?
		double scale = qMax( (tx==0) ? 0 : qAbs(tx)/max_tx, (ty==0) ? 0 : qAbs(ty)/max_ty );
		bool outOfBounds = scale > 1;
		if ( outOfBounds )
		{
			tx /= scale;
			ty /= scale;
		}
		
		double x0 = x;
		double y0 = y;
		
		x += tx;
		y += ty;
		
		plot.function()->x = x;
		plot.function()->y = y;
		
		double * coord = 0;
		if ( qAbs(tx) > qAbs(ty) )
		{
			plot.function()->m_implicitMode = Function::FixedX;
			coord = & y;
		}
		else
		{
			plot.function()->m_implicitMode = Function::FixedY;
			coord = & x;
		}
					
		bool found = findRoot( coord, plot, RoughRoot );
		if ( !found )
		{
			if ( foundRootPreviously )
			{
#ifdef DEBUG_IMPLICIT
				kDebug() << "Could not find root!\n";
#endif
				
				// Retrace our steps
				x = x0;
				y = y0;
				prev_diff_x = 0;
				prev_diff_y = 0;
				continue;
				foundRootPreviously = false;
			}
			else
			{
				kDebug() << "Couldn't find root - giving up.\n";
				break;
			}
		}
		else
			foundRootPreviously = true;
		
		prev_diff_x = x - x0;
		prev_diff_y = y - y0;
		
		painter->drawLine( prev2, prev1 );
		prev2 = prev1;
		prev1 = toPixel( QPointF( x, y ), ClipInfinite );
		markDiagramPointUsed( prev1 );
		
		if ( outOfBounds )
			break;
	}
	
	// and the final line
	painter->drawLine( prev2, prev1 );
}


void View::drawFunction( Function * function, QPainter * painter )
{
	if ( (function->type() == Function::Differential) &&
			 (function->eq[0]->order() == 1) &&
			function->plotAppearance( Function::Derivative0 ).showTangentField )
	{
		QList<Plot> plots = function->plots( Function::PlotCombinations(Function::AllCombinations) & ~Function::DifferentInitialStates );
		foreach ( Plot plot, plots )
			drawTangentField( plot, painter );
	}
	
	QList<Plot> plots = function->plots();
	foreach ( Plot plot, plots )
		drawPlot( plot, painter );
	
}


void View::drawTangentField( const Plot & plot, QPainter * painter )
{
	plot.updateFunction();
	Function * function = plot.function();
	
	assert( function->type() == Function::Differential );
	// Can only draw tangent fields for first order differential equations
	assert( function->eq[0]->order() == 1 );
	
	painter->setPen( penForPlot( plot, painter ) );
	
	bool useParameter = function->eq[0]->usesParameter();
	Vector v( useParameter ? 3 : 2 );
	
	if ( useParameter )
		v[1] = function->k;
	
	// For converting from real to pixels
	double sx = m_clipRect.width() / (m_xmax - m_xmin);
	double sy = m_clipRect.height() / (m_ymax - m_ymin);
	
	for ( double x = ticStartX; x <= m_xmax; x += ticSepX.value() )
	{
		v[0] = x;
		for ( double y = ticStartY; y <= m_ymax; y += ticSepY.value() )
		{
			v[ useParameter ? 2 : 1 ] = y;
			
			double df = XParser::self()->fkt( function->eq[0], v ) * (sy / sx);
			double theta = std::atan( df );
			double dx = std::cos( theta ) * (ticSepX.value() / 8.0);
			double dy = std::sin( theta ) * (ticSepY.value() / 8.0);
			
			QPointF mid( x, y );
			QPointF diff( dx, dy );
			
			painter->drawLine( toPixel( mid-diff ), toPixel( mid+diff ) );
		}
	}
}


void View::drawPlot( const Plot & plot, QPainter *painter )
{
	plot.updateFunction();
	Function * function = plot.function();
	
	// should use drawImplicit for implicit functions
	assert( function->type() != Function::Implicit );
	
	bool setAliased = false;
	if ( (plot.parameter.type() == Parameter::Animated) && (painter->renderHints() & QPainter::Antialiasing) )
	{
		// Don't use antialiasing, so that rendering is speeded up
		setAliased = true;
		painter->setRenderHint( QPainter::Antialiasing, false );
	}
	
	painter->setPen( penForPlot( plot, painter ) );
	
	double dmin = getXmin( function );
	double dmax = getXmax( function );
	
	if ( dmin >= dmax )
		return;
	
	// the 'middle' dx, which may be increased or decreased
	double base_dx = (dmax-dmin)/m_clipRect.width();
	if ( (function->type() == Function::Parametric) || (function->type() == Function::Polar) )
		base_dx *= 0.01;

	// Increase speed while translating the view
	bool quickDraw = ( m_zoomMode == Translating );
	if ( quickDraw )
		base_dx *= 4.0;
	
	double dx = base_dx;
	
	bool drawIntegral = m_integralDrawSettings.draw && (m_integralDrawSettings.plot == plot);
	double totalLength = 0.0; // total pixel length; used for drawing dotted lines
	
	bool p1Set = false;
	QPointF p1, p2;
	
	double x = dmin;
	double prevX = x; // the value of x before last adding dx to it
	do
	{	
		QPointF rv = realValue( plot, x, false );
			
		// If we are currently plotting a differential equation, and it became infinite,
		// then skip x forward to a point where it is finite
		if ( function->type() == Function::Differential && !XParser::self()->differentialFinite )
		{
			double new_x = XParser::self()->differentialDiverge;
			if ( new_x > x )
			{
				x = new_x;
				prevX = x;
				
				continue;
			}
		}
			
		p2 = toPixel( rv, ClipInfinite );

		if ( xclipflg || yclipflg )
		{
			prevX = x;
			x += dx;
			
			p1Set = false; // p1 wouldn't be finite (if we had set it)
			continue;
		}
		
		if ( !p1Set )
		{
			prevX = x;
			x += dx;
			
			p1 = p2;
			p1Set = true;
			continue;
		}
		
		
		//BEGIN adjust dx
		QRectF bound = QRectF( p1, QSizeF( (p2-p1).x(), (p2-p1).y() ) ).normalized();
		double length = QLineF( p1, p2 ).length();
		totalLength += length;

		double min_mod = (function->type() == Function::Cartesian || function->type() == Function::Differential) ? 0.1 : 5e-5;
		double max_mod = (function->type() == Function::Cartesian || function->type() == Function::Differential) ? 1e+1 : 5e+1;
		bool dxAtMinimum = (dx <= base_dx*min_mod);
		bool dxAtMaximum = (dx >= base_dx*max_mod);
		bool dxTooBig = false;
		bool dxTooSmall = false;
		
		if ( QRectF( m_clipRect ).intersects( bound ) )
		{
			dxTooBig = !dxAtMinimum && (length > (quickDraw ? max_mod : 4.0));
			dxTooSmall = !dxAtMaximum && (length < (quickDraw ? 10.0 : 1.0));
		}
		else
			dxTooSmall = !dxAtMaximum;
			
		if ( dxTooBig )
		{
			dx *= 0.5;
			x = prevX + dx;
			continue;
		}
		
		if ( dxTooSmall )
			dx *= 2.0;
		//END adjust dx
		
		
		if ( drawIntegral && (x >= m_integralDrawSettings.dmin) && (x <= m_integralDrawSettings.dmax) )
		{
			double y0 = yToPixel( 0 );

			QPointF points[4];
			points[0] = QPointF( p1.x(), y0 );
			points[1] = QPointF( p2.x(), y0 );
			points[2] = QPointF( p2.x(), p2.y() );
			points[3] = QPointF( p1.x(), p1.y() );

			painter->drawPolygon( points, 4 );
		}
		else if ( penShouldDraw( totalLength, plot ) )
			painter->drawLine( p1, p2 );
		
		markDiagramPointUsed( p2 );
		
		p1 = p2;
		Q_ASSERT( dx > 0 );
		
		prevX = x;
		x += dx;
	}
	while ( x <= dmax );
		
	if ( setAliased )
		painter->setRenderHint( QPainter::Antialiasing, true );
}


void View::drawFunctionInfo( QPainter * painter )
{
	// Don't draw info if translating the view
	if ( m_zoomMode == Translating )
		return;
	
	// The names of the plots are drawn around the edge of the view, in a clockwise
	// direction, starting from the top-right. Picture the positions like this:
	// 
	//   7  8  9  0
	//   6        1
	//   5  4  3  2
	
	// Used for determining where to draw the next label indicating the plot name
	int plotNameAt = 0;
	
	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		if ( m_stopCalculating )
			break;
		
		foreach ( Plot plot, function->plots() )
		{
			plot.updateFunction();
			
			// Draw extrema points?
			if ( (function->type() == Function::Cartesian) && function->plotAppearance( plot.plotMode ).showExtrema )
			{
				QList<QPointF> stationaryPoints = findStationaryPoints( plot );
				foreach ( QPointF realValue, stationaryPoints )
				{
					painter->setPen( QPen( Qt::black, mmToPenWidth( 1.5, painter ) ) );
					painter->drawPoint( toPixel( realValue ) );
					
					QString x = posToString( realValue.x(), (m_xmax-m_xmin)/m_clipRect.width(), View::DecimalFormat );
					QString y = posToString( realValue.y(), (m_ymax-m_ymin)/m_clipRect.width(), View::DecimalFormat );
					
					drawLabel( painter, plot.color(), realValue, QString( "x = %1   y = %2" ).arg(x).arg(y) );
				}
			}
			
			// Show the name of the plot?
			if ( function->plotAppearance( plot.plotMode ).showPlotName )
			{
				double x, y;
				
				double xmin = m_xmin + 0.1 * (m_xmax-m_xmin);
				double xmax = m_xmax - 0.1 * (m_xmax-m_xmin);
				double ymin = m_ymin + 0.1 * (m_ymax-m_ymin);
				double ymax = m_ymax - 0.1 * (m_ymax-m_ymin);
				
				// Find out where on the outer edge of the view to draw it
				if ( 0 <= plotNameAt && plotNameAt <= 2 )
				{
					x = xmax;
					y = ymax - (ymax-ymin)*plotNameAt/2;
				}
				else if ( 3 <= plotNameAt && plotNameAt <= 5 )
				{
					x = xmax - (xmax-xmin)*(plotNameAt-2)/3;
					y = ymin;
				}
				else if ( 6 <= plotNameAt && plotNameAt <= 7 )
				{
					x = xmin;
					y = ymin + (ymax-ymin)*(plotNameAt-5)/2;
				}
				else
				{
					x = xmin + (xmax-xmin)*(plotNameAt-7)/3;
					y = ymax;
				}
				
				plotNameAt = (plotNameAt+1) % 10;
				
				QPointF realPos;
				
				if ( function->type() == Function::Implicit )
				{
					findRoot( & x, & y, plot, RoughRoot );
					realPos = QPointF( x, y );
				}
				else
				{
					double t = getClosestPoint( QPointF( x, y ), plot );
					realPos = realValue( plot, t, false );
				}
				
				drawLabel( painter, plot.color(), realPos, plot.name() );
			}
		}
	}
}


void View::drawLabel( QPainter * painter, const QColor & color, const QPointF & realPos, const QString & text )
{
	QPalette palette;
	QColor outline = color;
	QColor background = outline.light( 500 );
	background.setAlpha( 127 );
	
	
	QPointF pixelCenter = toPixel( realPos );
	QRectF rect( pixelCenter, QSizeF( 1, 1 ) );
	
	int flags = Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop;
	rect = painter->boundingRect( rect, flags, text ).adjusted( -8, -4, 4, 2 );
	
	// Try and find a nice place for inserting the rectangle
	int bestCost = int(1e7);
	QPointF bestCenter = realPos;
	for ( double x = pixelCenter.x() - 300; x <= pixelCenter.x() + 300; x += 20 )
	{
		for ( double y = pixelCenter.y() - 300; y <= pixelCenter.y() + 300; y += 20 )
		{
			QPointF center( x, y ) ;
			rect.moveCenter( center );
			double length = (x-pixelCenter.x())*(x-pixelCenter.x()) + (y-pixelCenter.y())*(y-pixelCenter.y());
			int cost = rectCost( rect ) + int(length)/100;
			
			if ( cost < bestCost )
			{
				bestCenter = center;
				bestCost = cost;
			}
		}
	}
	
	rect.moveCenter( bestCenter );
	
	markDiagramAreaUsed( rect );
	
	painter->setBrush( background );
	painter->setPen( outline );
	painter->drawRect( rect );
	
	
	// If the rectangle does not lie over realPos, then draw a line to realPos from the rectangle
	if ( ! rect.contains( pixelCenter ) )
	{
		QPointF lineStart = bestCenter;
		QLineF line( pixelCenter, bestCenter );
		
		QPointF intersect = bestCenter;
		
		// Where does line intersect the rectangle?
		if ( QLineF( rect.topLeft(), rect.topRight() ).intersect( line, & intersect ) == QLineF::BoundedIntersection )
			lineStart = intersect;
		else if ( QLineF( rect.topRight(), rect.bottomRight() ).intersect( line, & intersect ) == QLineF::BoundedIntersection )
			lineStart = intersect;
		else if ( QLineF( rect.bottomRight(), rect.bottomLeft() ).intersect( line, & intersect ) == QLineF::BoundedIntersection )
			lineStart = intersect;
		else if ( QLineF( rect.bottomLeft(), rect.topLeft() ).intersect( line, & intersect ) == QLineF::BoundedIntersection )
			lineStart = intersect;
		
		painter->drawLine( lineStart, pixelCenter );
	}
	
	
	painter->setFont( m_labelFont );
	painter->setPen( Qt::black );
	painter->drawText( rect.adjusted( 8, 4, -4, -2 ), flags, text );
}


QRect View::usedDiagramRect( const QRectF & rect ) const
{
	double x0 = rect.left() / m_clipRect.width();
	double x1 = rect.right() / m_clipRect.width();
	
	double y0 = rect.top() / m_clipRect.height();
	double y1 = rect.bottom() / m_clipRect.height();
	
	int i0 = qMax( int( x0 * LabelGridSize ), 0 );
	int i1 = qMin( int( x1 * LabelGridSize ), LabelGridSize-1 );
	int j0 = qMax( int( y0 * LabelGridSize ), 0 );
	int j1 = qMin( int( y1 * LabelGridSize ), LabelGridSize-1 );
	
	return QRect( i0, j0, i1-i0+1, j1-j0+1 ) & QRect( 0, 0, LabelGridSize, LabelGridSize );
}


void View::markDiagramAreaUsed( const QRectF & rect )
{
	if ( m_zoomMode == Translating )
		return;
	
	QRect r = usedDiagramRect( rect );
	
	for ( int i = r.left(); i <= r.right(); ++i )
		for ( int j = r.top(); j <= r.bottom(); ++j )
			m_usedDiagramArea[i][j] = true;
}


void View::markDiagramPointUsed( const QPointF & point )
{
	if ( m_zoomMode == Translating )
		return;
	
	double x = point.x() / m_clipRect.width();
	double y = point.y() / m_clipRect.height();
	
	int i = int( x * LabelGridSize );
	int j = int( y * LabelGridSize );
	
	if ( i<0 || i>=LabelGridSize || j<0 || j>=LabelGridSize )
		return;
	
	m_usedDiagramArea[i][j] = true;
}


int View::rectCost( QRectF rect ) const
{
	rect = rect.normalized();
	
	int cost = 0;
	
	// If the rectangle goes off the edge, mark it as very high cost)
	if ( rect.intersects( m_clipRect ) )
	{
		QRectF intersect = (rect & m_clipRect);
		cost += int(rect.width() * rect.height() - intersect.width() * intersect.height());
	}
	else
	{
		// The rectangle is completely outside!
		cost += int(rect.width() * rect.height());
	}
	
	
	QRect r = usedDiagramRect( rect );
	
	for ( int i = r.left(); i <= r.right(); ++i )
		for ( int j = r.top(); j <= r.bottom(); ++j )
			if ( m_usedDiagramArea[i][j] )
				cost += 200;
	
	return cost;
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


QPen View::penForPlot( const Plot & plot, QPainter * painter ) const
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
	
	pen.setColor( plot.color() );

	Function * ufkt = plot.function();
	PlotAppearance appearance = ufkt->plotAppearance( plot.plotMode );
	
	double lineWidth_mm = appearance.lineWidth;
	
	if ( appearance.style == Qt::SolidLine )
		pen.setCapStyle( Qt::FlatCap );

	double width = mmToPenWidth( lineWidth_mm, painter );
	pen.setWidthF( width );
	
	return pen;
}


double View::mmToPenWidth( double width_mm, QPainter * painter ) const
{
	QPaintDevice * dev = painter->device();
	assert( dev->physicalDpiX() == dev->physicalDpiY() );
	double dpi = dev->physicalDpiX();
	
	return dpi * (width_mm/25.4);
}


void View::drawHeaderTable(QPainter *pDC)
{
	QString alx, aly, atx, aty, dfx, dfy;

	if( m_printHeaderTable )
	{
		pDC->translate(250., 150.);
		/// \todo fix this
// 		pDC->setPen(QPen(Qt::black, (int)(5.*m_scaler)));
		pDC->setPen(QPen(Qt::black, 0));
		pDC->setFont(QFont( Settings::headerTableFont(), 30) );
		puts( Settings::headerTableFont().toLatin1().data() );
		alx="[ "+Settings::xMin()+" | "+Settings::xMax()+" ]";
		aly="[ "+Settings::yMin()+" | "+Settings::yMax()+" ]";
		alx.replace( "pi", QChar(960) );
		aly.replace( "pi", QChar(960) );
		atx="1E  =  "+ticSepX.expression();
		aty="1E  =  "+ticSepY.expression();
		atx.replace( "pi", QChar(960) );
		aty.replace( "pi", QChar(960) );
		
		/// \todo do we want to do something with ticPrint[X/Y] ?
// 		dfx="1E  =  "+ticPrintX.expression()+" cm";
// 		dfx->replace( "pi", QChar(960) );
// 		dfy="1E  =  "+ticPrintY.expression()+" cm";
// 		dfy->replace( "pi", QChar(960) );

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
			foreach ( Equation * eq, it->eq )
			{
				if ( m_stopCalculating )
					break;

				QString fstr = eq->fstr();
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


QList< QPointF > View::findStationaryPoints( const Plot & plot )
{
	Plot plot2 = plot;
	plot2.differentiate();

	QList< double > roots = findRoots( plot2, getXmin( plot.function() ), getXmax( plot.function() ), RoughRoot );

	plot.updateFunction();
	QList< QPointF > stationaryPoints;
	foreach ( double x, roots )
	{
		QPointF real = realValue( plot, x, false );
		if ( real.y() >= m_ymin && real.y() <= m_ymax )
			stationaryPoints << real;
	}

	return stationaryPoints;
}


QList< double > View::findRoots( const Plot & plot, double min, double max, RootAccuracy accuracy )
{
	typedef QMap< double, double > DoubleMap;
	DoubleMap roots;
	
	int count = 10; // number of points to (initially) check for roots
	
	int prevNumRoots = 0;
	while ( count < 1000 )
	{
		// Use this to detect finding the same root.
		double prevX = 0.0;
		
		double dx = (max-min) / double(count);
		for ( int i = 0; i <= count; ++i )
		{
			double x = min + dx*i;
		
			bool found = findRoot( & x, plot, accuracy );
			if ( !found || x < min || x > max )
				continue;
		
			if ( !roots.isEmpty() )
			{
				// Check if already have a close root
				if ( qAbs(x-prevX) <= (dx/4) )
					continue;
			
				DoubleMap::iterator nextIt = roots.lowerBound(x);
				if ( nextIt == roots.end() )
					--nextIt;
				
				double lower, upper;
				lower = upper = *nextIt;
				if ( nextIt != roots.begin() )
					lower = *(--nextIt);
			
				if ( (qAbs(x-lower) <= (dx/4)) || (qAbs(x-upper) <= (dx/4)) )
					continue;
			}
		
			roots.insert( x, x );
			prevX = x;
		}
		
		int newNumRoots = roots.size();
		if ( newNumRoots == prevNumRoots )
			break;
		
		prevNumRoots = newNumRoots;
		count *= 4;
	}
	
	return roots.keys();
}


void View::setupFindRoot( const Plot & plot, RootAccuracy accuracy, double * max_k, double * max_f, int * n )
{
	plot.updateFunction();
	
	if ( accuracy == PreciseRoot )
	{
		*max_k = 200;
		*max_f = 1e-14;
	}
	else
	{
		// Rough root
		*max_k = 10;
		*max_f = 1e-10;
	}
	
	*n = 1 + plot.derivativeNumber();
}


bool View::findRoot( double * x, const Plot & plot, RootAccuracy accuracy )
{
#ifdef DEBUG_IMPLICIT
	root_find_requests++;
#endif
	
	double max_k, max_f;
	int n;
	setupFindRoot( plot, accuracy, & max_k, & max_f, & n );
	
	Equation * eq = plot.function()->eq[0];
	DifferentialState * state = plot.state();
	
	double h = qMin( m_xmax-m_xmin, m_ymax-m_ymin ) * 1e-5;

	double f = value( plot, 0, *x, false );
	int k;
	for ( k=0; k < max_k; ++k )
	{
		double df = XParser::self()->derivative( n, eq, state, *x, h );
		if ( qAbs(df) < 1e-20 )
			df = 1e-20 * ((df < 0) ? -1 : 1);

		double dx = f / df;
		*x -= dx;
		f = value( plot, 0, *x, false );
		
		if ( (qAbs(f) <= max_f) && (qAbs(dx) <= (h*1e-5)) )
			break;
	}
	
#ifdef DEBUG_IMPLICIT
	root_find_iterations += k;
#endif

	// We continue calculating until |f| < max_f; this may result in k reaching
	// max_k. However, if |f| is reasonably small (even if reaching max_k),
	// we consider it a root.
	return ( qAbs(f) < 1e-6 );
}


bool View::findRoot( double * x, double * y, const Plot & plot, RootAccuracy accuracy )
{
	double max_k, max_f;
	int n;
	setupFindRoot( plot, accuracy, & max_k, & max_f, & n );
	
	Function * function = plot.function();
	Equation * eq = function->eq[0];
	DifferentialState * state = plot.state();
	
	double hx = (m_xmax-m_xmin) * 1e-5;
	double hy = (m_ymax-m_ymin) * 1e-5;

	function->y = *y;
	function->m_implicitMode = Function::FixedY;
	double f = value( plot, 0, *x, false );
	
	for ( int k=0; k < max_k; ++k )
	{
		function->x = *x;
		function->y = *y;
		
		function->m_implicitMode = Function::FixedY;
		double dfx = XParser::self()->derivative( n, eq, state, *x, hx );
		
		function->m_implicitMode = Function::FixedX;
		double dfy = XParser::self()->derivative( n, eq, state, *y, hy );
		
		double dff = dfx*dfx + dfy*dfy;
		if ( dff < 1e-20 )
			dff = 1e-20;
		
		double dx = f * dfx / dff;
		*x -= dx;
		double dy = f * dfy / dff;
		*y -= dy;
		
		function->y = *y;
		function->m_implicitMode = Function::FixedY;
		f = value( plot, 0, *x, false );
		
		if ( (qAbs(f) <= max_f) && (qAbs(dx) <= (hx*1e-5)) && (qAbs(dy) <= (hy*1e-5)) )
			break;
	}

	// We continue calculating until |f| < max_f; this may result in k reaching
	// max_k. However, if |f| is reasonably small (even if reaching max_k),
	// we consider it a root.
	return ( qAbs(f) < 1e-6 );
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
		QPointF tl( toPixel( m_animateZoomRect.topLeft() ) );
		QPointF br( toPixel( m_animateZoomRect.bottomRight() ) );
		p.drawRect( QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) ) );
	}
	else if ( shouldShowCrosshairs() )
	{
		Function * function = m_currentPlot.function();
		
		QPen pen;
		
		if ( function )
		{
			QColor functionColor = m_currentPlot.color();
			pen.setColor( functionColor );
			p.setPen( pen );
			p.setRenderHint( QPainter::Antialiasing, true );
			
			double x = m_crosshairPosition.x();
			double y = m_crosshairPosition.y();
			
			p.setClipRect( m_clipRect );
			
			//BEGIN calculate curvature, normal
			double k = 0;
			double normalAngle = 0;
			
			switch ( function->type() )
			{
				case Function::Parametric:
				case Function::Polar:
					normalAngle = pixelNormal( m_currentPlot, m_trace_x );
					k = pixelCurvature( m_currentPlot, m_trace_x );
					break;
					
				case Function::Differential:
				case Function::Cartesian:
				case Function::Implicit:
					normalAngle = pixelNormal( m_currentPlot, x, y );
					k = pixelCurvature( m_currentPlot, x, y );
					break;
			}
			
			if ( k < 0 )
			{
				k = -k;
				normalAngle += M_PI;
			}
			//END calculate curvature, normal
			
			if ( k > 1e-5 && Settings::detailedTracing() )
			{
				p.save();
				
				// Transform the painter so that the center of the osculating circle is the origin,
				// with the normal line coming in frmo the left.
				QPointF center = m_crosshairPixelCoords + (1/k) * QPointF( cos( normalAngle ), sin( normalAngle ) );
				p.translate( center );
				p.rotate( normalAngle * 180 / M_PI );
				
				// draw osculating circle
				pen.setColor( functionColor );
				p.setPen( pen );
				p.drawEllipse( QRectF( -QPointF( 1/k, 1/k ), QSizeF( 2/k, 2/k ) ) );
				
				// draw normal
				pen.setColor( functionColor );
				p.setPen( pen );
				p.setBrush( pen.color() );
				p.drawLine( QLineF( -1/k, 0, 0, 0 ) );
				
				// draw normal arrow
				QPolygonF arrowHead(3);
				arrowHead[0] = QPointF( 0, 0 );
				arrowHead[1] = QPointF( -3, -2 );
				arrowHead[2] = QPointF( -3, +2 );
				p.drawPolygon( arrowHead );
				
				// draw tangent
				double tangent_scale = 1.2; // make the tangent look better
				p.drawLine( QLineF( -1/k, -qMax( 1/k, 15. ) * tangent_scale, -1/k, qMax( 1/k, 15. ) * tangent_scale ) );
				
				// draw perpendicular symbol
				QPolygonF perp(3);
				perp[0] = QPointF( -1/k, 10 );
				perp[1] = QPointF( -1/k + 10, 10 );
				perp[2] = QPointF( -1/k + 10, 0 );
				p.drawPolyline( perp );
				
				// draw intersection blob
				p.drawRect( QRectF( -1/k-1, -1, 2, 2 ) );
				
				p.restore();
				
				// Already show osculating circle, etc, so don't draw crosshairs quite so prominently
				functionColor.setAlpha( 63 );
				pen.setColor( functionColor );
			}
		}
		else
		{
			// Use an inverted background color for contrast
			QColor inverted = QColor( 255-m_backgroundColor.red(), 255-m_backgroundColor.green(), 255-m_backgroundColor.blue() );
			pen.setColor( inverted );
		}
		
		p.setPen( pen );
		p.Lineh( 0, m_crosshairPixelCoords.y(), m_clipRect.right() );
		p.Linev( m_crosshairPixelCoords.x(), m_clipRect.bottom(), 0 );
	}

	p.end();
}


double View::pixelNormal( const Plot & plot, double x, double y )
{
	Function * f = plot.function();
	assert( f );
	
	plot.updateFunction();
	
	// For converting from real to pixels
	double sx = m_clipRect.width() / (m_xmax - m_xmin);
	double sy = m_clipRect.height() / (m_ymax - m_ymin);
	
	double dx = 0;
	double dy = 0;
	
	double h = this->h( plot );
	
	int d0 = plot.derivativeNumber();
	int d1 = d0+1;
	
	switch ( f->type() )
	{
		case Function::Differential:
		case Function::Cartesian:
		{
			double df = XParser::self()->derivative( d1, f->eq[0], plot.state(), x, h );
			return -atan( df * (sy/sx) ) - (M_PI/2);
		}
		
		case Function::Implicit:
		{
			dx = XParser::self()->partialDerivative( d1, d0, f->eq[0], 0, x, y, h, h ) / sx;
			dy = XParser::self()->partialDerivative( d0, d1, f->eq[0], 0, x, y, h, h ) / sy;
			
			double theta = -atan( dy / dx );
			
			if ( dx < 0 )
				theta += M_PI;
			
			theta += M_PI;
			
			return theta;
		}
		
		case Function::Polar:
		{
			double r =  XParser::self()->derivative( d0, f->eq[0], 0, x, h );
			double dr = XParser::self()->derivative( d1, f->eq[0], 0, x, h );
			
			dx = (dr * lcos(x) - r * lsin(x) * XParser::self()->radiansPerAngleUnit()) * sx;
			dy = (dr * lsin(x) + r * lcos(x) * XParser::self()->radiansPerAngleUnit()) * sy;
			break;
		}
		
		case Function::Parametric:
		{
			dx = XParser::self()->derivative( d1, f->eq[0], 0, x, h ) * sx;
			dy = XParser::self()->derivative( d1, f->eq[1], 0, x, h ) * sy;
			break;
		}
	}
			
	double theta = - atan( dy / dx ) - (M_PI/2);
			
	if ( dx < 0 )
		theta += M_PI;
			
	return theta;
}


double View::pixelCurvature( const Plot & plot, double x, double y )
{
	Function * f = plot.function();
	
	// For converting from real to pixels
	double sx = m_clipRect.width() / (m_xmax - m_xmin);
	double sy = m_clipRect.height() / (m_ymax - m_ymin);
	
	double fdx = 0;
	double fdy = 0;
	double fddx = 0;
	double fddy = 0;
	double fdxy = 0;
	
	double h = this->h( plot );
	
	int d0 = plot.derivativeNumber();
	int d1 = d0+1;
	int d2 = d0+2;
	
	switch ( f->type() )
	{
		case Function::Differential:
		case Function::Cartesian:
		{
			DifferentialState * state = plot.state();
			
			fdx = sx;
			fddx = 0;
			
			fdy = XParser::self()->derivative( d1, f->eq[0], state, x, h ) * sy;
			fddy = XParser::self()->derivative( d2, f->eq[0], state, x, h) * sy;
			
// 			kDebug() << k_funcinfo << "fdy="<<fdy<<" fddy="<<fddy<<endl;
			
			break;
		}
		
		case Function::Polar:
		{
			double r = XParser::self()->derivative( d0, f->eq[0], 0, x, h );
			double dr = XParser::self()->derivative( d1, f->eq[0], 0, x, h );
			double ddr = XParser::self()->derivative( d2, f->eq[0], 0, x, h );
			
			fdx = (dr * lcos(x) - r * lsin(x) * XParser::self()->radiansPerAngleUnit()) * sx;
			fdy = (dr * lsin(x) + r * lcos(x) * XParser::self()->radiansPerAngleUnit()) * sy;
			
			double rpau = XParser::self()->radiansPerAngleUnit();
			
			fddx = (ddr * lcos(x) - 2 * dr * lsin(x) * rpau - r * lcos(x) * rpau*rpau) * sx;
			fddy = (ddr * lsin(x) + 2 * dr * lcos(x) * rpau - r * lsin(x) * rpau*rpau) * sy;
			
			break;
		}
		
		case Function::Parametric:
		{
			fdx = XParser::self()->derivative( d1, f->eq[0], 0, x, h ) * sx;
			fdy = XParser::self()->derivative( d1, f->eq[1], 0, x, h ) * sy;
			
			fddx = XParser::self()->derivative( d2, f->eq[0], 0, x, h ) * sx;
			fddy = XParser::self()->derivative( d2, f->eq[1], 0, x, h ) * sy;
			
			break;
		}
		
		case Function::Implicit:
		{
			fdx =  XParser::self()->partialDerivative( d1, d0, f->eq[0], 0, x, y, h, h ) / sx;
			fdy =  XParser::self()->partialDerivative( d0, d1, f->eq[0], 0, x, y, h, h ) / sy;
			
			fddx = XParser::self()->partialDerivative( d2, d0, f->eq[0], 0, x, y, h, h ) / (sx*sx);
			fddy = XParser::self()->partialDerivative( d0, d2, f->eq[0], 0, x, y, h, h ) / (sy*sy);
			
			fdxy = XParser::self()->partialDerivative( d1, d1, f->eq[0], 0, x, y, h, h ) / (sx*sy);
			
			
			break;
		}
	}
	
	double mod = pow( fdx*fdx + fdy*fdy, 1.5 );
	
	switch ( f->type() )
	{
		case Function::Differential:
		case Function::Cartesian:
		case Function::Parametric:
		case Function::Polar:
			return (fdx * fddy - fdy * fddx) / mod;
			
		case Function::Implicit:
			return ( fdx*fdx*fddy + fdy*fdy*fddx - 2*fdx*fdy*fdxy ) / mod;
	}
	
	kError() << "Unknown function type!\n";
	return 0;
}


void View::resizeEvent(QResizeEvent *)
{
	if (m_isDrawing) //stop drawing integrals
	{
		m_stopCalculating = true; //stop drawing
		return;
	}
	buffer = QPixmap( size() );
	drawPlot();
}


void View::drawPlot()
{
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

	if ( m_popupMenuStatus != NoPopup )
		return;

	if (m_isDrawing)
	{
		m_stopCalculating = true; //stop drawing
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
			QString popupTitle( m_currentPlot.name() );

			if ( hadFunction )
				m_popupMenuStatus = PopupDuringTrace;
			else
				m_popupMenuStatus = Popup;
			
			m_popupMenu->removeAction( m_popupMenuTitle );
			m_popupMenuTitle->deleteLater();
			m_popupMenuTitle = m_popupMenu->addTitle( popupTitle, MainDlg::self()->m_firstFunctionAction );
			m_popupMenu->exec( QCursor::pos() );
		}
		return;
	}

	if(e->button()!=Qt::LeftButton)
		return;

	if ( m_currentPlot.functionID() >= 0 ) //disable trace mode if trace mode is enable
	{
		m_currentPlot.setFunctionID( -1 );
		setStatusBar( QString(), RootSection );
		setStatusBar( QString(), FunctionSection );
		mouseMoveEvent(e);
		return;
	}

	QPointF closestPoint = getPlotUnderMouse();
	Function * function = m_currentPlot.function();
	if ( function )
	{
		QPointF ptd( toPixel( closestPoint ) );
		QPoint globalPos = mapToGlobal( ptd.toPoint() );
		QCursor::setPos( globalPos );
		setStatusBar( m_currentPlot.name().replace( '\n', " ; " ), FunctionSection );
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

	double best_distance = 1e30; // a nice large number
	QPointF best_cspos;

	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		const QList< Plot > plots = function->plots();
		foreach ( Plot plot, plots )
		{
			plot.updateFunction();
			
			double best_x = 0.0, distance;
			QPointF cspos;
			
			if ( function->type() == Function::Implicit )
			{
				double x = m_crosshairPosition.x();
				double y = m_crosshairPosition.y();
				findRoot( & x, & y, plot, PreciseRoot );
				
				QPointF d = toPixel( QPointF( x, y ), ClipInfinite ) - toPixel( QPointF( m_crosshairPosition.x(), m_crosshairPosition.y() ), ClipInfinite );
				
				distance = std::sqrt( d.x()*d.x() + d.y()*d.y() );
				cspos = QPointF( x, y );
			}
			else
			{
				best_x = getClosestPoint( m_crosshairPosition, plot );
				distance = pixelDistance( m_crosshairPosition, plot, best_x, false );
				cspos = realValue( plot, best_x, false );;
			}

			if ( distance < best_distance )
			{
				best_distance = distance;
				bestPlot = plot;
				m_trace_x = best_x;
				best_cspos = cspos;
			}
		}
	}
	
	if ( best_distance < 10.0 )
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
	plot.updateFunction();

	double best_x = 0.0;

	Function * function = plot.function();
	assert( function->type() != Function::Implicit ); // should use findRoot (3D version) for this

	switch ( function->type() )
	{
		case Function::Implicit:
			break;
		
		case Function::Differential:
		case Function::Cartesian:
		{
			double best_pixel_x = 0.0;
	
			QPointF pixelPos = toPixel( pos, ClipInfinite );
	
			double dmin = getXmin( function );
			double dmax = getXmax( function );
	
			double stepSize = (m_xmax-m_xmin)/m_clipRect.width();
	
			// Algorithm in use here: Work out the shortest distance between the
			// line joining (x0,y0) to (x1,y1) and the given point (real_x,real_y)
	
			double x = dmin;
			double y0 = value( plot, 0, x, false );
	
			double best_distance = 1e20; // a large distance
			
			while ( x <= dmax && (xToPixel(x) < best_pixel_x+best_distance) )
			{
				x += stepSize;
	
				double y1 = value( plot, 0, x, false );
	
				double _x0 = xToPixel( x-stepSize, ClipInfinite );
				double _x1 = xToPixel( x, ClipInfinite );
	
				double _y0 = yToPixel( y0, ClipInfinite );
				double _y1 = yToPixel( y1, ClipInfinite );
	
				double k = (_y1-_y0)/(_x1-_x0);
	
				double closest_x;
				if ( k == 0 )
					closest_x = _x0;
				else
					closest_x = (pixelPos.y() + pixelPos.x()/k + k*_x0 - _y0) / (k + 1.0/k);
	
				double closest_y = yToPixel( value( plot, 0, xToReal( closest_x ), false ), ClipInfinite );
	
				double dfx = qAbs( closest_x - pixelPos.x() );
				double dfy = qAbs( closest_y - pixelPos.y() );
	
				double distance = sqrt( dfx*dfx + dfy*dfy );
				if ( distance < best_distance )
				{
					best_distance = distance;
					best_pixel_x = closest_x;
				}
			}
	
			best_x = xToReal( best_pixel_x );
			break;
		}
		
		case Function::Polar:
		case Function::Parametric:
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
			break;
		}
	}
	
	return best_x;
}


double View::pixelDistance( const QPointF & pos, const Plot & plot, double x, bool updateFunction )
{
	QPointF f = realValue( plot, x, updateFunction );
	QPointF df = toPixel( pos, ClipInfinite ) - toPixel( f, ClipInfinite );

	return std::sqrt( df.x()*df.x() + df.y()*df.y() );
}


QString View::posToString( double x, double delta, PositionFormatting format, QColor color  ) const
{
	delta = qAbs(delta);
	if ( delta == 0 )
		delta = 1;

	QString numberText;

	int decimalPlaces = 1-int(log(delta)/log(10.0));
	
	// Avoid exponential format for smallish numbers
	if ( 0.01 < qAbs(x) && qAbs(x) < 10000 )
		format = DecimalFormat;

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
				numberText = QString::number( x*(pow(10.0,decimalPlaces)), 'f', 0 ) + QString( -decimalPlaces, '0' );

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
		setStatusBar( QString(), RootSection );

	QString sx, sy;

	if ( inBounds )
	{
		sx = "x = " + posToString( m_crosshairPosition.x(), (m_xmax-m_xmin)/m_clipRect.width(), View::DecimalFormat );
		sy = "y = " + posToString( m_crosshairPosition.y(), (m_ymax-m_ymin)/m_clipRect.width(), View::DecimalFormat );
	}
	else
		sx = sy = "";

	setStatusBar( sx, XSection );
	setStatusBar( sy, YSection );

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
			 (m_popupMenuStatus != NoPopup) &&
			 !m_popupMenu->isVisible() )
	{
		if ( m_popupMenuStatus==Popup)
			m_currentPlot.setFunctionID( -1 );
		m_popupMenuStatus = NoPopup;;
	}

	update();
	updateCursor();
}


bool View::updateCrosshairPosition()
{
	QPointF mousePos = mapFromGlobal( QCursor::pos() );

	bool out_of_bounds = false; // for the ypos
	
	m_crosshairPosition = toReal( mousePos );

	m_currentPlot.updateFunction();
	Function * it = m_currentPlot.function();

	if ( it && crosshairPositionValid( it ) && (m_popupMenuStatus != Popup) )
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
		else if ( it->type() == Function::Implicit )
		{
			double x = m_crosshairPosition.x();
			double y = m_crosshairPosition.y();
			findRoot( & x, & y, m_currentPlot, PreciseRoot );
			m_crosshairPosition = QPointF( x, y );
		}
		else
		{
			// cartesian or differential plot

			m_crosshairPosition.setY( value( m_currentPlot, 0, m_crosshairPosition.x(), false ) );
			mousePos.setY( yToPixel( m_crosshairPosition.y() ));

			if ( m_crosshairPosition.y()<m_ymin || m_crosshairPosition.y()>m_ymax) //the ypoint is not visible
			{
				out_of_bounds = true;
			}
			else if ( (fabs(yToReal(mousePos.y())) < (m_ymax-m_ymin)/80) && (it->type() == Function::Cartesian || it->type() == Function::Differential) )
			{
				double x0 = m_crosshairPosition.x();
				if ( !m_haveRoot && findRoot( &x0, m_currentPlot, PreciseRoot ) )
				{
					QString str="  ";
					str += i18n("root") + ":  x" + QChar(0x2080) + " = ";
					setStatusBar( str+QString().sprintf("%+.5f", x0), RootSection );
					m_haveRoot=true;
				}
			}
			else
				m_haveRoot=false;
		}

		// For Cartesian plots, only adjust the cursor position if it is not at the ends of the view
		if ( ((it->type() != Function::Cartesian) && (it->type() != Function::Differential)) || m_clipRect.contains( mousePos.toPoint() ) )
		{
			mousePos = toPixel( m_crosshairPosition );
			QPoint globalPos = mapToGlobal( mousePos.toPoint() );
			QCursor::setPos( globalPos );
		}
	}

	m_crosshairPixelCoords = mousePos;

	return !out_of_bounds && m_clipRect.contains( mousePos.toPoint() );
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
	QPointF real = toReal( mousePos );

	double diffx = (m_xmax-m_xmin)*zoomFactor;
	double diffy = (m_ymax-m_ymin)*zoomFactor;

	animateZoom( QRectF( real.x()-diffx, real.y()-diffy, 2.0*diffx, 2.0*diffy ) );
}


void View::zoomIn( const QRect & zoomRect )
{
	QPointF p = zoomRect.topLeft();
	double real1x = xToReal(p.x() );
	double real1y = yToReal(p.y() );
	p = zoomRect.bottomRight();
	double real2x = xToReal(p.x() );
	double real2y = yToReal(p.y() );

	if ( real1x > real2x )
		qSwap( real1x, real2x );
	if ( real1y > real2y )
		qSwap( real1y, real2y );

	animateZoom( QRectF( QPointF( real1x, real1y ), QSizeF( real2x-real1x, real2y-real1y ) ) );
}


void View::zoomOut( const QRect & zoomRect )
{
	QPointF p = zoomRect.topLeft();
	double _real1x = xToReal(p.x() );
	double _real1y = yToReal(p.y() );
	p = zoomRect.bottomRight();
	double _real2x = xToReal(p.x() );
	double _real2y = yToReal(p.y() );

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

	drawPlot(); //update all graphs

	m_zoomMode = Normal;
}


void View::translateView( int dx, int dy )
{
	double rdx = xToReal( dx ) - xToReal( 0.0 );
	double rdy = yToReal( dy ) - yToReal( 0.0 );

	m_xmin += rdx;
	m_xmax += rdx;
	m_ymin += rdy;
	m_ymax += rdy;

	Settings::setXMin( Parser::number( m_xmin ) );
	Settings::setXMax( Parser::number( m_xmax ) );
	Settings::setYMin( Parser::number( m_ymin ) );
	Settings::setYMax( Parser::number( m_ymax ) );

	drawPlot(); //update all graphs
}

void View::getSettings()
{
	//BEGIN get X/Y range
	m_xmin = XParser::self()->eval( Settings::xMin() );
	m_xmax = XParser::self()->eval( Settings::xMax() );
	if ( m_xmax <= m_xmin )
	{
		m_xmin = -8;
		m_xmax = +8;
	}
	
	m_ymin = XParser::self()->eval( Settings::yMin() );
	m_ymax = XParser::self()->eval( Settings::yMax() );
	if ( m_ymax <= m_ymin )
	{
		m_ymin = -8;
		m_ymax = +8;
	}
	//END get X/Y range
	
	
	//BEGIN get Tic Separation
	if ( Settings::xScalingMode() == 0 )
		ticSepX.updateExpression( (m_xmax-m_xmin)/16 ); // automatic x-scaling
	else
		ticSepX.updateExpression( Settings::xScaling() );

	if ( Settings::yScalingMode() == 0 )
		ticSepY.updateExpression( (m_ymax-m_ymin)/16 ); // automatic x-scaling
	else
		ticSepY.updateExpression( Settings::yScaling() );
	//END get Tic Separation
	
	
	//BEGIN get colours
	m_backgroundColor = Settings::backgroundcolor();
	if ( !m_backgroundColor.isValid() )
		m_backgroundColor = Qt::white;

	QPalette palette;
	palette.setColor( backgroundRole(), m_backgroundColor );
	setPalette(palette);
	//END get colours

	
	XParser::self()->setAngleMode( (Parser::AngleMode)Settings::anglemode() );
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
		m_stopCalculating = true;
}


QPointF View::findMinMaxValue( const Plot & plot, ExtremaType type, double dmin, double dmax )
{
	Function * ufkt = plot.function();
	assert( (ufkt->type() == Function::Cartesian) || (ufkt->type() == Function::Differential) );
	
	plot.updateFunction();
	
	Plot differentiated = plot;
	differentiated.differentiate();
	QList<double> roots = findRoots( differentiated, dmin, dmax, RoughRoot );
	
	// The minimum / maximum might occur at the end points
	roots << dmin << dmax;
	
	double best = (type == Maximum) ? -HUGE_VAL : +HUGE_VAL;
	QPointF bestPoint;
	
	foreach ( double root, roots )
	{
		QPointF rv = realValue( plot, root, false );
		if ( (type == Maximum && rv.y() > best) || (type == Minimum && rv.y() < best) )
		{
			best = rv.y();
			bestPoint = rv;
		}
	}
	
	return bestPoint;
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
		m_stopCalculating=true;
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
				m_currentPlot.functionID = (*it)->id();
			}
		}

		kDebug() << "************************" << endl;
		kDebug() << "m_currentPlot.functionID: " << (int)m_currentPlot.functionID << endl;
		kDebug() << "m_currentPlot.plotMode: " << (int)m_currentPlot.plotMode << endl;
		kDebug() << "m_currentFunctionParameter: " << m_currentFunctionParameter << endl;

		setStatusBar( (*it)->prettyName( m_currentPlot.plotMode ), FunctionSection );

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
	int sign = 1;
	if ( s.dmax < s.dmin )
	{
		qSwap( s.dmin, s.dmax );
		sign = -1;
	}
	
	else if ( s.dmax == s.dmin )
		return 0;

	Function * ufkt = s.plot.function();
	assert( ufkt );

	double dx = (s.dmax-s.dmin)/m_clipRect.width();
	if ( s.plot.plotMode == Function::Integral )
	{
		double max_dx = ufkt->eq[0]->differentialStates.step().value();
		if ( dx > max_dx )
			dx = max_dx;
	}

	// Make sure that we calculate the exact area (instead of missing out a
	// vertical slither at the end) by making sure dx tiles the x-range
	// a whole number of times
	int intervals = qRound( (s.dmax-s.dmin)/dx );
	dx = (s.dmax-s.dmin) / intervals;

	double calculated_area=0;
	double x = s.dmin;

	s.plot.updateFunction();

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
	m_integralDrawSettings.draw = true;
	drawPlot();
	m_integralDrawSettings.draw = false;
	return calculated_area * sign;
}

bool View::isCalculationStopped()
{
	if ( m_stopCalculating)
	{
		m_stopCalculating = false;
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
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if ( it->m_parameters.useSlider && !it->allPlotsAreHidden() )
		{
			if ( !m_sliderWindow )
			{
				m_sliderWindow = new KSliderWindow( this );
				connect( m_sliderWindow, SIGNAL( valueChanged() ), this, SLOT( drawPlot() ) );
				connect( m_sliderWindow, SIGNAL( windowClosed() ), this, SLOT( sliderWindowClosed() ) );
			}
			m_sliderWindow->show();
			m_menuSliderAction->setChecked( false );  //set the slider-item in the menu
		}
	}
}

void View::hideCurrentFunction()
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
void View::removeCurrentPlot()
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


void View::animateFunction()
{
	Function * f = m_currentPlot.function();
	if ( !f )
		return;
	
	ParameterAnimator * anim = new ParameterAnimator( this, f );
	anim->show();
}


void View::editCurrentPlot()
{
	MainDlg::self()->functionEditor()->setCurrentFunction( m_currentPlot.functionID() );
}


void View::zoomIn()
{
	m_zoomMode = ZoomIn;
	updateCursor();
}


void View::zoomOut()
{
	m_zoomMode = ZoomOut;
	updateCursor();
}


void View::zoomToTrigonometric()
{
	double rpau = XParser::self()->radiansPerAngleUnit();
	animateZoom( QRectF( -(47.0/24.0)*M_PI/rpau, -4.0, (47.0/12.0)*M_PI/rpau, 8.0 ) );
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
	
	if ( m_popupMenuStatus != NoPopup )
		return false;

	Function * it = m_currentPlot.function();

	QPoint mousePos = mapFromGlobal( QCursor::pos() );

	return ( underMouse() && (!it || crosshairPositionValid( it )) );
}


bool View::event( QEvent * e )
{
	if ( e->type() == QEvent::WindowDeactivate && m_isDrawing)
	{
		m_stopCalculating = true;
		return true;
	}
	return QWidget::event(e); //send the information further
}


void View::setStatusBar( const QString & t, StatusBarSection section )
{
	QString text;
	if ( section == FunctionSection )
		text = " " + t + " ";
	else
		text = t;
	
	if ( m_readonly) //if KmPlot is shown as a KPart with e.g Konqueror, it is only possible to change the status bar in one way: to call setStatusBarText
	{
		m_statusBarText[ section - 1 ] = text;
		
		QString text;
		for ( int i = 0; i < 4; ++i )
		{
			if ( m_statusBarText[i].isEmpty() )
				continue;
			
			if ( !text.isEmpty() )
				text.append( "  |  " );
			
			text.append( m_statusBarText[i] );
		}
		
		emit setStatusBarText(text);
	}
	else
	{
		QDBusReply<void> reply = QDBusInterface( QDBus::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.KmPlot" ).call( QDBus::NoBlock, "setStatusBarText", text, (int)section );
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
	draw = false;
}
//END class IntegralDrawSettings

#include "view.moc"
