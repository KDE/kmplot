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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

// KDE includes
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "settings.h"
#include "View.h"
#include "View.moc"
#include "xparser.h"

View::View( QWidget* parent, const char* name ) : QWidget( parent, name )
{   
	m_parser = new XParser( 10, 200, 20 );
	init();
	csflg=0;
	csmode=-1;
	setBackgroundColor(QColor(255, 255, 255));
	setMouseTracking(TRUE);
}


View::~View()
{
	delete m_parser;
}

XParser* View::parser()
{
	return m_parser;
}

void View::draw(QPaintDevice *dev, int form)
{	int ix, lx, ly;
	float sf;
	QRect rc;
	QPainter DC;				// our painter
	DC.begin(dev);				// start painting widget
	rc=DC.viewport();
	w=rc.width();
	h=rc.height();
	
	setPlotRange();
	setScaling();
	
	if(form==0)										// screen
	{   ref=QPoint(120, 100);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC.scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
		if(DC.xForm(QPoint(lx+2*ref.x(), ly)).x() > DC.viewport().right())
		{	DC.resetXForm();
			DC.scale((float)w/(float)(lx+2*ref.x()), (float)w/(float)(lx+2*ref.x()));
		}	
		wm=DC.worldMatrix();
		s=DC.xForm(QPoint(1000, 0)).x()/1000.;
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
	}
	else if(form==1)								// printer
	{   
		sf=72./254.;								// 72dpi
		ref=QPoint(100, 100);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC.scale(sf, sf);
		s=1.;
		m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1"; 
		drawHeaderTable( &DC );
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
	}
	else if(form==2)								// svg
	{	ref=QPoint(0, 0);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.translate(-dgr.GetFrame().left(), -dgr.GetFrame().top());
		s=1.;
	}
	else if(form==3)								// bmp, png
	{	sf=180./254.;								// 180dpi
		ref=QPoint(0, 0);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.end();
		((QPixmap *)dev)->resize((int)(dgr.GetFrame().width()*sf), (int)(dgr.GetFrame().height()*sf));
		((QPixmap *)dev)->fill();
		DC.begin(dev);
		DC.translate(-dgr.GetFrame().left()*sf, -dgr.GetFrame().top()*sf);
		DC.scale(sf, sf);
		s=1.;
	}
	
	dgr.borderThickness=(uint)(4*s);
	dgr.axesLineWidth = (uint)( Settings::axesLineWidth()*s );
	dgr.gridLineWidth = (uint)( Settings::gridLineWidth()*s );
	dgr.ticWidth = (uint)( Settings::ticWidth()*s );
	dgr.ticLength = (uint)( Settings::ticLength() );
	dgr.axesColor = Settings::axesColor().rgb();
	dgr.gridColor=Settings::gridColor().rgb();
	dgr.Skal( tlgx, tlgy );
	
	dgr.Plot(&DC);
	PlotArea=dgr.GetPlotArea();
	area=DC.xForm(PlotArea);
	hline.resize(area.width(), 1);
	vline.resize(1, area.height());

	stepWidth=Settings::relativeStepWidth() * (xmax-xmin)/area.width();
	for(ix=0; ix<m_parser->ufanz; ++ix)
	{   
		if(m_parser->chkfix(ix)==-1) continue;

		plotfkt(ix, &DC);
	}

	csflg=0;
	DC.end();			// painting done
}


