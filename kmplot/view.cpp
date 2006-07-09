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
#include "kminmax.h"
#include "settings.h"
#include "ksliderwindow.h"
#include "maindlg.h"
#include "parameteranimator.h"
#include "view.h"
#include "viewadaptor.h"


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
	ticSepX = ticSepY = drskalx = drskaly = 0.0;
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

	/// \todo fix title on popup menu (should display function name)
// 	m_popupmenu->addTitle( " " );
}

void View::setMinMaxDlg(KMinMax *minmaxdlg)
{
	m_minmax = minmaxdlg;
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
	
	double lx, ly;
	double sf;
	QRect rc;
	QPainter DC;    // our painter
	DC.begin(dev);    // start painting widget
	rc=DC.viewport();
	m_width = rc.width();
	m_height = rc.height();
	initDrawLabels();

	switch ( medium )
	{
		case Screen:
		{
			QPointF ref(120, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/ticSepX);
			ly=((m_ymax-m_ymin)*100.*drskaly/ticSepY);
			DC.scale( m_height/(ly+2*ref.y()), m_height/(ly+2*ref.y()));
			if ( ( QPointF(lx+2*ref.x(), ly) * DC.matrix() ).x() > DC.viewport().right())
			{
				DC.resetMatrix();
				DC.scale( m_width/(lx+2*ref.x()), m_width/(lx+2*ref.x()));
			}
			wm = DC.matrix();
			m_scaler=( QPoint(1000, 0) * DC.matrix() ).x()/1000.;
			initDiagram( ref, lx, ly );
			break;
		}

		case Printer:
		{
			sf=72./254.;        // 72dpi
			QPointF ref(100, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/ticSepX);
			ly=((m_ymax-m_ymin)*100.*drskaly/ticSepY);
			DC.scale(sf, sf);
			m_scaler = 1.;
			m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1";
			drawHeaderTable( &DC );
			initDiagram( ref, lx, ly );
			if ( ( (KPrinter* )dev )->option( "app-kmplot-printbackground" ) == "-1" )
				DC.fillRect( m_frame,  m_backgroundColor); //draw a colored background
			//DC.end();
			//((QPixmap *)dev)->fill(QColor("#FF00FF"));
			//DC.begin(dev);
			break;
		}

		case SVG:
		{
			QPointF ref(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/ticSepX);
			ly=((m_ymax-m_ymin)*100.*drskaly/ticSepY);
			initDiagram( ref, lx, ly );
			DC.translate(-m_frame.left(), -m_frame.top());
			m_scaler=1.;
			break;
		}

		case Pixmap:
		{
			sf=180./254.;								// 180dpi
			QPointF ref(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/ticSepX);
			ly=((m_ymax-m_ymin)*100.*drskaly/ticSepY);
			initDiagram( ref, lx, ly );
			DC.end();
			*((QPixmap *)dev) = QPixmap( (int)(m_frame.width()*sf), (int)(m_frame.height()*sf) );
			((QPixmap *)dev)->fill(m_backgroundColor);
			DC.begin(dev);
			DC.translate(-m_frame.left()*sf, -m_frame.top()*sf);
			DC.scale(sf, sf);
			m_scaler=1.;
			break;
		}
	}
	
	ticStartX = ceil(m_xmin/ticSepX)*ticSepX;
	ticStartY = ceil(m_ymin/ticSepY)*ticSepY;
	area = DC.matrix().mapRect( m_plotArea );

	DC.setRenderHint( QPainter::Antialiasing, true );
	drawDiagram( &DC );
	
	updateCursor();
	stop_calculating = false;

	// Antialiasing slows down rendering a lot, so turn it off if we are
	// sliding the view about
	DC.setRenderHint( QPainter::Antialiasing, m_zoomMode != Translating );
	
	DC.setClipping( true );
	DC.setClipRect( m_plotArea );
	foreach ( Function * ufkt, XParser::self()->m_ufkt )
	{
		if ( stop_calculating )
			break;

		if ( ufkt->type() == Function::Implicit )
			drawImplicit( ufkt, & DC );
		else
		{
			QList<Plot> plots = ufkt->allPlots();
			foreach ( Plot plot, plots )
				drawPlot( plot, &DC );
		}
	}
	drawFunctionInfo( &DC );
	DC.setClipping( false );

	m_isDrawing=false;
	updateCursor();
	DC.end();   // painting done
}


