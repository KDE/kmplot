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

// Qt includes
#include <qbitmap.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qpicture.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprogress.h>

// local includes
#include "editfunction.h"
#include "keditparametric.h"
#include "kminmax.h"
#include "settings.h"
#include "ksliderwindow.h"
#include "View.h"
#include "View.moc"

//minimum and maximum x range. Should always be accessible.
double View::xmin = 0;
double View::xmax = 0;


View::View(bool const r, bool &mo, KPopupMenu *p, QWidget* parent, const char* name ) : DCOPObject("View"), QWidget( parent, name , WStaticContents ),  buffer( width(), height() ), m_popupmenu(p), m_modified(mo), m_readonly(r), m_dcop_client(KApplication::kApplication()->dcopClient())
{
	csmode = csparam = -1;
	cstype = 0;
	areaDraw = false;
	areaUfkt = 0;
	areaPMode = 0;
	areaMin = areaMax = 0.0;
	w = h = 0;
	s = 0.0;
	fcx = 0;
	fcy = 0;
	csxpos = 0.0;
	csypos = 0.0;
	rootflg = false;
	tlgx = tlgy = drskalx = drskaly = 0.0;;
	stepWidth = 0.0;
	ymin = 0.0;;
	ymax = 0.0;;
	m_printHeaderTable = false;
	stop_calculating = false;
	m_minmax = 0;
	isDrawing = false;
	m_popupmenushown = 0;
	
	m_parser = new XParser(mo);
	init();
	csflg=0;
	csmode=-1;
	backgroundcolor = Settings::backgroundcolor();
	invertColor(backgroundcolor,inverted_backgroundcolor);
	setBackgroundColor(backgroundcolor);
	setMouseTracking(true);
	rootflg = false;
	for( int number = 0; number < SLIDER_COUNT; number++ )
		sliders[ number ] = 0;
	updateSliders();
	m_popupmenushown = 0;
	m_popupmenu->insertTitle( "",10);
    zoom_mode = Z_Normal;
	isDrawing=false;
	areaDraw = false;
}