void View::plotfkt(int ix, QPainter *pDC)
{	
	char fktmode, p_mode;
	int iy, k, ke, mflg;
	double dx, x, y, dmin, dmax;
	QString fname, fstr;
	QPoint p1, p2;
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);

	if(ix==-1 || ix>=m_parser->ufanz) return ;	    // ungltiger Index
	//if(m_parser->fktext[ix].f_mode==0) return ;	// NOPLOT

	dx=stepWidth;
	
	pen.setWidth((int)(m_parser->fktext[ix].linewidth*s) );
	pen.setColor(m_parser->fktext[ix].color);

	fktmode=m_parser->fktext[ix].extstr[0].latin1();
	if(fktmode!='y')
	{   
		dmin=m_parser->fktext[ix].dmin;
		dmax=m_parser->fktext[ix].dmax;
	}

	if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
	{   
		if(fktmode=='r')
		{   dmin=0.;
			dmax=2*M_PI;
		}
		else
		{
			dmin=xmin;
			dmax=xmax;
			
		}
	}
	
	if(fktmode=='r') dx=Settings::relativeStepWidth()*0.05/(dmax-dmin);
	else if(fktmode=='x')
	{   
		m_parser->getfkt(ix, fname, fstr);
		fname[0]='y';
		iy=m_parser->getfix(fname);
		if(iy==-1) return ;
	}
	else if(fktmode=='y') return ;

	p_mode=0;
	pDC->setPen(pen);
	while(1)
	{   
		k=0;
		ke=m_parser->fktext[ix].k_anz;
		do
		{
			
			m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[k]);
			mflg=2;
			if ( p_mode != 0 ||  m_parser->fktext[ix].f_mode) // if the  
			for(x=dmin; x<dmax; x+=dx)
			{
				errno=0;

				switch(p_mode)
					{
					case 0: 
						y=m_parser->fkt(ix, x);
						break;
					
					case 1:
					{
						y=m_parser->a1fkt(ix, x);
						pen.setWidth((int)(m_parser->fktext[ix].f1_linewidth*s) );
						pen.setColor(m_parser->fktext[ix].f1_color);
						pDC->setPen(pen);
						break;
					}
					case 2:
					{
						y=m_parser->a2fkt(ix, x);
						pen.setWidth((int)(m_parser->fktext[ix].f2_linewidth*s) );
						pen.setColor(m_parser->fktext[ix].f2_color);
						pDC->setPen(pen);
						break;
					}
					case 3:
					{
						pen.setWidth((int)(m_parser->fktext[ix].anti_linewidth*s) );
						pen.setColor(m_parser->fktext[ix].anti_color);
						pDC->setPen(pen);
						if ( m_parser->fktext[ix].anti_use_precision )
							dx = (m_parser->fktext[ix].anti_precision)/1500;
					else
						dx=Settings::relativeStepWidth()/1500; //the stepwith must be small for Euler's metod and not depend on the size on the mainwindow
					m_parser->euler_method(x, y,ix);
						break;
						}
					}
					

				if(errno!=0) continue;
                if(fktmode=='r')
				{   
					p2.setX(dgr.Transx(y*cos(x)));
					p2.setY(dgr.Transy(y*sin(x)));
				}
				else if(fktmode=='x')
				{   
					p2.setX(dgr.Transx(y));
					p2.setY(dgr.Transy(m_parser->fkt(iy, x)));
				}
				else
				{   
					p2.setX(dgr.Transx(x));
					p2.setY(dgr.Transy(y));
				}
				
				if(dgr.xclipflg || dgr.yclipflg)
				{	
					if(mflg>=1) p1=p2;
					else
                    {   
		    	pDC->drawLine(p1, p2); p1=p2;
                        mflg=1;
                    }

				}
				else
				{	
					if(mflg>1)
						p1=p2;
					else
		    				pDC->drawLine(p1, p2); p1=p2;
                    			mflg=0;
				}
            }
		}
        while(++k<ke);
	
		if(m_parser->fktext[ix].f1_mode==1 && p_mode< 1) p_mode=1;
		else if(m_parser->fktext[ix].f2_mode==1 && p_mode< 2) p_mode=2;
		else if(m_parser->fktext[ix].anti_mode==1 && p_mode< 3)
		{
			p_mode=3;
			dmin = m_parser->fktext[ix].startx; //the initial x-point
		}
		else break;

		//do we need this?
		//pen=QPen(m_parser->fktext[ix].color, 50);
		//pDC->setPen(pen);
	}
}