void View::initDiagram( QPointF Ref, 			    // Bezugspunkt links unten
						double lx, double ly) 			// Achsenl�gen
{
	int x, y, h, w;
	
	ticStartX = ceil(m_xmin/ticSepX)*ticSepX;
	ticStartY = ceil(m_ymin/ticSepY)*ticSepY;
	skx = lx/(m_xmax-m_xmin);			        // Skalierungsfaktoren berechnen
	sky = ly/(m_ymax-m_ymin);
	ox = Ref.x()-skx*m_xmin+0.5;	        // Ursprungskoordinaten berechnen
	oy = Ref.y()+sky*m_ymax+0.5;
	
	x = int(Ref.x());
	y = int(Ref.y());
	w = int(lx);
	h = int(ly);
	m_plotArea.setRect( x, y, w, h );
	
	if( Settings::showExtraFrame() )
	{
		x-=20;
		y-=20;
		w+=40;
		h+=40;

		if( Settings::showLabel() && m_ymin>=0. )
			h+=60;
	}

	m_frame.setRect(x, y, w, h);
}


QPointF View::toPixel( const QPointF & real, ClipBehaviour clipBehaviour )
{
	double x = xToPixel( real.x(), clipBehaviour );
	double y = yToPixel( real.y(), clipBehaviour );
	return QPointF( x, y );
}


double View::xToPixel( double x, ClipBehaviour clipBehaviour )		// reale x-Koordinate
{
	double xi;                			// transformierte x-Koordinate
	static double lastx;            // vorherige x-Koordinate
	if(isnan(x))
	{
		xclipflg=1;
		if(lastx<1. && lastx>-1.)
			xi=(ox-skx*lastx);
		else
			xi=(lastx<0)? m_plotArea.left(): m_plotArea.right();
	}
	else if(isinf(x)==-1)
	{
		xclipflg=0;
		xi=m_plotArea.left();
	}
	else if(isinf(x)==1)
	{
		xclipflg=0;
		xi=m_plotArea.right();
                
	}
	else if ( (x<m_xmin) && (clipBehaviour == ClipAll) )
	{
		xclipflg=1;
		xi=m_plotArea.left();
	}
	else if ( (x>m_xmax) && (clipBehaviour == ClipAll) )
	{
		xclipflg=1;
		xi=m_plotArea.right();
	}
	else
	{
		xclipflg=0;
		xi=(ox+skx*x);
	}

	lastx=x;
	return xi;
}


double View::yToPixel( double y, ClipBehaviour clipBehaviour )		// reale y-Koordinate
{   
	double yi;                     	// transformierte y-Koordinate
	static double lasty;            // vorherige y-Koordinate
	if(isnan(y))
	{
		yclipflg=1;
		if(lasty<1. && lasty>-1.)
			yi=(oy-sky*lasty);
		else
			yi=(lasty<0)? m_plotArea.bottom(): m_plotArea.top();
	}
	else if(isinf(y)==-1)
	{
		yclipflg=0;
		yi=m_plotArea.bottom();
                
	}
	else if(isinf(y)==1)
	{
		yclipflg=0;
		yi=m_plotArea.top();
                
	}
	else if ( (y<m_ymin) && (clipBehaviour == ClipAll) )
	{
		yclipflg=1;
		yi=m_plotArea.bottom();
	}
	else if ( (y>m_ymax) && (clipBehaviour == ClipAll) )
	{
		yclipflg=1;
		yi=m_plotArea.top();
	}
	else
	{
		yclipflg=0;
		yi=(oy-sky*y);
	}

	lasty=y;
	return yi;
}


QPointF View::toReal( const QPointF & pixel )
{
	double x = xToReal( pixel.x() );
	double y = yToReal( pixel.y() );
	return QPointF( x, y );
}


double View::xToReal(double x) 		// Bildschirmkoordinate
{   return (x-ox)/skx;     			// reale x-Koordinate
}


double View::yToReal(double y)        // Bildschirmkoordinate
{   return (oy-y)/sky;     			// reale y-Koordinate
}


void View::drawDiagram( QPainter * pDC )
{
	QColor frameColor = qRgb(0, 0, 0);
	
	double borderThickness = 0.2;
	QPen pen( frameColor, mmToPenWidth( borderThickness, true ) );

	drawGrid( pDC ); // draw the grid
	drawAxes( pDC ); // draw the axes
	if( Settings::showLabel() )
		drawLabels(pDC);        // Achsen beschriften
	if( Settings::showFrame() || Settings::showExtraFrame() )// FRAME zeichnen
	{
		pDC->setPen(pen);
		pDC->drawRect(m_frame);
	}
}