void View::setMinMaxDlg(KMinMax *minmaxdlg)
{
	m_minmax = minmaxdlg;
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
{
	int lx, ly;
	float sf;
	QRect rc;
	QPainter DC;    // our painter
	DC.begin(dev);    // start painting widget
	rc=DC.viewport();
	w=rc.width();
	h=rc.height();

	setPlotRange();
	setScaling();

	if(form==0)          // screen
	{
		ref=QPoint(120, 100);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC.scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
		if(DC.xForm(QPoint(lx+2*ref.x(), ly)).x() > DC.viewport().right())
		{
			DC.resetXForm();
			DC.scale((float)w/(float)(lx+2*ref.x()), (float)w/(float)(lx+2*ref.x()));
		}
		wm=DC.worldMatrix();
		s=DC.xForm(QPoint(1000, 0)).x()/1000.;
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
	}
	else if(form==1)        // printer
	{
		sf=72./254.;        // 72dpi
		ref=QPoint(100, 100);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC.scale(sf, sf);
		s=1.;
		m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1";
		drawHeaderTable( &DC );
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		if ( ( (KPrinter* )dev )->option( "app-kmplot-printbackground" ) == "-1" )
			DC.fillRect( dgr.GetFrame(),  backgroundcolor); //draw a colored background
		//DC.end();
		//((QPixmap *)dev)->fill(QColor("#FF00FF"));
		//DC.begin(dev);
	}
	else if(form==2)								// svg
	{
		ref=QPoint(0, 0);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.translate(-dgr.GetFrame().left(), -dgr.GetFrame().top());
		s=1.;
	}
	else if(form==3)								// bmp, png
	{
		sf=180./254.;								// 180dpi
		ref=QPoint(0, 0);
		lx=(int)((xmax-xmin)*100.*drskalx/tlgx);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.end();
		((QPixmap *)dev)->resize((int)(dgr.GetFrame().width()*sf), (int)(dgr.GetFrame().height()*sf));
		((QPixmap *)dev)->fill(backgroundcolor);
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

	if ( form!=0 && areaDraw)
	{
		areaUnderGraph(areaUfkt, areaPMode, areaMin,areaMax, areaParameter, &DC);
		areaDraw = false;
		if (stop_calculating)
			return;
	}

	dgr.Plot(&DC);
	PlotArea=dgr.GetPlotArea();
	area=DC.xForm(PlotArea);
	hline.resize(area.width(), 1);
	vline.resize(1, area.height());
	stepWidth=Settings::stepWidth();

	isDrawing=true;
	setCursor(Qt::WaitCursor );
	stop_calculating = false;
	for(QValueVector<Ufkt>::iterator ufkt=m_parser->ufkt.begin(); ufkt!=m_parser->ufkt.end() && !stop_calculating; ++ufkt)
		if ( !ufkt->fname.isEmpty() )
			plotfkt(ufkt, &DC);

	isDrawing=false;
	restoreCursor();
	csflg=0;
	DC.end();   // painting done
}


void View::plotfkt(Ufkt *ufkt, QPainter *pDC)
{
	char p_mode;
	int iy, k, ke, mflg;
	double x, y, dmin, dmax;
	QPoint p1, p2;
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);
	iy=0;
	y=0.0;

	char const fktmode=ufkt->fstr[0].latin1();
	if(fktmode=='y') return ;

	dmin=ufkt->dmin;
	dmax=ufkt->dmax;

	if(!ufkt->usecustomxmin)
	{
	  if(fktmode=='r')
	    dmin=0.;
	  else if ( fktmode == 'x' )
		  dmin = -M_PI;
	  else
	    dmin = xmin;
	}
	if(!ufkt->usecustomxmax)
	{
	  if(fktmode=='r')
		  dmax=2*M_PI;
	  else if ( fktmode == 'x' )
		  dmax = M_PI;
	  else
	    dmax = xmax;
	}
	double dx;
	if(fktmode=='r')
		if ( Settings::useRelativeStepWidth() )
			dx=stepWidth*0.05/(dmax-dmin);
		else
			dx=stepWidth;
	else
		if ( Settings::useRelativeStepWidth() )
			dx=stepWidth*(dmax-dmin)/area.width();
		else
			dx=stepWidth;

	if(fktmode=='x')
		iy = m_parser->ixValue(ufkt->id)+1;
	p_mode=0;
	pen.setWidth((int)(ufkt->linewidth*s) );
	pen.setColor(ufkt->color);
	pDC->setPen(pen);

	while(1)
	{
		k=0;
		ke=ufkt->parameters.count();
		do
		{
			kdDebug() << "drawing " << ufkt->id << endl;
			if ( p_mode == 3 && stop_calculating)
				break;
			if( ufkt->use_slider == -1 )
			{
				if ( !ufkt->parameters.isEmpty() )
					ufkt->setParameter( ufkt->parameters[k].value );
			}
			else
			{
				if ( KSliderWindow * sw = sliders[ ufkt->use_slider ] )
					ufkt->setParameter( sw->slider->value() );
			}

			mflg=2;
			if ( p_mode == 3)
			{
				if ( ufkt->integral_use_precision )
					if ( Settings::useRelativeStepWidth() )
						dx =  ufkt->integral_precision*(dmax-dmin)/area.width();
					else
						dx =  ufkt->integral_precision;
				startProgressBar((int)double((dmax-dmin)/dx)/2);
				x = ufkt->oldx = ufkt->startx; //the initial x-point
				ufkt->oldy = ufkt->starty;
				ufkt->oldyprim = ufkt->integral_precision;
				paintEvent(0);
			}
			else
				x=dmin;
			bool forward_direction;

			if (dmin<0 && dmax<0)
				forward_direction = false;
			else
				forward_direction = true;

			if ( p_mode != 0 || ufkt->f_mode) // if not the function is hidden
				while ((x>=dmin && x<=dmax) ||  (p_mode == 3 && x>=dmin && !forward_direction) || (p_mode == 3 && x<=dmax && forward_direction))
				{
					if ( p_mode == 3 && stop_calculating)
					{
						p_mode=1;
						x=dmax+1;
						continue;
					}
					switch(p_mode)
					{
					case 0:
						y=m_parser->fkt(ufkt, x);
						break;
					case 1:
						y=m_parser->a1fkt(ufkt, x);
						break;
					case 2:
						y=m_parser->a2fkt(ufkt, x);
						break;
					case 3:
						{
							y = m_parser->euler_method(x, ufkt);
							if ( int(x*100)%2==0)
							{
								KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
								increaseProgressBar();
							}
							break;
						}
					}

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

					if ( dgr.xclipflg || dgr.yclipflg )
					{
						//if(mflg>=1)
							p1=p2;
						/*else
						{
							pDC->drawLine(p1, p2);
							p1=p2;
							mflg=1;
						}*/
					}
					else
					{
						if(mflg<=1)
							pDC->drawLine(p1, p2);
						p1=p2;
						mflg=0;
					}

					if (p_mode==3)
					{
						if ( forward_direction)
						{
							x=x+dx;
							if (x>dmax && p_mode== 3)
							{
								forward_direction = false;
								x = ufkt->oldx = ufkt->startx;
								ufkt->oldy = ufkt->starty;
								ufkt->oldyprim = ufkt->integral_precision;
								paintEvent(0);
								mflg=2;
							}
						}
						else
							x=x-dx; // go backwards
					}
					else
						x=x+dx;
				}
		}
		while(++k<ke);

		if(ufkt->f1_mode==1 && p_mode< 1) //draw the 1st derivative
		{
			p_mode=1;
			pen.setWidth((int)(ufkt->f1_linewidth*s) );
			pen.setColor(ufkt->f1_color);
			pDC->setPen(pen);
		}
		else if(ufkt->f2_mode==1 && p_mode< 2) //draw the 2nd derivative
		{
			p_mode=2;
			pen.setWidth((int)(ufkt->f2_linewidth*s) );
			pen.setColor(ufkt->f2_color);
			pDC->setPen(pen);
		}
		else if( ufkt->integral_mode==1 && p_mode< 3) //draw the integral
		{
			p_mode=3;
			pen.setWidth((int)(ufkt->integral_linewidth*s) );
			pen.setColor(ufkt->integral_color);
			pDC->setPen(pen);
		}
		else break; //otherwise stop
	}
	if ( stopProgressBar() )
		if( stop_calculating)
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
}

void View::drawHeaderTable(QPainter *pDC)
{
	QString alx, aly, atx, aty, dfx, dfy;

	if( m_printHeaderTable )
	{
		pDC->translate(250., 150.);
		pDC->setPen(QPen(black, (int)(5.*s)));
		pDC->setFont(QFont( Settings::headerTableFont(), 30) );
		puts( Settings::headerTableFont().latin1() );
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
		int ypos = 380;
		//for(uint ix=0; ix<m_parser->countFunctions() && !stop_calculating; ++ix)
		for(QValueVector<Ufkt>::iterator it=m_parser->ufkt.begin(); it!=m_parser->ufkt.end() && !stop_calculating; ++it)
		{
			pDC->drawText(100, ypos, it->fstr);
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
{
	int i;
	QChar c(960);

	while((i=s->find('p')) != -1) s->replace(i, 2, &c, 1);
}


bool View::root(double *x0, Ufkt *it)
{
	if(rootflg)
		return false;
	double yn;
	double x=csxpos;
	double y=fabs(csypos);
	double dx=0.1;
	while(1)
	{
		if((yn=fabs(m_parser->fkt(it, x-dx))) < y)
		{
			x-=dx;
			y=yn;
		}
		else if((yn=fabs(m_parser->fkt(it, x+dx))) < y)
		{
			x+=dx;
			y=yn;
		}
		else
			dx/=10.;
		printf("x=%g,  dx=%g, y=%g\n", x, dx, y);
		if(y<1e-8)
		{
			*x0=x;
			return true;
		}
		if(fabs(dx)<1e-8)
			return false;
		if(x<xmin || x>xmax)
			return false;
	}
}

void View::paintEvent(QPaintEvent *)
{
	QPainter p;
	p.begin(this);
	bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
	p.end();
}

void View::resizeEvent(QResizeEvent *)
{
	if (isDrawing) //stop drawing integrals
	{
		stop_calculating = true; //stop drawing
		return;
	}
	buffer.resize(size() );
	drawPlot();
}

void View::drawPlot()
{
	if( m_minmax->isShown() )
		m_minmax->updateFunctions();
	buffer.fill(backgroundcolor);
	draw(&buffer, 0);
	QPainter p;
	p.begin(this);
	bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
	p.end();
}

void View::mouseMoveEvent(QMouseEvent *e)
{
	if ( isDrawing)
		return;
	if (zoom_mode==4 &&  e->stateAfter() != Qt::NoButton)
	{
		QPainter p;
		p.begin(this);
		bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
		p.end();

		QPainter painter(this);
		QPen pen(Qt::white, 1, Qt::DotLine);
		painter.setRasterOp (Qt::XorROP);
		painter.setPen(pen);
		painter.setBackgroundMode (QPainter::OpaqueMode);
		painter.setBackgroundColor (Qt::blue);

		painter.drawRect(rectangle_point.x(), rectangle_point.y(), e->pos().x()-rectangle_point.x(), e->pos().y()-rectangle_point.y());
		return;

	}
	if ( zoom_mode!=0)
		return;
	if( m_popupmenushown>0 && !m_popupmenu->isShown() )
	{
		if ( m_popupmenushown==1)
			csmode=-1;
		m_popupmenushown = 0;
		return;
	}
	if(csflg==1)        // Fadenkreuz l�chen
	{
	  	bitBlt(this, area.left(), fcy, &hline, 0, 0, area.width(), 1);
		bitBlt(this, fcx, area.top(), &vline, 0, 0, 1, area.height());
		csflg=0;
	}
	
	if(area.contains(e->pos()) || (e->button()==Qt::LeftButton && e->state()==Qt::LeftButton && csxpos>xmin && csxpos<xmax))
	{
		QPoint ptd, ptl;
		QPainter DC;
		bool out_of_bounds = false;

		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		ptl=DC.xFormDev(e->pos());
		Ufkt *it = 0;
		if( csmode >= 0 && csmode <= (int)m_parser->countFunctions() )
		{
			int const ix = m_parser->ixValue(csmode);
			if (ix!=-1 && ((!m_parser->ufkt[ix].usecustomxmin) || (m_parser->ufkt[ix].usecustomxmin && csxpos>m_parser->ufkt[ix].dmin)) && ((!m_parser->ufkt[ix].usecustomxmax)||(m_parser->ufkt[ix].usecustomxmax && csxpos<m_parser->ufkt[ix].dmax)) )
			{
				it = &m_parser->ufkt[ix];
				if( it->use_slider == -1 )
				{
					if( it->parameters.isEmpty() )
						it->setParameter( it->parameters[csparam].value );
				}
				else
					it->setParameter(sliders[ it->use_slider ]->slider->value() );
				if ( cstype == 0)
					ptl.setY(dgr.Transy(csypos=m_parser->fkt( it, csxpos=dgr.Transx(ptl.x()))));
				else if ( cstype == 1)
					ptl.setY(dgr.Transy(csypos=m_parser->a1fkt( it, csxpos=dgr.Transx(ptl.x()) )));
				else if ( cstype == 2)
					ptl.setY(dgr.Transy(csypos=m_parser->a2fkt( it, csxpos=dgr.Transx(ptl.x()))));

				if ( csypos<ymin || csypos>ymax) //the ypoint is not visible
					out_of_bounds = true;
				else if(fabs(dgr.Transy(ptl.y())) < (xmax-xmin)/80)
				{
					double x0;
					if(root(&x0, it))
					{
						QString str="  ";
						str+=i18n("root");
						setStatusBar(str+QString().sprintf(":  x0= %+.5f", x0), 3);
						rootflg=true;
					}
				}
				else
				{
					setStatusBar("", 3);
					rootflg=false;
				}
			}
			else
			{
			  csxpos=dgr.Transx(ptl.x());
			  csypos=dgr.Transy(ptl.y());
			  csflg = 1;
			}
		}
		else
		{
			csxpos=dgr.Transx(ptl.x());
			csypos=dgr.Transy(ptl.y());
		}
		ptd=DC.xForm(ptl);
		DC.end();
		QString sx, sy;
		if (out_of_bounds)
		{
		  sx = sy = "";
		}
		else
		{
		  sx.sprintf("  x= %+.2f", (float)dgr.Transx(ptl.x()));//csxpos);
		  sy.sprintf("  y= %+.2f", csypos);
		}
		if(csflg==0)        // Hintergrund speichern
		{
			bitBlt(&hline, 0, 0, this, area.left(), fcy=ptd.y(), area.width(), 1);
			bitBlt(&vline, 0, 0, this, fcx=ptd.x(), area.top(), 1, area.height());
			// Fadenkreuz zeichnen
			QPen pen;
			if ( csmode == -1)
				pen.setColor(inverted_backgroundcolor);
			else
			{
				if ( csmode == -1)
					pen.setColor(inverted_backgroundcolor);
				else
				{
					switch (cstype)
					{
					case 0:
						pen.setColor( it->color);
						break;
					case 1:
						pen.setColor( it->f1_color);
						break;
					case 2:
						pen.setColor( it->f2_color);
						break;
					default:
						pen.setColor(inverted_backgroundcolor);
					}
					if ( pen.color() == backgroundcolor) // if the "Fadenkreuz" has the same color as the background, the "Fadenkreuz" will have the inverted color of background so you can see it easier
						pen.setColor(inverted_backgroundcolor);
				}
			}
			DC.begin(this);
			DC.setPen(pen);
			DC.Lineh(area.left(), fcy, area.right());
			DC.Linev(fcx, area.bottom(), area.top());
			DC.end();
		}
		csflg=1;
		setCursor(Qt::blankCursor);
		setStatusBar(sx, 1);
		setStatusBar(sy, 2);
	}
	else
	{
		setCursor(arrowCursor);
		setStatusBar("", 1);
		setStatusBar("", 2);
	}
	
}


void View::mousePressEvent(QMouseEvent *e)
{
	if ( m_popupmenushown>0)
		return;

	if (isDrawing)
	{
		stop_calculating = true; //stop drawing
		return;
	}

	if (  zoom_mode==Z_Rectangular ) //rectangle zoom
	{
		zoom_mode=Z_Center;
		rectangle_point = e->pos();
		return;
	}
	else if (  zoom_mode==Z_ZoomIn ) //zoom in
	{
		QPainter DC;
		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		double real = dgr.Transx(DC.xFormDev(e->pos()).x());

		double const diffx = (xmax-xmin)*(double)Settings::zoomInStep()/100;
		double const diffy = (ymax-ymin)*(double)Settings::zoomInStep()/100;

		if ( diffx < 0.00001 || diffy < 0.00001)
			return;
		
		Settings::setXMin( Parser::number( real-double(diffx) ) );
		Settings::setXMax( Parser::number( real+double(diffx) ) );

		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		Settings::setYMin( Parser::number( real-double(diffy) ) );
		Settings::setYMax( Parser::number( real+double(diffy) ) );

		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
		return;

	}
	else if (  zoom_mode==3 ) //zoom out
	{
		QPainter DC;
		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		double real = dgr.Transx(DC.xFormDev(e->pos()).x());

		double const diffx = (xmax-xmin)*(((double)Settings::zoomOutStep()/100) +1);
		double const diffy = (ymax-ymin)*(((double)Settings::zoomOutStep()/100) +1);

		if ( diffx > 1000000 || diffy > 1000000)
			return;
		
		Settings::setXMin( Parser::number( real-double(diffx) ) );
		Settings::setXMax( Parser::number( real+double(diffx) ) );

		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		Settings::setYMin( Parser::number( real-double(diffy) ) );
		Settings::setYMax( Parser::number( real+double(diffy) ) );

		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
		return;

	}
	else if (  zoom_mode==5 ) //center
	{
		QPainter DC;
		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		double real = dgr.Transx(DC.xFormDev(e->pos()).x());
		
		double const diffx = (xmax-xmin)/2;
		double const diffy = (ymax-ymin)/2;

		Settings::setXMin( Parser::number( real-double(diffx) ) );
		Settings::setXMax( Parser::number( real+double(diffx) ) );

		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		Settings::setYMin( Parser::number( real-double(diffy) ) );
		Settings::setYMax( Parser::number( real+double(diffy) ) );

		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
		return;

	}
	double const g=tlgy*double(xmax-xmin)/(2*double(ymax-ymin));
	if( !m_readonly && e->button()==RightButton) //clicking with the right mouse button
	{
		char function_type;
		for( QValueVector<Ufkt>::iterator it = m_parser->ufkt.begin(); it != m_parser->ufkt.end(); ++it)
		{
			function_type = it->fstr[0].latin1();
			if ( function_type=='y' || function_type=='r' || it->fname.isEmpty()) continue;
			if (!(((!it->usecustomxmin) || (it->usecustomxmin && csxpos>it->dmin)) && ((!it->usecustomxmax)||(it->usecustomxmax && csxpos<it->dmax)) ))
			  continue;
			kdDebug() << "it:" << it->fstr << endl;
			int k=0;
			int const ke=it->parameters.count();
			do
			{
				if( it->use_slider == -1 )
				{
					if ( !it->parameters.isEmpty())
						it->setParameter(it->parameters[k].value);
				}
				else
				{
					if ( KSliderWindow * sw = sliders[ it->use_slider ] )
						it->setParameter( sw->slider->value() );
				}

				if ( function_type=='x' &&  fabs(csxpos-m_parser->fkt(it, csxpos))< g && it->fstr.contains('t')==1) //parametric plot
				{
					QValueVector<Ufkt>::iterator ufkt_y = it+1;
					if ( fabs(csypos-m_parser->fkt(ufkt_y, csxpos)<g)  && ufkt_y->fstr.contains('t')==1)
					{
						if ( csmode == -1)
						{
							csmode=it->id;
							cstype=0;
							csparam = k;
							m_popupmenushown = 1;
						}
						else
							m_popupmenushown = 2;
						QString y_name( ufkt_y->fstr );
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-1),false);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-2),false);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-3),false);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-4),false);
						m_popupmenu->changeTitle(10,ufkt_y->fstr+";"+y_name);
						m_popupmenu->exec(QCursor::pos());
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-1),true);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-2),true);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-3),true);
						m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-4),true);
						return;
					}
				}
				else if( fabs(csypos-m_parser->fkt(it, csxpos))< g && it->f_mode)
				{
					if ( csmode == -1)
					{
						csmode=it->id;
						cstype=0;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					m_popupmenu->changeTitle(10, it->fstr);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				else if(fabs(csypos-m_parser->a1fkt( it, csxpos))< g && it->f1_mode)
				{
					if ( csmode == -1)
					{
						csmode=it->id;
						cstype=1;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					QString function = it->fstr;
					function = function.left(function.find('(')) + '\'';
					m_popupmenu->changeTitle(10, function);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				else if(fabs(csypos-m_parser->a2fkt(it, csxpos))< g && it->f2_mode)
				{
					if ( csmode == -1)
					{
						csmode=it->id;
						cstype=2;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					QString function = it->fstr;
					function = function.left(function.find('(')) + "\'\'";
					m_popupmenu->changeTitle(10, function);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
			}
			while(++k<ke);
		}
		return;
	}
	if(e->button()!=LeftButton) return ;
	if(csmode>=0) //disable trace mode if trace mode is enable
	{
		csmode=-1;
		setStatusBar("",3);
		setStatusBar("",4);
		mouseMoveEvent(e);
		return ;
	}
	for( QValueVector<Ufkt>::iterator it = m_parser->ufkt.begin(); it != m_parser->ufkt.end(); ++it)
	{
		if (it->fname.isEmpty() )
			continue;
		switch(it->fstr[0].latin1())
		{
			case 'x': case 'y': case 'r': continue;   // Not possible to catch
		}
		if (!(((!it->usecustomxmin) || (it->usecustomxmin && csxpos>it->dmin)) && ((!it->usecustomxmax)||(it->usecustomxmax && csxpos<it->dmax)) ))
		  continue;
		int k=0;
		int const ke=it->parameters.count();
		do
		{
			if( it->use_slider == -1 )
			{
				if ( !it->parameters.isEmpty() )
					it->setParameter( it->parameters[k].value );
			}
			else
			{
				if ( KSliderWindow * sw = sliders[ it->use_slider ] )
					it->setParameter( sw->slider->value() );
			}
			if(fabs(csypos-m_parser->fkt(it, csxpos))< g && it->f_mode)
			{
				csmode=it->id;
				cstype=0;
				csparam = k;
				m_minmax->selectItem();
				setStatusBar(it->fstr,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a1fkt( it, csxpos))< g && it->f1_mode)
			{
				csmode=it->id;
				cstype=1;
				csparam = k;
				m_minmax->selectItem();
				QString function = it->fstr;
				function = function.left(function.find('(')) + '\'';
				setStatusBar(function,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a2fkt(it, csxpos))< g && it->f2_mode)
			{
				csmode=it->id;
				cstype=2;
				csparam = k;
				m_minmax->selectItem();
				QString function = it->fstr;
				function = function.left(function.find('(')) + "\'\'";
				setStatusBar(function,4);
				mouseMoveEvent(e);
				return;
			}
		}
		while(++k<ke);
	}

	csmode=-1;
}


void View::mouseReleaseEvent ( QMouseEvent * e )
{
	if ( zoom_mode == Z_Center )
	{
		zoom_mode = Z_Rectangular;
		if( (e->pos().x() - rectangle_point.x() >= -2) && (e->pos().x() - rectangle_point.x() <= 2) ||
		        (e->pos().y() - rectangle_point.y() >= -2) && (e->pos().y() - rectangle_point.y() <= 2) )
		{
			update();
			return;
		}
		QPainter DC;
		DC.begin(this);
		bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);

		QPoint p=DC.xFormDev(e->pos());
		double real1x = dgr.Transx(p.x() ) ;
		double real1y = dgr.Transy(p.y() ) ;
		p=DC.xFormDev(rectangle_point);
		double real2x = dgr.Transx(p.x() ) ;
		double real2y = dgr.Transy(p.y() ) ;


		if ( real1x>xmax || real2x>xmax || real1x<xmin  || real2x<xmin  ||
		        real1y>ymax || real2y>ymax || real1y<ymin  || real2y<ymin )
			return; //out of bounds

		//setting new x-boundaries
		if( real1x < real2x  )
		{
			if( real2x - real1x < 0.00001)
				return;
			Settings::setXMin( Parser::number( real1x ) );
			Settings::setXMax( Parser::number( real2x ) );
		}
		else
		{
			if (real1x - real2x < 0.00001)
				return;
			Settings::setXMin( Parser::number( real2x ) );
			Settings::setXMax( Parser::number( real1x ) );
		}
		//setting new y-boundaries
		if( real1y < real2y )
		{
			if( real2y - real1y < 0.00001)
				return;
			Settings::setYMin( Parser::number( real1y ) );
			Settings::setYMax( Parser::number( real2y ) );
		}
		else
		{
			if( real1y - real2y < 0.00001)
				return;
			Settings::setYMin( Parser::number( real2y ) );
			Settings::setYMax( Parser::number( real1y ) );
		}
		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
	}
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
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };

	if( Settings::xScaling() == 8) //automatic x-scaling
    {
		tlgx = double(xmax-xmin)/16;
        tlgxstr = units[ Settings::xScaling() ];
    }
	else
	{
		tlgxstr = units[ Settings::xScaling() ];
		tlgx = m_parser->eval( tlgxstr );
	}

	if( Settings::yScaling() == 8)  //automatic y-scaling
    {
		tlgy = double(ymax-ymin)/16;
        tlgystr = units[ Settings::yScaling() ];
    }
	else
	{
		tlgystr = units[ Settings::yScaling() ];
		tlgy = m_parser->eval( tlgystr );
	}

	drskalxstr = units[ Settings::xPrinting() ];
	drskalx = m_parser->eval( drskalxstr );
	drskalystr = units[ Settings::yPrinting() ];
	drskaly = m_parser->eval( drskalystr );
}

