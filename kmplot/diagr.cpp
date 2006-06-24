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

//local includes
#include "diagr.h"
#include "settings.h"
#include "View.h"

#include <kdebug.h>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>

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


//BEGIN class CDiagr
CDiagr * CDiagr::m_self = 0;

CDiagr * CDiagr::self()
{
	if ( !m_self )
		m_self = new CDiagr;
	
	return m_self;
}


CDiagr::CDiagr()
{
	m_textEdit = new QTextEdit;
	m_textEdit->setWordWrapMode( QTextOption::NoWrap );
	m_textEdit->setLineWrapMode( QTextEdit::NoWrap );
	
	m_textDocument = m_textEdit->document();
	
	updateSettings();
	ex=ey=1.;
}


CDiagr::~CDiagr()
{
	m_textEdit->deleteLater();
}

void CDiagr::Create( QPointF Ref, 			    // Bezugspunkt links unten
                    double lx, double ly, 			// Achsenl�gen
                    double xmin, double xmax,   // x-Wertebereich
                    double ymin, double ymax) 	// y-Wertebereich
{	int x, y, h, w;

	CDiagr::xmin=xmin;                  // globale Variablen setzen
	CDiagr::xmax=xmax;
	CDiagr::ymin=ymin;
	CDiagr::ymax=ymax;
	xmd=xmax+1e-6;
	ymd=ymax+1e-6;
	tsx=ceil(xmin/ex)*ex;
	tsy=ceil(ymin/ey)*ey;
	skx=lx/(xmax-xmin);			        // Skalierungsfaktoren berechnen
	sky=ly/(ymax-ymin);
	ox=Ref.x()-skx*xmin+0.5;	        // Ursprungskoordinaten berechnen
	oy=Ref.y()+sky*ymax+0.5;
	m_plotArea.setRect(x=int(Ref.x()), y=int(Ref.y()), w=int(lx), h=int(ly));
	if( Settings::showExtraFrame() )
	{
		x-=20;
		y-=20;
		w+=40;
		h+=40;

		if( Settings::showLabel() && ymin>=0. )
				h+=60;
	}

	m_frame.setRect(x, y, w, h);
}


void CDiagr::Skal( double ex, double ey )
{
	CDiagr::ex=ex;
	CDiagr::ey=ey;
	tsx=ceil(xmin/ex)*ex;
	tsy=ceil(ymin/ey)*ey;
}