void View::drawAxes( QPainter* pDC )	// draw axes
{
	double axesLineWidth = Settings::axesLineWidth();
	double ticWidth = Settings::ticWidth();
	double ticLength = Settings::ticLength();
	QColor axesColor = Settings::axesColor();
	
	double a, b, tl;
	double d, da, db;
	
	if( Settings::showAxes() )
	{
		pDC->save();
		
		int const dx=14;
		int const dy=8;
		
		pDC->setPen( QPen( axesColor, mmToPenWidth(axesLineWidth, true) ) );
		pDC->setBrush( axesColor );
		
		a=m_plotArea.right();
		b=yToPixel(0.);
		pDC->Lineh(m_plotArea.left(), b, a);	    // x-Achse
		if( Settings::showArrows()) 		    			// ARROWS
		{
			QPolygonF p(3);
			p[0] = QPointF( a, b );
			p[1] = QPointF( a-dx, b+dy );
			p[2] = QPointF( a-dx, b-dy );
			pDC->drawPolygon( p );
		}

		a=xToPixel(0.);
		b=m_plotArea.top();
		pDC->Linev(a, m_plotArea.bottom(), b); 	    // y-Achse
		if( Settings::showArrows() )   					// ARROWS
		{
			QPolygonF p(3);
			p[0] = QPointF( a, b );
			p[1] = QPointF( a-dy, b+dx );
			p[2] = QPointF( a+dy, b+dx );
			pDC->drawPolygon( p );
		}
		
		pDC->restore();
	}

	pDC->setPen( QPen( axesColor, mmToPenWidth(ticWidth, true) ) );
	if( Settings::showAxes() )
	{
		da=oy-(ticLength*10.0);
		db=oy+(ticLength*10.0);
		tl= Settings::showFrame()? 0: (ticLength*10.0);
		d=ticStartX;
		if(da<(double)m_plotArea.top())
		{
			a=m_plotArea.top()-tl;
			b=m_plotArea.top()+int(10.0*ticLength);
		}
		else if(db>(double)m_plotArea.bottom())
		{
			b=m_plotArea.bottom()+tl;
			a=m_plotArea.bottom()-(10.0*ticLength);
		}
		else
		{
			a=da;
			b=db;
		}

		while(d<m_xmax-ticSepX/2.)
		{
			pDC->Linev(xToPixel(d), a, b);
			d+=ticSepX;
		}

		da=ox-(10.0*ticLength);
		db=ox+(10.0*ticLength);
		d=ticStartY;
		if(da<(double)m_plotArea.left())
		{
			a=m_plotArea.left()-tl;
			b=m_plotArea.left()+(10.0*ticLength);
		}
		else if(db>(double)m_plotArea.right())
		{
			b=m_plotArea.right()+tl;
			a=m_plotArea.right()-(10.0*ticLength);
		}
		else
		{
			a=da;
			b=db;
		}

		while(d<m_ymax-ticSepY/2.)
		{
			pDC->Lineh(a, yToPixel(d), b);
			d+=ticSepY;
		}
	}
	else if( Settings::showFrame() )
	{
		a=m_plotArea.bottom()+(ticLength*10.0);
		b=m_plotArea.top()-(ticLength*10.0);
		d=ticStartX;
		while(d<m_xmax)
		{
			pDC->Linev(xToPixel(d), m_plotArea.bottom(), a);
			pDC->Linev(xToPixel(d), m_plotArea.top(), b);
			d+=ticSepX;
		}

		a=m_plotArea.left()+(ticLength*10.0);
		b=m_plotArea.right()-(ticLength*10.0);
		d=ticStartY;
		while(d<m_ymax)
		{
			pDC->Lineh(m_plotArea.left(), yToPixel(d), a);
			pDC->Lineh(m_plotArea.right(), yToPixel(d), b);
			d+=ticSepY;
		}
	}
}


void View::drawGrid( QPainter* pDC )
{
	QColor gridColor = Settings::gridColor();
	
	double gridLineWidth = Settings::gridLineWidth();
	QPen pen( gridColor, mmToPenWidth(gridLineWidth, true) );

	pDC->setPen(pen);
	
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
			for ( double d = ticStartX; d <= m_xmax; d += ticSepX )
				pDC->Linev(xToPixel(d), m_plotArea.bottom(), m_plotArea.top());
			
			for ( double d = ticStartY; d <= m_ymax; d += ticSepY )
				pDC->Lineh(m_plotArea.left(), yToPixel(d), m_plotArea.right());
			
			break;
		}
		
		case GridCrosses:
		{
			int const dx = 5;
			int const dy = 5;

			for( double x = ticStartX; x<m_xmax; x+=ticSepX)
			{
				double a = xToPixel(x);
				for( double y=ticStartY; y<m_ymax; y+=ticSepY)
				{
					double b = yToPixel(y);
					pDC->Lineh(a-dx, b, a+dx);
					pDC->Linev(a, b-dy, b+dy);
				}
			}
		}
		
		case GridPolar:
		{
			// Note: 1.42 \approx sqrt(2)
			
			pDC->setClipRect( m_plotArea );
			
			double xMax = qMax( qAbs(m_xmin), qAbs(m_xmax) ) * 1.42;
			double yMax = qMax( qAbs(m_ymin), qAbs(m_ymax) ) * 1.42;
			
			// The furthest pixel away from the origin
			double pixelMax = qMax( xMax*skx, yMax*sky );
			
			double x = ticSepX;
			double y = ticSepY;
			
			while ( x <= xMax || y <= yMax )
			{
				QRectF r;
				r.setTopLeft(		toPixel( QPointF( -x, y ), ClipInfinite ) ); 
				r.setBottomRight(	toPixel( QPointF( x, -y ), ClipInfinite ) );
				
				pDC->drawEllipse( r );
				
				x += ticSepX;
				y += ticSepY;
			}
			
			for ( double theta = 0; theta < 2.0*M_PI; theta += M_PI/12.0 )
			{
				QPointF start = toPixel( QPointF( 0, 0 ), ClipInfinite );
				QPointF end = start + QPointF( pixelMax * cos(theta), pixelMax * sin(theta) );
				
				pDC->drawLine( start, end );
			}
			
			pDC->setClipping( false );
			break;
		}
	}
}