void View::getSettings()
{
	m_parser->setAngleMode( Settings::anglemode() );
	m_parser->linewidth0 = Settings::gridLineWidth();

	backgroundcolor = Settings::backgroundcolor();
	invertColor(backgroundcolor,inverted_backgroundcolor);
	setBackgroundColor(backgroundcolor);
}

void View::init()
{
	getSettings();
	QValueVector<Ufkt>::iterator it = m_parser->ufkt.begin();
	it->fname="";
	while ( m_parser->ufkt.count() > 1)
		m_parser->Parser::delfkt( &m_parser->ufkt.last() );
}


void View::stopDrawing()
{
	if (isDrawing)
		stop_calculating = true;
}

void View::findMinMaxValue(Ufkt *ufkt, char p_mode, bool minimum, double &dmin, double &dmax, const QString &str_parameter)
{
	double x, y = 0;
	double result_x = 0;
	double result_y = 0;
	bool start = true;

	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QValueList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression == str_parameter)
			{
				ufkt->setParameter( (*it).value );
				break;
			}
		}
	}

	isDrawing=true;
	setCursor(Qt::WaitCursor );

	double dx;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		if ( ufkt->integral_use_precision )
			if ( ufkt->integral_use_precision )
				dx = ufkt->integral_precision*(dmax-dmin)/area.width();
			else
				dx = ufkt->integral_precision;
		else
			if ( ufkt->integral_use_precision )
				dx = stepWidth*(dmax-dmin)/area.width();
			else
				dx = stepWidth;
		startProgressBar((int)double((dmax-dmin)/dx)/2);
		x = ufkt->oldx = ufkt->startx; //the initial x-point
		ufkt->oldy = ufkt->starty;
		ufkt->oldyprim = ufkt->integral_precision;
		paintEvent(0);
	}
	else
	{
		dx = stepWidth*(dmax-dmin)/area.width();
		x=dmin;
	}

	bool forward_direction;
	if (dmin<0 && dmax<0)
		forward_direction = false;
	else
		forward_direction = true;
	while ((x>=dmin && x<=dmax) ||  (p_mode == 3 && x>=dmin && !forward_direction) || (p_mode == 3 && x<=dmax && forward_direction))
	{
		if ( p_mode == 3 && stop_calculating)
		{
			p_mode = 1;
			x=dmax+1;
			continue;
		}
		switch(p_mode)
		{
		case 0:
			y=m_parser->fkt(ufkt, x);
			break;

		case 1:
			{
				y=m_parser->a1fkt( ufkt, x);
				break;
			}
		case 2:
			{
				y=m_parser->a2fkt(ufkt, x);
				break;
			}
		case 3:
			{
				y = m_parser->euler_method(x, ufkt);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
					increaseProgressBar();
				}
				break;
			}
		}
		if ( !isnan(x) && !isnan(y) )
		{
			kdDebug() << "x " << x << endl;
			kdDebug() << "y " << y << endl;
			if (x>=dmin && x<=dmax)
			{
				if ( start)
				{
					result_x = x;
					result_y = y;
					start=false;
				}
				else if ( minimum &&y <=result_y)
				{
					result_x = x;
					result_y = y;
				}
				else if ( !minimum && y >=result_y)
				{
					result_x = x;
					result_y = y;
				}
			}
		}
		if (p_mode==3)
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode== 3)
				{
					forward_direction = false;
					x = ufkt->oldx = ufkt->startx;
					ufkt->oldy = ufkt->starty;
					ufkt->oldyprim = ufkt->integral_precision;
					paintEvent(0);
				}
			}
			else
				x=x-dx; // go backwards
		}
		else
			x=x+dx;
	}
	stopProgressBar();
	isDrawing=false;
	restoreCursor();
	
	dmin = int(result_x*1000)/double(1000);
	dmax = int(result_y*1000)/double(1000);
	
	switch (p_mode)
	{
	case 0:
		dmax=m_parser->fkt(ufkt, dmin);
		break;
	case 1:
		dmax=m_parser->a1fkt(ufkt, dmin);
		break;
	case 2:
		dmax=m_parser->a2fkt(ufkt, dmin);
		break;
	}
}

