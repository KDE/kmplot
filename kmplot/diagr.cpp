/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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

//local includes
#include "diagr.h"
#include "settings.h"
#include "View.h"

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

#include <kdebug.h>

CDiagr::CDiagr()
{
	updateSettings();
	ex=ey=1.;
}


CDiagr::~CDiagr()
{}

void CDiagr::Create(QPoint Ref, 			    // Bezugspunkt links unten
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
	PlotArea.setRect(x=Ref.x(), y=Ref.y(), w=int(lx), h=int(ly));
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
	QPen pen(QColor(frameColor), View::self()->mmToPenWidth( borderThickness, true ) );

	if( g_mode != GRID_NONE )
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


double CDiagr::TransxToPixel( double x )		// reale x-Koordinate
{
	double xi;                			// transformierte x-Koordinate
	static double lastx;            // vorherige x-Koordinate
	if(isnan(x))
	{
		xclipflg=1;
		if(lastx<1. && lastx>-1.)
			xi=(ox-skx*lastx);
		else
			xi=(lastx<0)? PlotArea.left(): PlotArea.right();
	}
	else if(isinf(x)==-1)
	{
		xclipflg=0;
		xi=PlotArea.left();
	}
	else if(isinf(x)==1)
	{
		xclipflg=0;
		xi=PlotArea.right();
                
	}
	else if(x<xmin)
	{
		xclipflg=1;
		xi=PlotArea.left();
	}
	else if(x>xmax)
	{
		xclipflg=1;
		xi=PlotArea.right();
	}
	else
	{
		xclipflg=0;
		xi=(ox+skx*x);
	}

	lastx=x;
	return xi;
}


double CDiagr::TransyToPixel(double y)		// reale y-Koordinate
{   
	double yi;                     	// transformierte y-Koordinate
	static double lasty;            // vorherige y-Koordinate
	if(isnan(y))
	{
		yclipflg=1;
		if(lasty<1. && lasty>-1.)
			yi=(oy-sky*lasty);
		else
			yi=(lasty<0)? PlotArea.bottom(): PlotArea.top();
	}
	else if(isinf(y)==-1)
	{
		yclipflg=0;
		yi=PlotArea.bottom();
                
	}
	else if(isinf(y)==1)
	{
		yclipflg=0;
		yi=PlotArea.top();
                
	}
	else if(y<ymin)
	{
		yclipflg=1;
		yi=PlotArea.bottom();
	}
	else if(y>ymax)
	{
		yclipflg=1;
		yi=PlotArea.top();
	}
	else
	{
		yclipflg=0;
		yi=(oy-sky*y);
	}

	lasty=y;
	return yi;
}


double CDiagr::TransxToReal(double x) 		// Bildschirmkoordinate
{   return (x-ox)/skx;     			// reale x-Koordinate
}


double CDiagr::TransyToReal(double y)        // Bildschirmkoordinate
{   return (oy-y)/sky;     			// reale y-Koordinate
}


void CDiagr::drawAxes( QPainter* pDC )	// draw axes
{
	double a, b, tl;
	double d, da, db;
	
	if( Settings::showAxes() )
	{
		pDC->setPen( QPen( QColor(axesColor), View::self()->mmToPenWidth(axesLineWidth, true) ) );
		a=PlotArea.right();
		b=TransyToPixel(0.);
		pDC->Lineh(PlotArea.left(), b, a);	    // x-Achse
		if( Settings::showArrows()) 		    			// ARROWS
		{	int const dx=40;
			int const dy=15;
			pDC->Line( QPointF( a, b ), QPointF( a-dx, b+dy) );
			pDC->Line( QPointF( a, b ), QPointF( a-dx, b-dy) );
		}

		a=TransxToPixel(0.);
		b=PlotArea.top();
		pDC->Linev(a, PlotArea.bottom(), b); 	    // y-Achse
		if( Settings::showArrows() )   					// ARROWS
		{	int const dx=15;
			int const dy=40;
			pDC->Line( QPointF( a, b ), QPointF( a-dx, b+dy) );
			pDC->Line( QPointF( a, b ), QPointF( a+dx, b+dy) );
		}
	}

	pDC->setPen( QPen( QColor(axesColor), View::self()->mmToPenWidth(ticWidth, true) ) );
	if( Settings::showAxes() )
	{
		da=oy-(ticLength*10.0);
		db=oy+(ticLength*10.0);
		tl= Settings::showFrame()? 0: (ticLength*10.0);
		d=tsx;
		if(da<(double)PlotArea.top())
		{
			a=PlotArea.top()-tl;
			b=PlotArea.top()+int(10.0*ticLength);
		}
		else if(db>(double)PlotArea.bottom())
		{
			b=PlotArea.bottom()+tl;
			a=PlotArea.bottom()-(10.0*ticLength);
		}
		else
		{
			a=da;
			b=db;
		}

		while(d<xmd-ex/2.)
		{
			pDC->Linev(TransxToPixel(d), a, b);
			d+=ex;
		}

		da=ox-(10.0*ticLength);
		db=ox+(10.0*ticLength);
		d=tsy;
		if(da<(double)PlotArea.left())
		{
			a=PlotArea.left()-tl;
			b=PlotArea.left()+(10.0*ticLength);
		}
		else if(db>(double)PlotArea.right())
		{
			b=PlotArea.right()+tl;
			a=PlotArea.right()-(10.0*ticLength);
		}
		else
		{
			a=da;
			b=db;
		}

		while(d<ymd-ey/2.)
		{
			pDC->Lineh(a, TransyToPixel(d), b);
			d+=ey;
		}
	}
	else if( Settings::showFrame() )
	{
		a=PlotArea.bottom()+(ticLength*10.0);
		b=PlotArea.top()-(ticLength*10.0);
		d=tsx;
		while(d<xmd)
		{
			pDC->Linev(TransxToPixel(d), PlotArea.bottom(), a);
			pDC->Linev(TransxToPixel(d), PlotArea.top(), b);
			d+=ex;
		}

		a=PlotArea.left()+(ticLength*10.0);
		b=PlotArea.right()-(ticLength*10.0);
		d=tsy;
		while(d<ymd)
		{
			pDC->Lineh(PlotArea.left(), TransyToPixel(d), a);
			pDC->Lineh(PlotArea.right(), TransyToPixel(d), b);
			d+=ey;
		}
	}
}


void CDiagr::drawGrid( QPainter* pDC )
{
	double a, b;
	double d, x, y;
	QPen pen( QColor(gridColor), View::self()->mmToPenWidth(gridLineWidth, true) );

	pDC->setPen(pen);
	if( g_mode==GRID_LINES )
	{
		d=tsx;
		while(d<xmd)
		{
			pDC->Linev(TransxToPixel(d), PlotArea.bottom(), PlotArea.top());
			d+=ex;
		}
		d=tsy;
		while(d<ymd)
		{
			pDC->Lineh(PlotArea.left(), TransyToPixel(d), PlotArea.right());
			d+=ey;
		}
	}
	else if( g_mode==GRID_CROSSES )
	{
		int const dx = 5;
		int const dy = 5;

		for(x=tsx; x<xmd; x+=ex)
		{
			a=TransxToPixel(x);
			for(y=tsy; y<ymd; y+=ey)
			{
				b=TransyToPixel(y);
				pDC->Lineh(a-dx, b, a+dx);
				pDC->Linev(a, b-dy, b+dy);
			}
		}
	}
	else if( g_mode==GRID_POLAR )
	{
		double y2;
		double w;
		QRect const rc=PlotArea;
		
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
		pDC->setClipping(FALSE);
	}
}


void CDiagr::drawLabels(QPainter* pDC)
{
	int const dx=15;
	int const dy=40;
	QFont const font=QFont( Settings::axesFont(), Settings::axesFontSize() );
	pDC->setFont(font);
	double const x=TransxToPixel(0.);
	double const y=TransyToPixel(0.);
	double d;
	int n;
	QString s;

	//pDC->drawText(x-dx, y+dy, 0, 0, Qt::AlignRight|Qt::AlignVCenter|Qt::TextDontClip, "0");
	char draw_next=0;
	QFontMetrics const test(font);
	int swidth=0;

	for(d=tsx, n=(int)ceil(xmin/ex); d<xmd; d+=ex, ++n)
	{
		if(n==0 || fabs(d-xmd)<=1.5*ex)
			continue;


		if(n<0)
			s="-";
		else
			s="+";
		if(fabs(ex-M_PI/2.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/2");
			else if(n%2 == 0)
			{
				if(n==-2 || n==2)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/2);
					s+=QChar(960);
				}
			}
		}
		else if(fabs(ex-M_PI/3.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/3");
			else if(n%3==0)
			{
				if(n==-3 || n==3)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/3);
					s+=QChar(960);
				}
			}
		}
		else if(fabs(ex-M_PI/4.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/4");
			else if(n%4==0)
			{
				if(n==-4 || n==4)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/4);
					s+=QChar(960);
				}
			}
		}
		else if((n%5==0 || n==1 || n==-1 || draw_next))
		{
			s=QString().sprintf("%+0.3g", n*ex);
		}
		if ( (s != "-") && (s != "+") )
		{
			swidth = test.width(s);
			if (  TransxToPixel(d)-x<swidth && TransxToPixel(d)-x>-swidth && draw_next==0)
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
			pDC->drawText( QRectF( TransxToPixel(d), y+dy, 0, 0 ), Qt::AlignCenter|Qt::TextDontClip, s);
		}
	}

	if(ymax<0 && xmax<0)
		pDC->drawText( QRectF( TransxToPixel(xmax)-(4*dx), y+(dy-20), 0, 0 ), Qt::AlignCenter|Qt::TextDontClip, "x");
	else
		pDC->drawText( QRectF( TransxToPixel(xmax)-dx, y+dy, 0, 0 ), Qt::AlignCenter|Qt::TextDontClip, "x");

	for(d=tsy, n=(int)ceil(ymin/ey); d<ymd; d+=ey, ++n)
	{
		if(n==0 || fabs(d-ymd)<=1.5*ey)
			continue;

		if(n<0)
			s="-";
		else
			s="+";

		if(fabs(ey-M_PI/2.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/2");
			else if(n%2==0)
			{
				if(n==-2 || n==2)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/2);
					s+=QChar(960);
				}
			}
		}
		else if(fabs(ey-M_PI/3.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/3");
			else if(n%3==0)
			{
				if(n==-3 || n==3)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/3);
					s+=QChar(960);
				}
			}
		}
		else if(fabs(ey-M_PI/4.)<1e-3)
		{
			if(n==-1 || n==1)
				s+=QChar(960)+QString("/4");
			else if(n%4==0)
			{
				if(n==-4 || n==4)
					s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/4);
					s+=QChar(960);
				}
			}
		}
		else if((n%5==0 || n==1 || n==-1))
		{
			s=QString().sprintf("%+0.3g", n*ey);
		}
		if ( (s != "-") && (s != "+") )
		{
			if (xmin>=0)
			{
				QRectF drawRect( x+dx, TransyToPixel(d), 0, 0 );
				pDC->drawText( drawRect, Qt::AlignVCenter|Qt::AlignLeft|Qt::TextDontClip, s);
			}
			else
			{
				QRectF drawRect( x-dx, TransyToPixel(d), 0, 0 );
				QRectF br = pDC->boundingRect( drawRect, Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, s);
				if ( br.left() < 0 )
				{
					// have to adjust drawRect so that we don't draw off the edge of the view
					drawRect.translate( -br.left(), 0 );
				}
				
				pDC->drawText( drawRect, Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, s);
			}
		}
	}

	if(ymax<0 && xmax<0)
		pDC->drawText( QRectF( x-dx, TransyToPixel(ymax)+(2*dy), 0, 0 ), Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, "y");
	else if (xmin>0)
		pDC->drawText( QRectF( x-(2*dx), TransyToPixel(ymax)+dy, 0, 0 ), Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, "y");
	else
		pDC->drawText( QRectF( x-dx, TransyToPixel(ymax)+dy, 0, 0 ), Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip, "y");
}


void CDiagr::updateSettings( )
{
	frameColor=qRgb(0, 0, 0);
	borderThickness = 0.2;
	
	g_mode = Settings::gridStyle();
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