void View::drawLabels(QPainter* pDC)
{
	QColor axesColor = Settings::axesColor();
	int const dx=15;
	int const dy=40;
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

	for(d=ticStartX, n=(int)ceil(m_xmin/ticSepX); d<m_xmax; d+=ticSepX, ++n)
	{
		if(n==0 || fabs(d-m_xmax)<=1.5*ticSepX)
			continue;
		
		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ticSepX-M_PI/frac[i])> 1e-3 )
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
			
			break;
		}
		
		if ( !found && (n%5==0 || n==1 || n==-1 || draw_next))
		{
			s = posToString( n*ticSepX, (m_xmax-m_xmin)/4, View::ScientificFormat, axesColor );
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
			
			QPointF drawPoint( xToPixel(d)-(br.width()/2), y+dy-(br.height()/2) );
			
			pDC->translate( drawPoint );
			m_textDocument->documentLayout()->draw( pDC, QAbstractTextDocumentLayout::PaintContext() ); 
			pDC->translate( -drawPoint );
			
		}
	}

	QRectF drawRect;
	
	if ( m_ymax<0 && m_xmax<0 )
		drawRect = QRectF( xToPixel(m_xmax)-(4*dx), y+(dy-20), 0, 0 );
	else
		drawRect = QRectF( xToPixel(m_xmax)-dx, y+dy, 0, 0 );
	pDC->drawText( drawRect, Qt::AlignCenter|Qt::TextDontClip, "x" );

	for(d=ticStartY, n=(int)ceil(m_ymin/ticSepY); d<m_ymax; d+=ticSepY, ++n)
	{
		if(n==0 || fabs(d-m_ymax)<=1.5*ticSepY)
			continue;

		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ticSepY-M_PI/frac[i])> 1e-3 )
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
			
			break;
		}
		if( !found && (n%5==0 || n==1 || n==-1))
		{
			s = posToString( n*ticSepY, (m_ymax-m_ymin)/4, View::ScientificFormat, axesColor );
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
		return qMin( (m_xmax-m_xmin)/area.width(), (m_ymax-m_ymin)/area.height() );
	
	
}