void View::getYValue(Ufkt *ufkt, char p_mode,  double x, double &y, const QString &str_parameter)
{
	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QValueList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression == str_parameter)
			{
				ufkt->setParameter((*it).value);
				break;
			}
		}
	}

	switch (p_mode)
	{
	case 0:
		y= m_parser->fkt(ufkt, x);
		break;
	case 1:
		y=m_parser->a1fkt( ufkt, x);
		break;
	case 2:
		y=m_parser->a2fkt( ufkt, x);
		break;
	case 3:
		double dmin = ufkt->dmin;
		double dmax = ufkt->dmax;
		const double target = x; //this is the x-value the user had chosen
		bool forward_direction;
		if ( target>=0)
			forward_direction = true;
		else
			forward_direction = false;

		if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
		{
			dmin=xmin;
			dmax=xmax;
		}

		double dx;
		if ( ufkt->integral_use_precision )
			dx = ufkt->integral_precision*(dmax-dmin)/area.width();
		else
			dx=stepWidth*(dmax-dmin)/area.width();

		stop_calculating = false;
		isDrawing=true;
		setCursor(Qt::WaitCursor );
		bool target_found=false;
		startProgressBar((int) double((dmax-dmin)/dx)/2);
		x = ufkt->oldx = ufkt->startx; //the initial x-point
		ufkt->oldy = ufkt->starty;
		ufkt->oldyprim = ufkt->integral_precision;
		paintEvent(0);
		while (x>=dmin && !stop_calculating && !target_found)
		{
			y = m_parser->euler_method( x, ufkt );
			if ( int(x*100)%2==0)
			{
				KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
				increaseProgressBar();
			}

			if ( (x+dx > target && forward_direction) || ( x+dx < target && !forward_direction)) //right x-value is found
				target_found = true;



			if (forward_direction)
			{
				x=x+dx;
				if (x>dmax)
				{
					forward_direction = false;
					x = ufkt->oldx = ufkt->startx;
					ufkt->oldy = ufkt->starty;
					ufkt->oldyprim = ufkt->integral_precision;
					paintEvent(0);
				}
			}
			else
				x=x-dx; // go backwards
		}
		stopProgressBar();
		isDrawing=false;
		restoreCursor();
		break;
	}
}