void CDiagr::Plot(QPainter* pDC)
{
	QPen pen( frameColor, View::self()->mmToPenWidth( borderThickness, true ) );

	if( m_gridMode != GridNone )
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


QPointF CDiagr::toPixel( const QPointF & real, ClipBehaviour clipBehaviour )
{
	double x = xToPixel( real.x(), clipBehaviour );
	double y = yToPixel( real.y(), clipBehaviour );
	return QPointF( x, y );
}


double CDiagr::xToPixel( double x, ClipBehaviour clipBehaviour )		// reale x-Koordinate
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
	else if ( (x<xmin) && (clipBehaviour == ClipAll) )
	{
		xclipflg=1;
		xi=m_plotArea.left();
	}
	else if ( (x>xmax) && (clipBehaviour == ClipAll) )
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


double CDiagr::yToPixel( double y, ClipBehaviour clipBehaviour )		// reale y-Koordinate
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
	else if ( (y<ymin) && (clipBehaviour == ClipAll) )
	{
		yclipflg=1;
		yi=m_plotArea.bottom();
	}
	else if ( (y>ymax) && (clipBehaviour == ClipAll) )
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


QPointF CDiagr::toReal( const QPointF & pixel )
{
	double x = xToReal( pixel.x() );
	double y = yToReal( pixel.y() );
	return QPointF( x, y );
}


double CDiagr::xToReal(double x) 		// Bildschirmkoordinate
{   return (x-ox)/skx;     			// reale x-Koordinate
}


double CDiagr::yToReal(double y)        // Bildschirmkoordinate
{   return (oy-y)/sky;     			// reale y-Koordinate
}


void CDiagr::drawAxes( QPainter* pDC )	// draw axes
{
	double a, b, tl;
	double d, da, db;
	
	if( Settings::showAxes() )
	{
		pDC->save();
		
		int const dx=14;
		int const dy=8;
		
		pDC->setPen( QPen( axesColor, View::self()->mmToPenWidth(axesLineWidth, true) ) );
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

	pDC->setPen( QPen( axesColor, View::self()->mmToPenWidth(ticWidth, true) ) );
	if( Settings::showAxes() )
	{
		da=oy-(ticLength*10.0);
		db=oy+(ticLength*10.0);
		tl= Settings::showFrame()? 0: (ticLength*10.0);
		d=tsx;
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

		while(d<xmd-ex/2.)
		{
			pDC->Linev(xToPixel(d), a, b);
			d+=ex;
		}

		da=ox-(10.0*ticLength);
		db=ox+(10.0*ticLength);
		d=tsy;
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

		while(d<ymd-ey/2.)
		{
			pDC->Lineh(a, yToPixel(d), b);
			d+=ey;
		}
	}
	else if( Settings::showFrame() )
	{
		a=m_plotArea.bottom()+(ticLength*10.0);
		b=m_plotArea.top()-(ticLength*10.0);
		d=tsx;
		while(d<xmd)
		{
			pDC->Linev(xToPixel(d), m_plotArea.bottom(), a);
			pDC->Linev(xToPixel(d), m_plotArea.top(), b);
			d+=ex;
		}

		a=m_plotArea.left()+(ticLength*10.0);
		b=m_plotArea.right()-(ticLength*10.0);
		d=tsy;
		while(d<ymd)
		{
			pDC->Lineh(m_plotArea.left(), yToPixel(d), a);
			pDC->Lineh(m_plotArea.right(), yToPixel(d), b);
			d+=ey;
		}
	}
}


void CDiagr::drawGrid( QPainter* pDC )
{
	double a, b;
	double d, x, y;
	QPen pen( gridColor, View::self()->mmToPenWidth(gridLineWidth, true) );

	pDC->setPen(pen);
	
	switch ( m_gridMode )
	{
		case GridNone:
			break;
			
		case GridLines:
		{
			d=tsx;
			while(d<xmd)
			{
				pDC->Linev(xToPixel(d), m_plotArea.bottom(), m_plotArea.top());
				d+=ex;
			}
			d=tsy;
			while(d<ymd)
			{
				pDC->Lineh(m_plotArea.left(), yToPixel(d), m_plotArea.right());
				d+=ey;
			}
			
			break;
		}
		
		case GridCrosses:
		{
			int const dx = 5;
			int const dy = 5;

			for(x=tsx; x<xmd; x+=ex)
			{
				a=xToPixel(x);
				for(y=tsy; y<ymd; y+=ey)
				{
					b=yToPixel(y);
					pDC->Lineh(a-dx, b, a+dx);
					pDC->Linev(a, b-dy, b+dy);
				}
			}
		}
		
		case GridPolar:
		{
			double y2;
			double w;
			QRect const rc=m_plotArea;
		
			pDC->setClipRect(rc);
			double const c=hypot(xmd*skx, ymd*sky);
			double const xm=(c+ox);
			double const dr=(skx*ex);
			double const d2r=(2.*skx*ex);
			double x1=ox-dr;
			double y1=oy-dr;
			double x2=y2=d2r;

			do
			{
				pDC->drawEllipse( QRectF( x1, y1, x2, y2 ) );
				x1-=dr;
				y1-=dr;
				x2+=d2r;
				y2+=d2r;
			}
			while(x2<=xm);

			x1=ox;
			y1=oy;
			for(w=0.; w<2.*M_PI; w+=M_PI/12.)
			{
				x2=(ox+c*cos(w));
				y2=(oy+c*sin(w));
				pDC->Line( QPointF( x1, y1 ), QPointF( x2, y2 ) );
			}
			pDC->setClipping( false );
			
			break;
		}
	}
}


void CDiagr::drawLabels(QPainter* pDC)
{
	int const dx=15;
	int const dy=40;
	QFont const font=QFont( Settings::axesFont(), Settings::axesFontSize() );
	pDC->setFont(font);
	m_textDocument->setDefaultFont( font );
	
	double const x=xToPixel(0.);
	double const y=yToPixel(0.);
	double d;
	int n;

	//pDC->drawText(x-dx, y+dy, 0, 0, Qt::AlignRight|Qt::AlignVCenter|Qt::TextDontClip, "0");
	char draw_next=0;
	QFontMetrics const test(font);
	int swidth=0;

// 	kDebug() << "tsx="<<tsx<<" xmd="<<xmd<<" ex="<<ex << " (xmd-tsx)/ex="<<(xmd-tsx)/ex<<endl;
	for(d=tsx, n=(int)ceil(xmin/ex); d<xmd; d+=ex, ++n)
	{
		if(n==0 || fabs(d-xmd)<=1.5*ex)
			continue;
		
		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ex-M_PI/frac[i])> 1e-3 )
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
			s = View::self()->posToString( n*ex, (xmax-xmin)/4, View::ScientificFormat, axesColor );
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
	
	if ( ymax<0 && xmax<0 )
		drawRect = QRectF( xToPixel(xmax)-(4*dx), y+(dy-20), 0, 0 );
	else
		drawRect = QRectF( xToPixel(xmax)-dx, y+dy, 0, 0 );
	pDC->drawText( drawRect, Qt::AlignCenter|Qt::TextDontClip, "x" );

	for(d=tsy, n=(int)ceil(ymin/ey); d<ymd; d+=ey, ++n)
	{
		if(n==0 || fabs(d-ymd)<=1.5*ey)
			continue;

		QString s;
		
		int frac[] = { 2, 3, 4 };
		bool found = false;
		for ( unsigned i = 0; i < 3; ++i )
		{
			if( fabs(ey-M_PI/frac[i])> 1e-3 )
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
			s = View::self()->posToString( n*ey, (ymax-ymin)/4, View::ScientificFormat, axesColor );
		}
		
		if ( !s.isEmpty() )
		{
			m_textDocument->setHtml( s );
			
			QRectF br = m_textDocument->documentLayout()->frameBoundingRect( m_textDocument->rootFrame() );
			
			QPointF drawPoint( 0, yToPixel(d)-(br.height()/2) );
			
			if (xmin>=0)
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

	
	if(ymax<0 && xmax<0)
		drawRect = QRectF( x-dx, yToPixel(ymax)+(2*dy), 0, 0 );
	else if (xmin>0)
		drawRect = QRectF( x-(2*dx), yToPixel(ymax)+dy, 0, 0 );
	else
		drawRect = QRectF( x-dx, yToPixel(ymax)+dy, 0, 0 );
	pDC->drawText( drawRect, Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, "y" );
}


void CDiagr::updateSettings( )
{
	frameColor=qRgb(0, 0, 0);
	borderThickness = 0.2;
	
	m_gridMode = (GridStyle)Settings::gridStyle();
	axesLineWidth = Settings::axesLineWidth();
	gridLineWidth = Settings::gridLineWidth();
	ticWidth = Settings::ticWidth();
	ticLength = Settings::ticLength();
	
	axesColor = Settings::axesColor();
	if ( !axesColor.isValid() )
		axesColor = Qt::black;
	
	gridColor=Settings::gridColor();
	if ( !gridColor.isValid() )
		gridColor = QColor( 0xc0, 0xc0, 0xc0 );
}
//END class CDiagr