void View::drawHeaderTable(QPainter *pDC)
{   int ix, ypos;
	QString alx, aly, atx, aty, dfx, dfy;
	
	if( m_printHeaderTable )
	{   pDC->translate(250., 150.);
		pDC->setPen(QPen(black, (int)(5.*s)));
		pDC->setFont(QFont( Settings::headerTableFont().family(), 30) );
		puts( Settings::headerTableFont().family().latin1() );
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
		
		pDC->drawText(0, 0, 300, 100, AlignCenter, i18n("Parameters:"));
		pDC->drawText(300, 0, 400, 100, AlignCenter, i18n("Plotting Area"));
		pDC->drawText(700, 0, 400, 100, AlignCenter, i18n("Axes Division"));
		pDC->drawText(1100, 0, 400, 100, AlignCenter, i18n("Printing Format"));
		pDC->drawText(0, 100, 300, 65, AlignCenter, i18n("x-Axis:"));
		pDC->drawText(0, 165, 300, 65, AlignCenter, i18n("y-Axis:"));
		pDC->drawText(300, 100, 400, 65, AlignCenter, alx);
		pDC->drawText(300, 165, 400, 65, AlignCenter, aly);
		pDC->drawText(700, 100, 400, 65, AlignCenter, atx);
		pDC->drawText(700, 165, 400, 65, AlignCenter, aty);
		pDC->drawText(1100, 100, 400, 65, AlignCenter, dfx);
		pDC->drawText(1100, 165, 400, 65, AlignCenter, dfy);

		pDC->drawText(0, 300, i18n("Functions:"));
		pDC->Lineh(0, 320, 700);
		for(ix=0, ypos=380; ix<m_parser->ufanz; ++ix)
		{  
			 if(m_parser->chkfix(ix)==-1) continue;

			pDC->drawText(100, ypos, m_parser->fktext[ix].extstr);
            ypos+=60;
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
{   int i;
	QChar c(960);

	while((i=s->find('p')) != -1) s->replace(i, 2, &c, 1);
}


bool View::root(double *x0)
{   double x, y, yn, dx;

    if(rootflg==1) return FALSE;
    
    x=csxpos;
    y=fabs(csypos);
    dx=0.1;
    
    while(1)
    {   if((yn=fabs(m_parser->fkt(csmode, x-dx))) < y) {x-=dx; y=yn;}
        else if((yn=fabs(m_parser->fkt(csmode, x+dx))) < y) {x+=dx; y=yn;}
        else dx/=10.;
        printf("x=%g,  dx=%g, y=%g\n", x, dx, y);
        if(y<1e-8) {*x0=x; return TRUE;}
        if(fabs(dx)<1e-8) return FALSE;
        if(x<xmin || x>xmax) return FALSE;
    }
}


// Slots

void View::paintEvent(QPaintEvent *)
{   draw(this, 0);
}


void View::mouseMoveEvent(QMouseEvent *e)
{   char sx[20], sy[20];

	if(csflg==1)        // Fadenkreuz l�chen
	{	bitBlt(this, area.left(), fcy, &hline, 0, 0, area.width(), 1);
		bitBlt(this, fcx, area.top(), &vline, 0, 0, 1, area.height());
		csflg=0;
	}

	if(area.contains(e->pos()))
	{   QPoint ptd, ptl;
		QPainter DC;

		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		ptl=DC.xFormDev(e->pos());
		if((csmode=m_parser->chkfix(csmode)) >= 0)
		{   ptl.setY(dgr.Transy(csypos=m_parser->fkt(csmode, csxpos=dgr.Transx(ptl.x()))));

            if(fabs(csypos)<0.2)
            {   double x0;
                QString str;

                if(root(&x0))
                {   str="  ";
                    str+=i18n("root");
                    stbar->changeItem(str+QString().sprintf(":  x0= %+.5f", x0), 3);
                    rootflg=1;
                }
            }
            else
            {   stbar->changeItem("", 3);
                rootflg=0;
            }
        }
		else
		{   csxpos=dgr.Transx(ptl.x());
			csypos=dgr.Transy(ptl.y());
		}
		ptd=DC.xForm(ptl);
		DC.end();

		sprintf(sx, "  x= %+.2f", (float)dgr.Transx(ptl.x()));//csxpos);
		sprintf(sy, "  y= %+.2f", csypos);

		if(csflg==0)        // Hintergrund speichern
		{	bitBlt(&hline, 0, 0, this, area.left(), fcy=ptd.y(), area.width(), 1);
			bitBlt(&vline, 0, 0, this, fcx=ptd.x(), area.top(), 1, area.height());

			// Fadenkreuz zeichnen
			QPen pen((csmode>=0)? m_parser->fktext[csmode].color : 0, 1);

			DC.begin(this);
			DC.setPen(pen);
			DC.Lineh(area.left(), fcy, area.right());
			DC.Linev(fcx, area.bottom(), area.top());
			DC.end();
		}
		csflg=1;
		setCursor(blankCursor);
	}
	else
	{   setCursor(arrowCursor);
		sx[0]=sy[0]=0;
	}

	stbar->changeItem(sx, 1);
	stbar->changeItem(sy, 2);
}


void View::mousePressEvent(QMouseEvent *e)
{   int ix, k, ke;
	double g;

	if(e->button()!=LeftButton) return ;
	if(csmode>=0)
	{   csmode=-1;
		mouseMoveEvent(e);
		return ;
	}

	g=tlgy/5.;
	for(ix=0; ix<m_parser->ufanz; ++ix)
	{   switch(m_parser->fktext[ix].extstr[0].latin1())
		{  case 0:
		   case 'x':
		   case 'y':
		   case 'r':    continue;   // Fangen nicht m�lich
		}

		k=0;
		ke=m_parser->fktext[ix].k_anz;
		do
		{   m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[k]);
			if(fabs(csypos-m_parser->fkt(ix, csxpos))< g)
			{   csmode=ix;
				mouseMoveEvent(e);
				return ;
			}
		}
		while(++k<ke);
	}

	csmode=-1;
}


void View::coordToMinMax( const int koord, const QString minStr, const QString maxStr, 
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
		min = m_parser->eval( minStr );
		max = m_parser->eval( maxStr );
	}
}

void View::setPlotRange()
{
	coordToMinMax( Settings::xRange(), Settings::xMin(), Settings::xMax(), xmin, xmax );
	coordToMinMax( Settings::yRange(), Settings::yMin(), Settings::yMax(), ymin, ymax );
}

void View::setScaling()
{
	const char* units[ 8 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4" };
	
	tlgxstr = units[ Settings::xScaling() ];
	tlgx = m_parser->eval( tlgxstr );
	tlgystr = units[ Settings::yScaling() ];
	tlgy = m_parser->eval( tlgystr );

	drskalxstr = units[ Settings::xPrinting() ];
	drskalx = m_parser->eval( drskalxstr );
	drskalystr = units[ Settings::yPrinting() ];
	drskaly = m_parser->eval( drskalystr );
}

void View::getSettings()
{
	m_parser->setAngleMode( Settings::anglemode() );
	m_parser->dicke0 = Settings::gridLineWidth();
	m_parser->fktext[ 0 ].color = Settings::color0().rgb();
	m_parser->fktext[ 1 ].color = Settings::color1().rgb();
	m_parser->fktext[ 2 ].color = Settings::color2().rgb();
	m_parser->fktext[ 3 ].color = Settings::color3().rgb();
	m_parser->fktext[ 4 ].color = Settings::color4().rgb();
	m_parser->fktext[ 5 ].color = Settings::color5().rgb();
	m_parser->fktext[ 6 ].color = Settings::color6().rgb();
	m_parser->fktext[ 7 ].color = Settings::color7().rgb();
	m_parser->fktext[ 8 ].color = Settings::color8().rgb();
	m_parser->fktext[ 9 ].color = Settings::color9().rgb();
}

void View::init()
{
	getSettings();

	for ( int ix = 0; ix < m_parser->ufanz; ++ix )
		m_parser->delfkt( ix );
}