void View::keyPressEvent( QKeyEvent * e)
{
    if ( zoom_mode != Z_Normal && e->key() == Qt::Key_Escape )
    {
      resetZoom();
      return;
    }
	if ( zoom_mode == Z_Center) //drawing a rectangle
	{
		zoom_mode = Z_Rectangular;
		update();
		return;
	}

	if (isDrawing)
	{
		stop_calculating=true;
		return;
	}

	if (csmode==-1 ) return;

	QMouseEvent *event;
	if (e->key() == Qt::Key_Left )
		event = new QMouseEvent(QEvent::MouseMove,QPoint(fcx-1,fcy-1),Qt::LeftButton,Qt::LeftButton);
	else if (e->key() == Qt::Key_Right )
		event = new QMouseEvent(QEvent::MouseMove,QPoint(fcx+1,fcy+1),Qt::LeftButton,Qt::LeftButton);
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) //switch graph in trace mode
	{
		QValueVector<Ufkt>::iterator it = &m_parser->ufkt[m_parser->ixValue(csmode)];
		int const ke=it->parameters.count();
		if (ke>0)
		{
			csparam++;
			if (csparam >= ke)
				csparam=0;
		}
		if (csparam==0)
		{
			int const old_csmode=csmode;
			char const old_cstype = cstype;
			bool start = true;
			bool found = false;
			while ( 1 )
			{
				if ( old_csmode==csmode && !start)
				{
					cstype=old_cstype;
					break;
				}
				kdDebug() << "csmode: " << csmode << endl;
				switch(it->fstr[0].latin1())
				{
				case 'x':
				case 'y':
				case 'r':
					break;
				default:
					{
						for (cstype=0;cstype<3;cstype++) //going through the function, the first and the second derivative
						{
							if (start)
							{
								if ( cstype==2)
									cstype=0;
								else
									cstype=old_cstype+1;
								start=false;
							}
							kdDebug() << "   cstype: " << (int)cstype << endl;
							switch (cstype)
							{
							case (0):
											if (it->f_mode )
												found=true;
								break;
							case (1):
											if ( it->f1_mode )
												found=true;
								break;
							case (2):
											if ( it->f2_mode )
												found=true;
								break;
							}
							if (found)
								break;
						}
						break;
					}
				}
				if (found)
					break;

				if ( ++it == m_parser->ufkt.end())
					it = m_parser->ufkt.begin();
				csmode = it->id;
			}
		}

		kdDebug() << "************************" << endl;
		kdDebug() << "csmode: " << (int)csmode << endl;
		kdDebug() << "cstype: " << (int)cstype << endl;
		kdDebug() << "csparam: " << csparam << endl;

		//change function in the statusbar
		switch (cstype )
{
		case 0:
			setStatusBar(it->fstr,4);
			break;
		case 1:
			{
				QString function = it->fstr;
				function = function.left(function.find('(')) + '\'';
				setStatusBar(function,4);
				break;
			}
		case 2:
			{
				QString function = it->fstr;
				function = function.left(function.find('(')) + "\'\'";
				setStatusBar(function,4);
				break;
			}
		}
		event = new QMouseEvent(QEvent::MouseMove,QPoint(fcx,fcy),Qt::LeftButton,Qt::LeftButton);
	}
	else if ( e->key() == Qt::Key_Space  )
	{
		event = new QMouseEvent(QEvent::MouseButtonPress,QCursor::pos(),Qt::RightButton,Qt::RightButton);
		mousePressEvent(event);
		delete event;
		return;
	}
	else
	{
		event = new QMouseEvent(QEvent::MouseButtonPress,QPoint(fcx,fcy),Qt::LeftButton,Qt::LeftButton);
		mousePressEvent(event);
		delete event;
		return;
	}
	mouseMoveEvent(event);
	delete event;
}