double View::value( const Plot & plot, int eq, double x, bool updateFunction )
{
	Function * function = plot.function();
	assert( function );
	
	if ( updateFunction )
		plot.updateFunction();
	
	Equation * equation = function->eq[eq];
	
	double dx = h( plot );
	if ( plot.function()->type() == Function::Differential )
		return XParser::self()->differential( equation, & equation->differentialStates[ plot.state ], x, dx );
	else
		return XParser::self()->derivative( plot.derivativeNumber(), equation, x, dx );
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
			return QPointF( y * cos(x), y * sin(x) );
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
			case Function::Polar:
				min = 0.0;
				break;

			case Function::Parametric:
				min = -M_PI;
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
			case Function::Polar:
				max = 2.0*M_PI;
				break;

			case Function::Parametric:
				max = M_PI;
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
	
	const QList< Plot > plots = function->allPlots();
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
		
		painter->setPen( penForPlot( plot, painter->renderHints() & QPainter::Antialiasing ) );
		
		
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
		FuzzyPoint::dx = (m_xmax-m_xmin) * SegmentMin * 0.1 / area.width();
		FuzzyPoint::dy = (m_ymax-m_ymin) * SegmentMin * 0.1 / area.height();
		foreach ( QPointF point, singular )
			singularSorted.insert( point, point );
		singular = singularSorted.values();
		
		foreach ( QPointF point, singular )
		{
			// radius of circle around singular point
			double epsilon = qMin( FuzzyPoint::dx, FuzzyPoint::dy );
			
			QString fstr;
			fstr = QString("%1(x)=%2(%3+%6*cos(x),%4+%6*sin(x),%5)")
					.arg( circular.function()->eq[0]->name() )
					.arg( function->eq[0]->name() )
					.arg( XParser::self()->number( point.x() ) )
					.arg( XParser::self()->number( point.y() ) )
					.arg( XParser::self()->number( function->k ) )
					.arg( XParser::self()->number( epsilon ) );
			
			bool setFstrOk = circular.function()->eq[0]->setFstr( fstr );
			assert( setFstrOk );
			
			QList<double> roots = findRoots( circular, 0, 2*M_PI, PreciseRoot );
			
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
				double x = point.x() + epsilon * cos(t);
				double y = point.y() + epsilon * sin(t);
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
	// before drawing or commiting itself to anything potentially bad
	QPointF prev2 = toPixel( QPointF( x, y ), ClipInfinite );
	QPointF prev1 = prev2;
	
	// Allow us to doubly retrace
	double prev_diff_x = 0;
	double prev_diff_y = 0;
	
	for ( int i = 0; i < 500; ++i ) // allow a maximum of 100 traces (to prevent possibly infinite loop)
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
		
		double angle = arctan(ty/tx) + ((tx<0) ? M_PI : 0);
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
	
	painter->setPen( penForPlot( plot, painter->renderHints() & QPainter::Antialiasing ) );
	
	double dmin = getXmin( function );
	double dmax = getXmax( function );
	
	// the 'middle' dx, which may be increased or decreased
	double base_dx = (dmax-dmin)/area.width();
	if ( (function->type() == Function::Parametric) || (function->type() == Function::Polar) )
		base_dx *= 0.01;

	// Increase speed while translating the view
	bool quickDraw = ( m_zoomMode == Translating );
	if ( quickDraw )
		base_dx *= 4.0;
	
	double dx = base_dx;
	
	bool drawIntegral = m_drawIntegral && (m_integralDrawSettings.plot == plot);
	double totalLength = 0.0; // total pixel length; used for drawing dotted lines
	
	bool p1Set = false;
	QPointF p1, p2;
	
	double x = dmin;
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
				continue;
			}
		}
			
		p2 = toPixel( rv, ClipInfinite );

		if ( xclipflg || yclipflg || !p1Set )
		{
			p1 = p2;
			x += dx;
			p1Set = true;
			continue;
		}
		
		
		//BEGIN adjust dx
		QPointF p1_pixel = p1 * painter->matrix();
		QPointF p2_pixel = p2 * painter->matrix();
		QRectF bound = QRectF( p1_pixel, QSizeF( (p2_pixel-p1_pixel).x(), (p2_pixel-p1_pixel).y() ) ).normalized();
		double length = QLineF( p1_pixel, p2_pixel ).length();
		totalLength += length;

		double min_mod = (function->type() == Function::Cartesian || function->type() == Function::Differential) ? 0.1 : 5e-5;
		double max_mod = (function->type() == Function::Cartesian || function->type() == Function::Differential) ? 1e+1 : 5e+1;
		bool dxAtMinimum = (dx <= base_dx*min_mod);
		bool dxAtMaximum = (dx >= base_dx*max_mod);
		bool dxTooBig = false;
		bool dxTooSmall = false;
		
		if ( QRectF( area ).intersects( bound ) )
		{
			dxTooBig = !dxAtMinimum && (length > (quickDraw ? max_mod : 4.0));
			dxTooSmall = !dxAtMaximum && (length < (quickDraw ? 10.0 : 1.0));
		}
		else
			dxTooSmall = !dxAtMaximum;
			
		if ( dxTooBig )
		{
			dx *= 0.5;
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
	
	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		if ( stop_calculating )
			break;
		
		foreach ( Plot plot, function->allPlots() )
		{
			// Draw extrema points?
			if ( (function->type() == Function::Cartesian) && function->plotAppearance( plot.plotMode ).showExtrema )
			{
				QList<QPointF> stationaryPoints = findStationaryPoints( plot );
				foreach ( QPointF realValue, stationaryPoints )
				{
					painter->setPen( QPen( Qt::black, 10 ) );
					painter->drawPoint( toPixel( realValue ) );
					
					QString x = posToString( realValue.x(), (m_xmax-m_xmin)/area.width(), View::DecimalFormat );
					QString y = posToString( realValue.y(), (m_ymax-m_ymin)/area.width(), View::DecimalFormat );
					
					drawLabel( painter, plot.color(), realValue, QString( "x = %1   y = %2" ).arg(x).arg(y) );
				}
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
	for ( double x = pixelCenter.x() - 400; x <= pixelCenter.x() + 400; x += 40 )
	{
		for ( double y = pixelCenter.y() - 400; y <= pixelCenter.y() + 400; y += 40 )
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


QRect View::usedDiagramRect( QRectF rect ) const
{
	if ( !area.isValid() )
		return QRect();
	
	rect = wm.mapRect( rect );
	rect = rect & area;
	
	double x0 = (rect.left() - area.left()) / area.width();
	double x1 = (rect.right() - area.left()) / area.width();
	
	double y0 = (rect.top() - area.top()) / area.height();
	double y1 = (rect.bottom() - area.top()) / area.height();
	
	int i0 = int( x0 * LabelGridSize );
	int i1 = int( x1 * LabelGridSize );
	int j0 = int( y0 * LabelGridSize );
	int j1 = int( y1 * LabelGridSize );
	
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


void View::markDiagramPointUsed( QPointF point )
{
	if ( m_zoomMode == Translating )
		return;
	
	point = wm.map( point );
	if ( ! QRectF(area).contains(point) )
		return;
	
	double x = (point.x() - area.left()) / area.width();
	double y = (point.y() - area.top()) / area.height();
	
	int i = int( x * LabelGridSize );
	int j = int( y * LabelGridSize );
	
	m_usedDiagramArea[i][j] = true;
}


int View::rectCost( const QRectF & rect ) const
{
	int cost = 0;
	
	// If the rectangle goes off the edge, mark it as very high cost)
	QRectF mapped = wm.mapRect( rect );
	QRectF intersect = mapped & area;
	cost += int(mapped.width() * mapped.height()) - int(intersect.width() * intersect.height());
	
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
	
	PlotAppearance appearance = ufkt->plotAppearance( plot.plotMode );
	
	double lineWidth_mm = appearance.lineWidth;
	pen.setColor( plot.color() );
	
	if ( appearance.style == Qt::SolidLine )
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
		atx="1E  =  "+ticSepXstr;
		setpi(&atx);
		aty="1E  =  "+ticSepYstr;
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
			foreach ( Equation * eq, it->eq )
			{
				if ( stop_calculating )
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
		break;
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
	
	double h = qMin( m_xmax-m_xmin, m_ymax-m_ymin ) * 1e-5;

	double f = value( plot, 0, *x, false );
	int k;
	for ( k=0; k < max_k; ++k )
	{
		double df = XParser::self()->derivative( n, eq, *x, h );
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
		double dfx = XParser::self()->derivative( n, eq, *x, hx );
		
		function->m_implicitMode = Function::FixedX;
		double dfy = XParser::self()->derivative( n, eq, *y, hy );
		
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
		p.save();
		p.setMatrix( wm );
		QPointF tl( xToPixel( m_animateZoomRect.left() ), yToPixel( m_animateZoomRect.top() ) );
		QPointF br( xToPixel( m_animateZoomRect.right() ), yToPixel( m_animateZoomRect.bottom() ) );
		p.drawRect( QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) ) );
		p.restore();
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
			
			p.save();
			p.setMatrix( wm );
			p.setClipRect( wm.inverted().mapRect( area ) );
			
			p.restore();
			p.setClipRect( area );
			
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
			
			if ( k > 1e-5 )
			{
				p.save();
				
				// Transform the painter so that the center of the osculating circle is the origin,
				// with the normal line coming in frmo the left.
				QPointF center = m_crosshairPixelCoords + (1/k) * QPointF( cos( normalAngle ), sin( normalAngle ) );
				p.translate( center );
				p.rotate( normalAngle * 180 / M_PI );
				
				// draw osculating circle
// 				pen.setColor( QColor( 0x4f, 0xb3, 0xff ) );
// 				pen.setColor( Qt::black );
				pen.setColor( functionColor );
				p.setPen( pen );
				p.drawEllipse( QRectF( -QPointF( 1/k, 1/k ), QSizeF( 2/k, 2/k ) ) );
				
				// draw normal
// 				pen.setColor( QColor( 0xff, 0x37, 0x61 ) );
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
				p.drawLine( QLineF( -1/k, -1/k * tangent_scale, -1/k, 1/k * tangent_scale ) );
				
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
			else
			{
				// Curve is practically straight
			}
		}
		else
			pen.setColor(m_invertedBackgroundColor);
		
		p.setPen( pen );
		p.Lineh( area.left(), m_crosshairPixelCoords.y(), area.right() );
		p.Linev( m_crosshairPixelCoords.x(), area.bottom(), area.top());
	}
	
// 	if ( m_prevCursor == CursorMagnify )
// 		p.drawPixmap( m_crosshairPixelCoords.toPoint() - QPoint( 11, 11 ), m_magnifyPixmap );

	p.end();
}


double View::pixelNormal( const Plot & plot, double x, double y )
{
	Function * f = plot.function();
	assert( f );
	
	plot.updateFunction();
	
	// For converting from real to pixels
	double sx = area.width() / (m_xmax - m_xmin);
	double sy = area.height() / (m_ymax - m_ymin);
	
	double dx = 0;
	double dy = 0;
	
	double h = this->h( plot );
	
	int d0 = plot.derivativeNumber();
	int d1 = d0+1;
	
	switch ( f->type() )
	{
		case Function::Cartesian:
		{
			double df = XParser::self()->derivative( d1, f->eq[0], x, h );
			return -arctan( df * (sy/sx) ) - (M_PI/2);
		}
		
		case Function::Implicit:
		{
			dx = XParser::self()->partialDerivative( d1, d0, f->eq[0], x, y, h, h ) / sx;
			dy = XParser::self()->partialDerivative( d0, d1, f->eq[0], x, y, h, h ) / sy;
			
			double theta = -arctan( dy / dx );
			
			if ( dx < 0 )
				theta += M_PI;
			
			theta += M_PI;
			
			return theta;
		}
		
		case Function::Polar:
		{
			double r =  XParser::self()->derivative( d0, f->eq[0], x, h );
			double dr = XParser::self()->derivative( d1, f->eq[0], x, h );
			
			dx = (dr * cos(x) - r * sin(x)) * sx;
			dy = (dr * sin(x) + r * cos(x)) * sy;
			break;
		}
		
		case Function::Parametric:
		{
			dx = XParser::self()->derivative( d1, f->eq[0], x, h ) * sx;
			dy = XParser::self()->derivative( d1, f->eq[1], x, h ) * sy;
			break;
		}
	}
			
	double theta = - arctan( dy / dx ) - (M_PI/2);
			
	if ( dx < 0 )
		theta += M_PI;
			
	return theta;
}


double View::pixelCurvature( const Plot & plot, double x, double y )
{
	Function * f = plot.function();
	
	// For converting from real to pixels
	double sx = area.width() / (m_xmax - m_xmin);
	double sy = area.height() / (m_ymax - m_ymin);
	
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
		case Function::Cartesian:
		{
			fdx = sx;
			fddx = 0;
			
			fdy = XParser::self()->derivative( d1, f->eq[0], x, h ) * sy;
			fddy = XParser::self()->derivative( d2, f->eq[0], x, h) * sy;
			
// 			kDebug() << k_funcinfo << "fdy="<<fdy<<" fddy="<<fddy<<endl;
			
			break;
		}
		
		case Function::Polar:
		{
			double r = XParser::self()->derivative( d0, f->eq[0], x, h );
			double dr = XParser::self()->derivative( d1, f->eq[0], x, h );
			double ddr = XParser::self()->derivative( d2, f->eq[0], x, h );
			
			fdx = (dr * cos(x) - r * sin(x)) * sx;
			fdy = (dr * sin(x) + r * cos(x)) * sy;
			
			fddx = (ddr * cos(x) - 2 * dr * sin(x) - r * cos(x)) * sx;
			fddy = (ddr * sin(x) + 2 * dr * cos(x) - r * sin(x)) * sy;
			
			break;
		}
		
		case Function::Parametric:
		{
			fdx = XParser::self()->derivative( d1, f->eq[0], x, h ) * sx;
			fdy = XParser::self()->derivative( d1, f->eq[1], x, h ) * sy;
			
			fddx = XParser::self()->derivative( d2, f->eq[0], x, h ) * sx;
			fddy = XParser::self()->derivative( d2, f->eq[1], x, h ) * sy;
			
			break;
		}
		
		case Function::Implicit:
		{
			fdx =  XParser::self()->partialDerivative( d1, d0, f->eq[0], x, y, h, h ) / sx;
			fdy =  XParser::self()->partialDerivative( d0, d1, f->eq[0], x, y, h, h ) / sy;
			
			fddx = XParser::self()->partialDerivative( d2, d0, f->eq[0], x, y, h, h ) / (sx*sx);
			fddy = XParser::self()->partialDerivative( d0, d2, f->eq[0], x, y, h, h ) / (sy*sy);
			
			fdxy = XParser::self()->partialDerivative( d1, d1, f->eq[0], x, y, h, h ) / (sx*sy);
			
			
			break;
		}
	}
	
	double mod = pow( fdx*fdx + fdy*fdy, 1.5 );
	
	switch ( f->type() )
	{
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
			
			m_showFunctionExtrema->setChecked( function->plotAppearance( m_currentPlot.plotMode ).showExtrema );
			
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
		QPointF ptd( toPixel( closestPoint ) );
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

	double best_distance = 1e30; // a nice large number
	QPointF best_cspos;

	foreach ( Function * function, XParser::self()->m_ufkt )
	{
		const QList< Plot > plots = function->allPlots();
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
	plot.updateFunction();

	double best_x = 0.0;

	Function * function = plot.function();
	assert( function->type() != Function::Implicit ); // should use findRoot (3D version) for this

	if ( function->type() == Function::Cartesian )
	{
		double best_pixel_x = 0.0;

		QPointF pixelPos = toPixel( pos, ClipInfinite );

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
	}
	else
	{
		// Either polar or parametric
		
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


double View::pixelDistance( const QPointF & pos, const Plot & plot, double x, bool updateFunction )
{
	QPointF f = realValue( plot, x, updateFunction );
	QPointF df = toPixel( pos, ClipInfinite ) - toPixel( f, ClipInfinite );

	return std::sqrt( df.x()*df.x() + df.y()*df.y() );
}


QString View::posToString( double x, double delta, PositionFormatting format, QColor color  ) const
{
// 	assert( delta != 0.0 );
	if ( delta == 0 )
		delta = 1;

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
	m_crosshairPosition = toReal( ptl );

	m_currentPlot.updateFunction();
	Function * it = m_currentPlot.function();

	if ( it && crosshairPositionValid( it ) && (m_popupmenushown != 1) )
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
			ptl.setY(yToPixel( m_crosshairPosition.y() ));

			if ( m_crosshairPosition.y()<m_ymin || m_crosshairPosition.y()>m_ymax) //the ypoint is not visible
			{
				out_of_bounds = true;
			}
			else if ( (fabs(yToReal(ptl.y())) < (m_ymax-m_ymin)/80) && (it->type() == Function::Cartesian) )
			{
				double x0 = m_crosshairPosition.x();
				if ( !m_haveRoot && findRoot( &x0, m_currentPlot, PreciseRoot ) )
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
		if ( (it->type() != Function::Cartesian) || (it->type() != Function::Differential) || area.contains( mousePos ) )
		{
			ptl = toPixel( m_crosshairPosition );
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
	double realx = xToReal((mousePos * wm.inverted()).x());
	double realy = yToReal((mousePos * wm.inverted()).y());

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
	double real1x = xToReal(p.x() );
	double real1y = yToReal(p.y() );
	p = rect.bottomRight();
	double real2x = xToReal(p.x() );
	double real2y = yToReal(p.y() );

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
	double _real1x = xToReal(p.x() );
	double _real1y = yToReal(p.y() );
	p = rect.bottomRight();
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

	Settings::setXRange(4); //custom x-range
	Settings::setYRange(4); //custom y-range

	setScaling();

	drawPlot(); //update all graphs

	m_zoomMode = Normal;
}


void View::translateView( int dx, int dy )
{
	double rdx = xToReal( dx / m_scaler ) - xToReal( 0.0 );
	double rdy = yToReal( dy / m_scaler ) - yToReal( 0.0 );

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
		break;
	}
}

void View::setScaling()
{
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };

	assert( (Settings::xScaling() >= 0) && (Settings::xScaling() < 9) );
	assert( (Settings::yScaling() >= 0) && (Settings::yScaling() < 9) );

	if( Settings::xScaling() == 8) //automatic x-scaling
    {
		ticSepX = double(m_xmax-m_xmin)/16;
        ticSepXstr = units[ Settings::xScaling() ];
    }
	else
	{
		ticSepXstr = units[ Settings::xScaling() ];
		ticSepX = XParser::self()->eval( ticSepXstr );
	}

	if( Settings::yScaling() == 8)  //automatic y-scaling
    {
		ticSepY = double(m_ymax-m_ymin)/16;
        ticSepYstr = units[ Settings::yScaling() ];
    }
	else
	{
		ticSepYstr = units[ Settings::yScaling() ];
		ticSepY = XParser::self()->eval( ticSepYstr );
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
	assert( (ufkt->type() == Function::Cartesian) || (ufkt->type() == Function::Differential) );

	double x = 0;
	double y = 0;
	double result_x = 0;
	double result_y = 0;
	bool start = true;

	double dx = (dmax-dmin)/area.width();
	if ( plot.plotMode == Function::Integral || ufkt->type() == Function::Differential )
	{
		double max_dx = ufkt->eq[0]->differentialStates.step().value();
		if ( max_dx < dx )
			dx = max_dx;
	}

	x=dmin;

	plot.updateFunction();

	while ( (x>=dmin && x<=dmax) )
	{
		y = value( plot, 0, x, false );

		if ( !isnan(x) && !isnan(y) )
		{
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

	double dx = (s.dmax-s.dmin)/area.width();
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


void View::animateFunction()
{
	Function * f = m_currentPlot.function();
	if ( !f )
		return;
	
	ParameterAnimator * anim = new ParameterAnimator( this, f );
	anim->show();
}


void View::showExtrema( bool show )
{
	Function * f = m_currentPlot.function();
	if ( !f )
		return;
	
	f->plotAppearance( m_currentPlot.plotMode ).showExtrema = show;
	drawPlot();
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
	
	if ( m_popupmenushown > 0 )
		return false;

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
        QDBusReply<void> reply = QDBusInterface( QDBus::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.Kmplot" ).call( QDBus::NoBlock, "setStatusBarText", text, id );
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

#include "view.moc"
