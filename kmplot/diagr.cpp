/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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

//local includes
#include "diagr.h"
#include "settings.h"

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
	frameColor=qRgb(0, 0, 0);
	axesColor=qRgb(0, 0, 0);
	gridColor=qRgb(192, 192, 192);
	borderThickness=2;
	axesLineWidth = Settings::axesLineWidth();
	gridLineWidth = Settings::gridLineWidth();
	ticWidth = Settings::ticWidth();
	ticLength = Settings::ticLength();
	g_mode = Settings::gridStyle();
	ex=ey=1.;
}


CDiagr::~CDiagr()
{}

void CDiagr::Create(QPoint Ref, 			    // Bezugspunkt links unten
                    int lx, int ly, 			// Achsenl�gen
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
	PlotArea.setRect(x=Ref.x(), y=Ref.y(), w=lx, h=ly);
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
	g_mode = Settings::gridStyle();
	tsx=ceil(xmin/ex)*ex;
	tsy=ceil(ymin/ey)*ey;
}


void CDiagr::Plot(QPainter* pDC)
{
	QPen pen(frameColor, borderThickness);

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


int CDiagr::Transx( double x )		// reale x-Koordinate
{
        int xi;                			// transformierte x-Koordinate
	static double lastx;            // vorherige x-Koordinate
	if(isnan(x))
	{
		xclipflg=1;
		if(lastx<1. && lastx>-1.)
			xi=(int)(ox-skx*lastx);
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
		xi=(int)(ox+skx*x);
	}

	lastx=x;
	return xi;
}


int CDiagr::Transy(double y)		// reale y-Koordinate
{   
        int yi;                     	// transformierte y-Koordinate
	static double lasty;            // vorherige y-Koordinate
	if(isnan(y))
	{
		yclipflg=1;
		if(lasty<1. && lasty>-1.)
			yi=(int)(oy-sky*lasty);
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
		yi=(int)(oy-sky*y);
	}

	lasty=y;
	return yi;
}


double CDiagr::Transx(int x) 		// Bildschirmkoordinate
{   return (x-ox)/skx;     			// reale x-Koordinate
}


double CDiagr::Transy(int y)        // Bildschirmkoordinate
{   return (oy-y)/sky;     			// reale y-Koordinate
}


void CDiagr::drawAxes( QPainter* pDC )	// draw axes
{	int a, b, tl;
	double d, da, db;

	if( Settings::showAxes() )
	{  
		pDC->setPen( QPen( axesColor, axesLineWidth ) );
                b = Transy(0.);
                a = PlotArea.right();
		pDC->Lineh(PlotArea.left(), b, a);	    // x-Achse
		if( Settings::showArrows()) 		    			// ARROWS
		{	int const dx=40;
			int const dy=15;
			pDC->Line(a, b, a-dx, b+dy);
			pDC->Line(a, b, a-dx, b-dy);
		}

	        a = Transx(0.);
		b = PlotArea.top();
		pDC->Linev(a, PlotArea.bottom(), b); 	    // y-Achse
		if( Settings::showArrows() )   					// ARROWS
		{	int const dx=15;
			int const dy=40;
			pDC->Line(a, b, a-dx, b+dy);
			pDC->Line(a, b, a+dx, b+dy);
		}
	}

	pDC->setPen( QPen( axesColor, ticWidth ) );
	if( Settings::showAxes() )
	{
		da=oy-ticLength;
		db=oy+ticLength;
		tl= Settings::showFrame()? 0: ticLength;
		d=tsx;
		if(da<(double)PlotArea.top())
		{
			a=PlotArea.top()-tl;
			b=PlotArea.top()+ticLength;
		}
		else if(db>(double)PlotArea.bottom())
		{
			b=PlotArea.bottom()+tl;
			a=PlotArea.bottom()-ticLength;
		}
		else
		{
			a=(int)da;
			b=(int)db;
		}

		while(d<xmd-ex/2.)
		{
			pDC->Linev(Transx(d), a, b);
			d+=ex;
		}

		da=ox-ticLength;
		db=ox+ticLength;
		d=tsy;
		if(da<(double)PlotArea.left())
		{
			a=PlotArea.left()-tl;
			b=PlotArea.left()+ticLength;
		}
		else if(db>(double)PlotArea.right())
		{
			b=PlotArea.right()+tl;
			a=PlotArea.right()-ticLength;
		}
		else
		{
			a=(int)da;
			b=(int)db;
		}

		while(d<ymd-ey/2.)
		{
			pDC->Lineh(a, Transy(d), b);
			d+=ey;
		}
	}
	else if( Settings::showFrame() )
	{
		a=PlotArea.bottom()+ticLength;
		b=PlotArea.top()-ticLength;
		d=tsx;
		while(d<xmd)
		{
			pDC->Linev(Transx(d), PlotArea.bottom(), a);
			pDC->Linev(Transx(d), PlotArea.top(), b);
			d+=ex;
		}

		a=PlotArea.left()+ticLength;
		b=PlotArea.right()-ticLength;
		d=tsy;
		while(d<ymd)
		{
			pDC->Lineh(PlotArea.left(), Transy(d), a);
			pDC->Lineh(PlotArea.right(), Transy(d), b);
			d+=ey;
		}
	}
}


void CDiagr::drawGrid( QPainter* pDC )
{
	int a, b;
	double d, x, y;
	QPen pen( gridColor, gridLineWidth );

	pDC->setPen(pen);
	if( g_mode==GRID_LINES )
	{
		d=tsx;
		while(d<xmd)
		{
			pDC->Linev(Transx(d), PlotArea.bottom(), PlotArea.top());
			d+=ex;
		}
		d=tsy;
		while(d<ymd)
		{
			pDC->Lineh(PlotArea.left(), Transy(d), PlotArea.right());
			d+=ey;
		}
	}
	else if( g_mode==GRID_CROSSES )
	{
		int const dx = 5;
		int const dy = 5;

		for(x=tsx; x<xmd; x+=ex)
		{
			a=Transx(x);
			for(y=tsy; y<ymd; y+=ey)
			{
				b=Transy(y);
				pDC->Lineh(a-dx, b, a+dx);
				pDC->Linev(a, b-dy, b+dy);
			}
		}
	}
	else if( g_mode==GRID_POLAR )
	{
		int y2;
		double w;
		QRect const rc=PlotArea;

		pDC->setClipRect(pDC->xForm(rc));
		double const c=hypot(xmd*skx, ymd*sky);
		int const xm=(int)(c+ox);
		int const dr=(int)(skx*ex);
		int const d2r=(int)(2.*skx*ex);
		int x1=(int)ox-dr;
		int y1=(int)oy-dr;
		int x2=y2=d2r;

		do
		{
			pDC->drawEllipse(x1, y1, x2, y2);
			x1-=dr;
			y1-=dr;
			x2+=d2r;
			y2+=d2r;
		}
		while(x2<=xm);

		x1=(int)ox;
		y1=(int)oy;
		for(w=0.; w<2.*M_PI; w+=M_PI/12.)
		{
			x2=(int)(ox+c*cos(w));
			y2=(int)(oy+c*sin(w));
			pDC->Line(x1, y1, x2, y2);
		}
		pDC->setClipping(false);
	}
}


void CDiagr::drawLabels(QPainter* pDC)
{
	int const dx=15;
	int const dy=40;
	QFont const font=QFont( Settings::axesFont(), Settings::axesFontSize() );
	pDC->setFont(font);
	int const x=Transx(0.);
	int const y=Transy(0.);
	double d;
	int n;
	QString s;

	//pDC->drawText(x-dx, y+dy, 0, 0, Qt::AlignRight|Qt::AlignVCenter|Qt::DontClip, "0");
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
				s+="pi/2";//s+=QChar(960)+QString("/2");
			else if(n%2 == 0)
			{
				if(n==-2 || n==2)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/2);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			swidth = test.width(s);
			if (  Transx(d)-x<swidth && Transx(d)-x>-swidth && draw_next==0)
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
			pDC->drawText(Transx(d), y+dy, 0, 0, Qt::AlignCenter|Qt::DontClip, s);
		}
		else if(fabs(ex-M_PI/3.)<1e-3)
		{
			if(n==-1 || n==1)
				s+="pi/3";//s+=QChar(960)+QString("/3");
			else if(n%3==0)
			{
				if(n==-3 || n==3)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/3);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			swidth = test.width(s);
			if (  Transx(d)-x<swidth && Transx(d)-x>-swidth && draw_next==0)
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
			pDC->drawText(Transx(d), y+dy, 0, 0, Qt::AlignCenter|Qt::DontClip, s);
		}
		else if(fabs(ex-M_PI/4.)<1e-3)
		{
			if(n==-1 || n==1)
				s+="pi/4";//s+=QChar(960)+QString("/4");
			else if(n%4==0)
			{
				if(n==-4 || n==4)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/4);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			swidth = test.width(s);
			if (  Transx(d)-x<swidth && Transx(d)-x>-swidth && draw_next==0)
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
			pDC->drawText(Transx(d), y+dy, 0, 0, Qt::AlignCenter|Qt::DontClip, s);
		}
		else if((n%5==0 || n==1 || n==-1 || draw_next))
		{
			s=QString().sprintf("%+0.3g", n*ex);
			swidth = test.width(s);
			if (  Transx(d)-x<swidth && Transx(d)-x>-swidth && draw_next==0)
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
			pDC->drawText(Transx(d), y+dy, 0, 0, Qt::AlignCenter|Qt::DontClip, s);
		}
	}

	if(ymax<0 && xmax<0)
		pDC->drawText(Transx(xmax)-(4*dx), y+(dy-20), 0, 0, Qt::AlignCenter|Qt::DontClip, "x");
	else
		pDC->drawText(Transx(xmax)-dx, y+dy, 0, 0, Qt::AlignCenter|Qt::DontClip, "x");

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
				s+="pi/2";//s+=QChar(960)+QString("/2");
			else if(n%2==0)
			{
				if(n==-2 || n==2)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/2);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			if (xmin>=0)
				pDC->drawText(x+dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignLeft|Qt::DontClip, s);
			else
				pDC->drawText(x-dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, s);
		}
		else if(fabs(ey-M_PI/3.)<1e-3)
		{
			if(n==-1 || n==1)
				s+="pi/3";//s+=QChar(960)+QString("/3");
			else if(n%3==0)
			{
				if(n==-3 || n==3)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/3);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			if (xmin>=0)
				pDC->drawText(x+dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignLeft|Qt::DontClip, s);
			else
				pDC->drawText(x-dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, s);
		}
		else if(fabs(ey-M_PI/4.)<1e-3)
		{
			if(n==-1 || n==1)
				s+="pi/4";//s+=QChar(960)+QString("/4");
			else if(n%4==0)
			{
				if(n==-4 || n==4)
					s+="pi";//s+=QChar(960);
				else
				{
					s=QString().sprintf("%+d", n/4);
					s+="pi";//s+=QChar(960);
				}
			}
			else
				continue;
			if (xmin>=0)
				pDC->drawText(x+dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignLeft|Qt::DontClip, s);
			else
				pDC->drawText(x-dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, s);
		}
		else if((n%5==0 || n==1 || n==-1))
		{
			s=QString().sprintf("%+0.3g", n*ey);
			if (xmin>=0)
				pDC->drawText(x+dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignLeft|Qt::DontClip, s);
			else
				pDC->drawText(x-dx, Transy(d), 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, s);
		}
	}

	if(ymax<0 && xmax<0)
		pDC->drawText(x-dx, Transy(ymax)+(2*dy), 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, "y");
	else if (xmin>0)
		pDC->drawText(x-(2*dx), Transy(ymax)+dy, 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, "y");
	else
		pDC->drawText(x-dx, Transy(ymax)+dy, 0, 0, Qt::AlignVCenter|Qt::AlignRight|Qt::DontClip, "y");
}