void View::areaUnderGraph( Ufkt *ufkt, char const p_mode,  double &dmin, double &dmax, const QString &str_parameter, QPainter *DC )
{
	double x, y = 0;
	float calculated_area=0;
	int rectheight;
	areaMin = dmin;
	QPoint p;
	QColor color;
	switch(p_mode)
	{
	case 0:
		color = ufkt->color;
		break;
	case 1:
		color = ufkt->f1_color;
		break;
	case 2:
		color = ufkt->f2_color;
		break;
	case 3:
		color = ufkt->integral_color;
		break;
	}
	if ( DC == 0) //screen
	{
		int ly;
		buffer.fill(backgroundcolor);
		DC = new QPainter(&buffer);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC->scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
	}

	if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
	{
		dmin=xmin;
		dmax=xmax;
	}

	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QValueList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression == str_parameter)
			{
				ufkt->setParameter((*it).value);
				break;
			}
		}
	}
	double dx;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		if ( ufkt->integral_use_precision )
			dx = ufkt->integral_precision*(dmax-dmin)/area.width();
		else
			dx = stepWidth*(dmax-dmin)/area.width();
		startProgressBar((int)double((dmax-dmin)/dx)/2);
		x = ufkt->oldx = ufkt->startx; //the initial x-point
		ufkt->oldy = ufkt->starty;
		ufkt->oldyprim = ufkt->integral_precision;
		//paintEvent(0);

		/*QPainter p;
		p.begin(this);
		bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
		p.end();*/
	}
	else
	{
		dx = stepWidth*(dmax-dmin)/area.width();
		x=dmin;
	}


	int const origoy = dgr.Transy(0.0);
	int const rectwidth = dgr.Transx(dx)- dgr.Transx(0.0)+1;

	setCursor(Qt::WaitCursor );
	isDrawing=true;

	bool forward_direction;
	if (dmin<0 && dmax<0)
		forward_direction = false;
	else
		forward_direction = true;
	
	int intervals = qRound((dmax-dmin)/dx);
	int at = 0;
	
	while ((at<=intervals) ||  (p_mode == 3 && x>=dmin && !forward_direction) || (p_mode == 3 && x<=dmax && forward_direction))
	{
		if ( p_mode != 3 )
			x = (intervals-at)*dmin + at*dmax;
		++at;
		
		if ( p_mode == 3 && stop_calculating)
		{
			if (forward_direction)
				x=dmin-1;
			else
				x=dmax+1;
			break;
			continue;
		}
		switch(p_mode)
		{
			case 0:
				y=m_parser->fkt( ufkt, x);
				break;
	
			case 1:
				y=m_parser->a1fkt( ufkt, x);
				break;
			case 2:
				y=m_parser->a2fkt( ufkt, x);
				break;
			case 3:
			{
				y = m_parser->euler_method(x, ufkt);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
						increaseProgressBar();
				}
				break;
			}
		}

		p.setX(dgr.Transx(x));
		p.setY(dgr.Transy(y));
		if (dmin<=x && x<=dmax)
		{
			if( dgr.xclipflg || dgr.yclipflg ) //out of bounds
			{
				if (y>-10e10 && y<10e10)
				{
					if ( y<0)
						rectheight = origoy-p.y() ;
					else
						rectheight= -1*( p.y()-origoy);	
					calculated_area = calculated_area + ( dx*y);
					DC->fillRect(p.x(),p.y(),rectwidth,rectheight,color);
				}
			}
			else
			{
				if ( y<0)
					rectheight =  origoy-p.y();
				else
					rectheight = -1*( p.y()-origoy);
				
				calculated_area = calculated_area + (dx*y);
				/*kdDebug() << "Area: " << area << endl;
				kdDebug() << "x:" << p.height() << endl;
				kdDebug() << "y:" << p.y() << endl;
				kdDebug() << "*************" << endl;*/
				
				DC->fillRect(p.x(),p.y(),rectwidth,rectheight,color);
			}
		}

		if (p_mode==3)
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode== 3)
				{
					forward_direction = false;
					x = ufkt->oldx = ufkt->startx;
					ufkt->oldy = ufkt->starty;
					ufkt->oldyprim = ufkt->integral_precision;
					paintEvent(0);
				}
			}
			else
				x=x-dx; // go backwards
		}
		else
			x=x+dx;
	}
	if ( stopProgressBar() )
	{
		if( stop_calculating)
		{
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
			isDrawing=false;
			restoreCursor();
			return;
		}
	}
	isDrawing=false;
	restoreCursor();


	areaUfkt = ufkt;
	areaPMode = p_mode;
	areaMax = dmax;
	areaParameter = str_parameter;

	if ( DC->device() == &buffer) //draw the graphs to the screen
	{
		areaDraw=true;
		DC->end();
		setFocus();
		update();
		draw(&buffer,0);
	}

	if ( calculated_area>0)
		dmin = int(calculated_area*1000)/double(1000);
	else
		dmin = int(calculated_area*1000)/double(1000)*-1; //don't answer with a negative number
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
	for( int number = 0; number < SLIDER_COUNT; number++)
	{
		if (sliders[ number ])
		{
			sliders[ number ]->hide();
			mnuSliders[ number ]->setChecked(false); //uncheck the slider-item in the menu
		}
	}

	for(QValueVector<Ufkt>::iterator it=m_parser->ufkt.begin(); it!=m_parser->ufkt.end(); ++it)
	{
		if (it->fname.isEmpty() ) continue;
		if( it->use_slider > -1  &&  (it->f_mode || it->f1_mode || it->f2_mode || it->integral_mode))
		{
			// create the slider if it not exists already
			if ( sliders[ it->use_slider ] == 0 )
			{
				sliders[ it->use_slider ] = new KSliderWindow( this, it->use_slider);
				connect( sliders[ it->use_slider ]->slider, SIGNAL( valueChanged( int ) ), this, SLOT( drawPlot() ) );
				connect( sliders[ it->use_slider ], SIGNAL( windowClosed( int ) ), this , SLOT( sliderWindowClosed(int) ) );
				mnuSliders[ it->use_slider ]->setChecked(true);  //set the slider-item in the menu
			}
			sliders[ it->use_slider ]->show();
		}
	}
}

void View::mnuHide_clicked()
{
    if ( csmode == -1 )
      return;
    
	Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(csmode)];
	switch (cstype )
	{
	case 0:
		ufkt->f_mode=0;
		break;
	case 1:
		ufkt->f1_mode=0;
		break;
	case 2:
		ufkt->f2_mode=0;
		break;
	}
	drawPlot();
	m_modified = true;
	updateSliders();
	if (csmode==-1)
		return;
	if ( !ufkt->f_mode && !ufkt->f1_mode && !ufkt->f2_mode) //all graphs for the function are hidden
	{
		csmode=-1;
		QMouseEvent *event = new QMouseEvent(QMouseEvent::KeyPress,QCursor::pos(),Qt::LeftButton,Qt::LeftButton);
		mousePressEvent(event); //leave trace mode
		delete event;
		return;
	}
	else
	{
		QKeyEvent *event = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_Up ,Qt::Key_Up ,0);
		keyPressEvent(event); //change selected graph
		delete event;
		return;
	}
}
void View::mnuRemove_clicked()
{
  if ( csmode == -1 )
    return;
  
	if ( KMessageBox::warningContinueCancel(this,i18n("Are you sure you want to remove this function?"), QString::null, KStdGuiItem::del()) == KMessageBox::Continue )
	{
		Ufkt *ufkt =  &m_parser->ufkt[m_parser->ixValue(csmode)];
		char const function_type = ufkt->fstr[0].latin1();
		if (!m_parser->delfkt( ufkt ))
		  return;

		if (csmode!=-1) // if trace mode is enabled
		{
		  csmode=-1;
		  QMouseEvent *event = new QMouseEvent(QMouseEvent::KeyPress,QCursor::pos(),Qt::LeftButton,Qt::LeftButton);
		  mousePressEvent(event); //leave trace mode
		  delete event;
		}
		
		drawPlot();
		if ( function_type != 'x' &&  function_type != 'y' && function_type != 'r' )
			updateSliders();
		m_modified = true;
	}
}
void View::mnuEdit_clicked()
{
  if ( csmode == -1 )
    return;
	if ( m_parser->ufkt[m_parser->ixValue(csmode)].fstr[0] == 'x') // a parametric function
	{
		int y_index = csmode+1; //the y-function
		if ( y_index == (int)m_parser->countFunctions())
			y_index=0;
		KEditParametric* editParametric = new KEditParametric( m_parser, this );
		editParametric->setCaption(i18n( "New Parametric Plot" ));
		editParametric->initDialog( csmode,y_index );
		if( editParametric->exec() == QDialog::Accepted )
		{
			drawPlot();
			m_modified = true;
		}

	}
	else // a plot function
	{
		EditFunction* editFunction = new EditFunction( m_parser, this );
		editFunction->setCaption(i18n( "Edit Function Plot" ));
		editFunction->initDialog( csmode );
		if( editFunction->exec() == QDialog::Accepted )
		{
			drawPlot();
			updateSliders();
			m_modified = true;
		}
	}
}

void View::mnuCopy_clicked()
{
  if ( csmode == -1 )
  return;
  
	if ( m_parser->sendFunction(csmode) )
		m_modified = true;
}

void View::mnuMove_clicked()
{
  if ( csmode == -1 )
    return;
  
	if ( m_parser->sendFunction(csmode) )
	{
	  	if (!m_parser->delfkt(csmode) )
		  return;
		drawPlot();
		m_modified = true;
	}
}

void View::mnuNoZoom_clicked()
{
	setCursor(Qt::ArrowCursor);
	zoom_mode = Z_Normal;
}

void View::mnuRectangular_clicked()
{
  if ( zoom_mode == Z_Rectangular )
    resetZoom();
  else
  {
    setCursor(Qt::CrossCursor);
    zoom_mode = Z_Rectangular;
  }
}
void View::mnuZoomIn_clicked()
{
    if ( zoom_mode == Z_ZoomIn )
      resetZoom();
    else
    {
	   setCursor( QCursor( SmallIcon( "magnify", 32), 10, 10 ) );
	   zoom_mode = Z_ZoomIn;
    }
}

void View::mnuZoomOut_clicked()
{
  if ( zoom_mode == Z_ZoomOut )
    resetZoom();
  else
  {
	setCursor( QCursor( SmallIcon( "lessen", 32), 10, 10 ) );
	zoom_mode = Z_ZoomOut;
  }
}
void View::mnuCenter_clicked()
{
  if ( zoom_mode == Z_Center )
    resetZoom();
  else
  {
	setCursor(Qt::PointingHandCursor);
	zoom_mode = Z_Center;
  }
}
void View::mnuTrig_clicked()
{
	if ( Settings::anglemode()==0 ) //radians
	{
	  Settings::setXMin("-(47/24)pi");
	  Settings::setXMax("(47/24)pi");
	}
	else //degrees
	{
		Settings::setXMin("-352.5" );
		Settings::setXMax("352.5" );
	}
	Settings::setYMin("-4");
	Settings::setYMax("4");

	Settings::setXRange(4); //custom x-range
	Settings::setYRange(4); //custom y-range
	drawPlot(); //update all graphs
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
void View::restoreCursor()
{
	switch (zoom_mode)
	{
		case Z_Normal:  //no zoom
			setCursor(Qt::ArrowCursor);
			break;
		case Z_Rectangular: //rectangle zoom
			setCursor(Qt::CrossCursor);
			break;
		case Z_ZoomIn: //zoom in
			setCursor( QCursor( SmallIcon( "magnify", 32), 10, 10 ) );
			break;
		case Z_ZoomOut: //zoom out
			setCursor( QCursor( SmallIcon( "lessen", 32), 10, 10 ) );
			break;
		case Z_Center: //center a point
			setCursor(Qt::PointingHandCursor);
			break;
	}
}

bool View::event( QEvent * e )
{
	if ( e->type() == QEvent::WindowDeactivate && isDrawing)
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
		QByteArray parameters;
		QDataStream arg( parameters, IO_WriteOnly);
		arg << text << id;
		m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","setStatusBarText(QString,int)", parameters);
	}
}
void View::startProgressBar(int steps)
{
	QByteArray data;
	QDataStream stream(data, IO_WriteOnly);
	stream << steps;
	m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","startProgressBar(int)", data);
}
bool View::stopProgressBar()
{
	QCString replyType;
	QByteArray replyData;
	m_dcop_client->call(m_dcop_client->appId(), "KmPlotShell","stopProgressBar()", QByteArray(), replyType, replyData);
	bool result;
	QDataStream stream(replyData, IO_ReadOnly);
	stream >> result;
	return result;
}
void View::increaseProgressBar()
{
	m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","increaseProgressBar()", QByteArray());
}

void View::sliderWindowClosed(int num)
{
	mnuSliders[num]->setChecked(false);
}
