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

// Qt includes
#include <qpicture.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h> 
#include <kprogress.h> 

// local includes
#include "editfunction.h"
#include "kminmax.h"
#include "settings.h"
#include "sliderwindow.h"
#include "View.h"
#include "View.moc"


//minimum and maximum x range. Should always be accessible.
double View::xmin = 0;
double View::xmax = 0;

KmPlotProgress::KmPlotProgress( QWidget* parent, const char* name ) : QWidget( parent, name) 
{
	button = new KPushButton(this);
	button->setPixmap( SmallIcon( "cancel" ) );
	button->setGeometry( QRect( 0, 0, 30, 27 ) );
	button->setMaximumHeight(height()-10);
	
	progress = new KProgress(this);
	progress->setGeometry( QRect( 30, 0, 124, 26 ) );
	progress->setMaximumHeight(height()-10);
	
	hide();
	setMinimumWidth(154);
}

KmPlotProgress::~KmPlotProgress()
{
}


void KmPlotProgress::increase()
{
	progress->setProgress( progress->progress()+1);
}


/*
 * View implementation
 */

View::View(KPopupMenu *m, QWidget* parent, const char* name ) : QWidget( parent, name , WStaticContents ), buffer( width(), height() )
{   
	m_parser = new XParser( 10, 200, 20 );

	init();
	csflg=0;
	csmode=-1;
	backgroundcolor = Settings::backgroundcolor();
	setBackgroundColor(backgroundcolor);
	setMouseTracking(TRUE);
	areaDraw = false;
	for( int number = 0; number < SLIDER_COUNT; number++ )
	{
		sliders[ number ] = new SliderWindow( this, QString( "slider%1" ).arg( number ).latin1(), false, Qt::WStyle_Tool-Qt::WStyle_Maximize );
		sliders[ number ]->setCaption( i18n( "Slider %1" ).arg( number ) );
		connect( sliders[ number ]->slider, SIGNAL( valueChanged( int ) ), this, SLOT( drawPlot() ) );
		QWhatsThis::add( sliders[ number ]->slider, i18n( "Move slider to change the parameter of the function plot connected to this slider." ) );
		QToolTip::add( sliders[ number ]->slider, i18n( "Slider no. %1" ).arg( number ) );
	}
	updateSliders();
	m_popupmenu = m;
	m_popupmenushown = 0;
	m_popupmenu->insertTitle( "",10);
	zoom_mode = 0;
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
	
	if ( form!=0 && areaDraw)
		areaUnderGraph(areaIx, areaPMode, areaMin,areaMax, areaParameter, &DC);
	
	dgr.Plot(&DC);
	PlotArea=dgr.GetPlotArea();
	area=DC.xForm(PlotArea);
	hline.resize(area.width(), 1);
	vline.resize(1, area.height());
	stepWidth=Settings::relativeStepWidth() * (xmax-xmin) / area.width();
	
	isDrawing=true;
	stop_calculating = false;
	for(ix=0; ix<m_parser->ufanz && !stop_calculating; ++ix)
	{
		if(m_parser->chkfix(ix)==-1) continue;
		plotfkt(ix, &DC);
	}
	isDrawing=false;
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
		{   
			dmin=0.;
			dmax=2*M_PI;
		}
		else
		{
			dmin=xmin;
			dmax=xmax;
		}
	}
	
	if(fktmode=='r') 
		dx=Settings::relativeStepWidth()*0.05/(dmax-dmin);
	else if(fktmode=='x')
	{   
		m_parser->getfkt(ix, fname, fstr);
		fname[0]='y';
		iy=m_parser->getfix(fname);
		if(iy==-1) 
			return ;
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
			if ( p_mode == 3 && stop_calculating)
				break;
			if( m_parser->fktext[ ix ].use_slider == -1 )
				m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[k]);
			else
				m_parser->setparameter(ix, sliders[ m_parser->fktext[ix].use_slider ]->slider->value() );
			mflg=2;
			bool forward_direction = true;
			if ( p_mode == 3)
			{
				if ( m_parser->fktext[ix].anti_use_precision )
					dx = (m_parser->fktext[ix].anti_precision)/1000;
				else
					dx=Settings::relativeStepWidth()/1000; //the stepwidth must be small for Euler's metod and not depend on the size on the mainwindow
				progressbar->progress->reset();
				progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
				progressbar->show();
				x = m_parser->fktext[ix].startx; //the initial x-point
			}
			else
				x=dmin;
			if ( p_mode != 0 || m_parser->fktext[ix].f_mode) // if not the function is hidden
			while (x>=dmin && x<=dmax)
			{
				
				if ( p_mode == 3 && stop_calculating)
				{
					x=dmax+1;
					continue;
				}
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
						y=m_parser->fkt(ix, x);
						m_parser->euler_method(x, y,ix);
						if ( int(x*100)%2==0)
						{
							KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated anti-derivative function
							progressbar->increase();
							paintEvent(0);
						}
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
					if(mflg<=1)
						pDC->drawLine(p1, p2);
					p1=p2;
                    			mflg=0;
				}
		    
		    		if (forward_direction)
		    		{
					x=x+dx;
					if (x>dmax && p_mode== 3)
					{
						forward_direction = false;
						x = m_parser->fktext[ix].startx;
						mflg=2;
					}
		    		}
		    		else
			    		x=x-dx; // go backwards

            		}
		}
        	while(++k<ke);
	
		if(m_parser->fktext[ix].f1_mode==1 && p_mode< 1) p_mode=1;
		else if(m_parser->fktext[ix].f2_mode==1 && p_mode< 2) p_mode=2;
		else if( m_parser->fktext[ix].anti_mode==1 && p_mode< 3) p_mode=3;
		else break;

		//do we need this?
		//pen=QPen(m_parser->fktext[ix].color, 50);
		//pDC->setPen(pen);
	}
	if (  progressbar->isVisible())
	{
		progressbar->hide(); // hide the progressbar-widget if it was shown
		if( stop_calculating)
		{
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
			stop_calculating=false;
		}
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
{   
	QPainter p;
	p.begin(this);
	bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
	p.end();
}

void View::resizeEvent(QResizeEvent *)
{
	buffer.resize(size() );
	drawPlot();
}

void View::drawPlot()
{
	if( m_minmax->isShown() )
		m_minmax->updateFunctions();
	buffer.fill(backgroundcolor);
	areaDraw = false;
	draw(&buffer, 0);
	QPainter p;
	p.begin(this);
	bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
	p.end();	
}

void View::mouseMoveEvent(QMouseEvent *e)
{   char sx[20], sy[20];
	
	if (zoom_mode==4 &&  e->stateAfter() != Qt::NoButton)
	{
		QPainter p;
		p.begin(this);
		bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
		p.end();
		
		QPainter painter(this);
		QPen pen;
		pen.setStyle(Qt::DotLine);
		painter.setPen(pen);
		painter.drawRect(rectangle_point.x(), rectangle_point.y(), e->pos().x()-rectangle_point.x(), e->pos().y()-rectangle_point.y());
		return;
		
	}

	if( m_popupmenushown>0 && !m_popupmenu->isShown() )
	{
		if ( m_popupmenushown==1)
			csmode=-1;
		m_popupmenushown = 0;
		return;
	}
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
	{
		if( m_parser->fktext[ csmode ].use_slider == -1 )
			m_parser->setparameter(csmode, m_parser->fktext[csmode].k_liste[csparam]);
		else
			m_parser->setparameter(csmode, sliders[ m_parser->fktext[csmode].use_slider ]->slider->value() );
		
		if ( cstype == 0)
			ptl.setY(dgr.Transy(csypos=m_parser->fkt(csmode, csxpos=dgr.Transx(ptl.x()))));
		else if ( cstype == 1)
			ptl.setY(dgr.Transy(csypos=m_parser->a1fkt(csmode, csxpos=dgr.Transx(ptl.x()))));
		else if ( cstype == 2)
			ptl.setY(dgr.Transy(csypos=m_parser->a2fkt(csmode, csxpos=dgr.Transx(ptl.x()))));

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
	if ( m_popupmenushown>0)
		return;
	
	if (  zoom_mode==1 ) //rectangle zoom
	{
		zoom_mode=4;
		rectangle_point = e->pos();
		return;
	}
	else if (  zoom_mode==2 ) //zoom in
	{
		QPainter DC;
		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		double real = dgr.Transx(DC.xFormDev(e->pos()).x());
		
		QString str_tmp;
		double diffx = (xmax-xmin)*(double)Settings::zoomStep()/200; // == 100*2=200
		double diffy = (ymax-ymin)*(double)Settings::zoomStep()/200;
		
		if ( diffx < 0.00001 || diffy < 0.00001)
			return;
			
		str_tmp.setNum(real-double(diffx));
		Settings::setXMin(str_tmp);
		str_tmp.setNum(real+double(diffx));
		Settings::setXMax(str_tmp);
		
		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		str_tmp.setNum(real-double(diffy));
		Settings::setYMin(str_tmp);
		str_tmp.setNum(real+double(diffy));
		Settings::setYMax(str_tmp);
		
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

		QString str_tmp;
		double diffx = (xmax-xmin)/((double)Settings::zoomStep()/50);// == 100/2=50
		double diffy = (ymax-ymin)/((double)Settings::zoomStep()/50);
			
		if ( diffx > 1000000 || diffy > 1000000)
			return;
		
		str_tmp.setNum(real-double(diffx));
		Settings::setXMin(str_tmp);
		str_tmp.setNum(real+double(diffx));
		Settings::setXMax(str_tmp);
		
		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		str_tmp.setNum(real-double(diffy));
		Settings::setYMin(str_tmp);
		str_tmp.setNum(real+double(diffy));
		Settings::setYMax(str_tmp);
		
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
		QString str_tmp;
		double diffx = (xmax-xmin)/2;
		double diffy = (ymax-ymin)/2;
		
		str_tmp.setNum(real-double(diffx));
		Settings::setXMin(str_tmp);
		str_tmp.setNum(real+double(diffx));
		Settings::setXMax(str_tmp);
		
		real = dgr.Transy(DC.xFormDev(e->pos()).y());
		str_tmp.setNum(real-double(diffy));
		Settings::setYMin(str_tmp);
		str_tmp.setNum(real+double(diffy));
		Settings::setYMax(str_tmp);
		
		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
		return;
		
	}
	if (!stop_calculating && isDrawing) //stop drawing anti-derivatives
	{
		stop_calculating = true;
		return;
	}
	
	if(e->button()==RightButton) //clicking with the right mouse button
	{
		g=tlgy/5.;
		for(ix=0; ix<m_parser->ufanz; ++ix)
		{
			switch(m_parser->fktext[ix].extstr[0].latin1())
			{
				case 0: case 'x': case 'y': case 'r': continue;   // Not possible to catch
			}
		
			k=0;
			ke=m_parser->fktext[ix].k_anz;
			do
			{
				if( m_parser->fktext[ ix ].use_slider == -1 )
					m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[k]);
				else
					m_parser->setparameter(ix, sliders[ m_parser->fktext[ix].use_slider ]->slider->value() );
				if(fabs(csypos-m_parser->fkt(ix, csxpos))< g && m_parser->fktext[ix].f_mode)
				{
					if ( csmode == -1)
					{
						csmode=ix;
						cstype=0;
						csparam = k;
						m_popupmenu->changeTitle(10, m_parser->fktext[ ix ].extstr);
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				if(fabs(csypos-m_parser->a1fkt(ix, csxpos))< g && m_parser->fktext[ix].f1_mode)
				{
					if ( csmode == -1)
					{
						csmode=ix;
						cstype=1;
						csparam = k;
						QString function = m_parser->fktext[ ix ].extstr;
						function = function.left(function.find('(')) + '\'';
						m_popupmenu->changeTitle(10, function);
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				if(fabs(csypos-m_parser->a2fkt(ix, csxpos))< g && m_parser->fktext[ix].f2_mode)
				{
					if ( csmode == -1)
					{
						csmode=ix;
						cstype=2;
						csparam = k;
						QString function = m_parser->fktext[ ix ].extstr;
						function = function.left(function.find('(')) + "\'\'";
						m_popupmenu->changeTitle(10, function);
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
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
		stbar->changeItem("",4);
		mouseMoveEvent(e);
		return ;
	}

	g=tlgy/5.;  
	for(ix=0; ix<m_parser->ufanz; ++ix)
	{
		switch(m_parser->fktext[ix].extstr[0].latin1())
		{
			case 0: case 'x': case 'y': case 'r': continue;   // Not possible to catch
		}
	
		k=0;
		ke=m_parser->fktext[ix].k_anz;
		do
		{
			if( m_parser->fktext[ ix ].use_slider == -1 )
				m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[k]);
			else
				m_parser->setparameter(ix, sliders[ m_parser->fktext[ix].use_slider ]->slider->value() );
			if(fabs(csypos-m_parser->fkt(ix, csxpos))< g && m_parser->fktext[ix].f_mode)
			{
				csmode=ix;
				cstype=0;
				csparam = k;
				m_minmax->selectItem();
				stbar->changeItem(m_parser->fktext[ ix ].extstr,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a1fkt(ix, csxpos))< g && m_parser->fktext[ix].f1_mode)
			{
				csmode=ix;
				cstype=1;
				csparam = k;
				m_minmax->selectItem();
				QString function = m_parser->fktext[ ix ].extstr;
				function = function.left(function.find('(')) + '\'';
				stbar->changeItem(function,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a2fkt(ix, csxpos))< g && m_parser->fktext[ix].f2_mode)
			{
				csmode=ix;
				cstype=2;
				csparam = k;
				m_minmax->selectItem();
				QString function = m_parser->fktext[ ix ].extstr;
				function = function.left(function.find('(')) + "\'\'";
				stbar->changeItem(function,4);
				mouseMoveEvent(e);
				return;
			}
		}
		while(++k<ke);
	}

	csmode=-1;
	stbar->changeItem("",4);
}


void View::mouseReleaseEvent ( QMouseEvent * e )
{
	if ( zoom_mode==4)
	{
		zoom_mode=1;
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
		
		QString str_tmp;
		
		
		
		if( real1x < real2x  )
		{
			if( real2x - real1x < 0.00001)
			    return;
			str_tmp.setNum(real1x );
			Settings::setXMin(str_tmp );
			str_tmp.setNum(real2x );
			Settings::setXMax(str_tmp );
		}
		else
		{
			if (real1x - real2x < 0.00001)
				return;
			str_tmp.setNum(real2x );
			Settings::setXMin(str_tmp );
			str_tmp.setNum(real1x );
			Settings::setXMax(str_tmp );
		}
		
		if( real1y < real2y )
		{
			if( real2y - real1y < 0.00001)
				return;
			str_tmp.setNum(real1y );
			Settings::setYMin(str_tmp );
			str_tmp.setNum(real2y);
			Settings::setYMax(str_tmp );
		}
		else
		{
			if( real1y - real2y < 0.00001)
				return;
			str_tmp.setNum(real2y  );
			Settings::setYMin(str_tmp );
			str_tmp.setNum(real1y );
			Settings::setYMax(str_tmp );
		}
		Settings::setXRange(4); //custom x-range
		Settings::setYRange(4); //custom y-range
		drawPlot(); //update all graphs
	}
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
	const char* units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4","automatic" };
	
	if( Settings::xScaling() == 8) //automatic x-scaling
		tlgx = double(xmax-xmin)/16;
	else
	{
		tlgxstr = units[ Settings::xScaling() ];
		tlgx = m_parser->eval( tlgxstr );
	}
	
	if( Settings::yScaling() == 8)  //automatic y-scaling
		tlgy = double(ymax-ymin)/16;
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
	
	for (int i=0;i<10;i++)
	{
		m_parser->fktext[i].f1_color = m_parser->fktext[i].color;
		m_parser->fktext[i].f2_color = m_parser->fktext[i].color;
		m_parser->fktext[i].anti_color = m_parser->fktext[i].color;
	}
	backgroundcolor = Settings::backgroundcolor();
}

void View::init()
{
	getSettings();

	for ( int ix = 0; ix < m_parser->ufanz; ++ix )
		m_parser->delfkt( ix );
}


void View::progressbar_clicked()
{
	stop_calculating = true;
}

void View::findMinMaxValue(int ix, char p_mode, bool minimum, double &dmin, double &dmax, QString &str_parameter)
{
	double dx, x, y, result_x, result_y;
	bool start = false;
	if(ix==-1 || ix>=m_parser->ufanz) return ;	    // ungltiger Index

	dx = stepWidth;

	// TODO: parameter sliders
	int i=0;
	if ( m_parser->fktext[ix].k_anz != 0)
		for ( QStringList::Iterator it = m_parser->fktext[ix].str_parameter.begin(); it != m_parser->fktext[ix].str_parameter.end(); ++it )
		{
			if ( *it == str_parameter)
			{
				m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[i]);
				break;
			}
			i++;
		}

	bool forward_direction = true;
	isDrawing=true;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		progressbar->progress->reset();
		if ( m_parser->fktext[ix].anti_use_precision )
			dx = (m_parser->fktext[ix].anti_precision)/1000;
		else
			dx=Settings::relativeStepWidth()/1000; //the stepwidth must be small for Euler's metod and not depend on the size on the mainwindow
		progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
		progressbar->show();
		x = m_parser->fktext[ix].startx; //the initial x-point
	}
	else
		x=dmin;
	while (x>=dmin && x<=dmax)
	{
		if ( p_mode == 3 && stop_calculating)
		{
			x=dmax+1;
			continue;
		}
		errno=0;

		switch(p_mode)
		{
			case 0: 
				y=m_parser->fkt(ix, x);
				break;
			
			case 1:
			{
				y=m_parser->a1fkt(ix, x);
				break;
			}
			case 2:
			{
				y=m_parser->a2fkt(ix, x);
				break;
			}
			case 3:
			{
				y=m_parser->fkt(ix, x);
				m_parser->euler_method(x, y,ix);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated anti-derivative function
					progressbar->increase();
					paintEvent(0);
				}
				break;
			}
		}
		if(errno!=0) continue;
		
		if ( y>=ymin &&y<=ymax)
		{
			if ( !start)
			{
				result_x = x;
				result_y = y;
				start=true;	
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
	
		if (forward_direction)
		{
			x=x+dx;
			if (x>dmax && p_mode== 3)
			{
				forward_direction = false;
				x = m_parser->fktext[ix].startx;
			}
		}
		else
			x=x-dx; // go backwards
	}
	if (  progressbar->isVisible())
		progressbar->hide(); // hide the progressbar-widget if it was shown
	isDrawing=false;
	
	dmin = int(result_x*1000)/double(1000);
	dmax = int(result_y*1000)/double(1000);
}

void View::getYValue(int ix, char p_mode,  double x, double &y, QString &str_parameter)
{	
	int i=0;
	// TODO: parameter sliders
	if ( m_parser->fktext[ix].k_anz != 0)
		for ( QStringList::Iterator it = m_parser->fktext[ix].str_parameter.begin(); it != m_parser->fktext[ix].str_parameter.end(); ++it )
	{
		if ( *it == str_parameter)
		{
			m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[i]);
			break;
		}
		i++;
	}
	
	switch (p_mode)
	{
		case 0:
			y= m_parser->fkt(ix, x);
			break;
		case 1:
			y=m_parser->a1fkt(ix, x);
			break;
		case 2:
			y=m_parser->a2fkt(ix, x);
			break;
		case 3:
			
			if(ix==-1 || ix>=m_parser->ufanz) return;  // ungltiger Index
		
			double dmin = m_parser->fktext[ix].dmin;
			double dmax = m_parser->fktext[ix].dmax;
			double dx = stepWidth;
			bool forward_direction = true;
			const double target = x; //this is the x-value the user had chosen
			
			if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
			{   
				dmin=xmin;
				dmax=xmax;
			}
			
			stop_calculating = false;
			isDrawing=true;
			bool target_found=false;
			if ( m_parser->fktext[ix].anti_use_precision )
				dx = (m_parser->fktext[ix].anti_precision)/1000;
			else
				dx=Settings::relativeStepWidth()/1000; //the stepwidth must be small for Euler's metod and not depend on the size on the mainwindow
			progressbar->progress->reset();
			progressbar->progress->setTotalSteps ((int) double((dmax-dmin)/dx)/2 );
			progressbar->show();
			x = m_parser->fktext[ix].startx; //the initial x-point
			while (x>=dmin && x<=dmax  && !stop_calculating && !target_found)
			{
				y=m_parser->fkt(ix, x);
				m_parser->euler_method(x, y,ix);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated anti-derivative function
					progressbar->increase();
					paintEvent(0);
				}
				
				if(errno!=0) continue;

				if ( x+dx > target) //right x-value is found
					target_found = true;

				if (forward_direction)
				{
					x=x+dx;
					if (x>dmax)
					{
						forward_direction = false;
						x = m_parser->fktext[ix].startx;
					}
				}
				else
					x=x-dx; // go backwards
			}
			if (  progressbar->isVisible())
				progressbar->hide(); // hide the progressbar-widget if it was shown

			isDrawing=false;
			break;
	}
}

void View::keyPressEvent( QKeyEvent * e)
{
	if ( zoom_mode == 4) //drawing a rectangle
	{	
		zoom_mode = 1;
		update();
		return;
	}
	
	/*if ( e->key() == Qt::Key_A) //disable zoom mode
		zoom_mode=0;
	else if ( e->key() == Qt::Key_S) //rectangular zoom mode
		zoom_mode=1;
	else if ( e->key() == Qt::Key_D) //zoom in mode
		zoom_mode=2;
	else if ( e->key() == Qt::Key_F) //zoom out mode
		zoom_mode=3;
	else if ( e->key() == Qt::Key_G) //center mode
		zoom_mode=5;
	else if ( e->key() == Qt::Key_H) //adapt the widget for trigonometry
	{
		if ( Settings::anglemode()==0 ) //radians
		{
			Settings::setXMin("-6.152285613" );
			Settings::setXMax("6.152285613" );
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
		return;
}*/
		
	
	if (csmode==-1 ) return;
	
	QMouseEvent *event;
	if (e->key() == Qt::Key_Left )
		event = new QMouseEvent(QEvent::MouseMove,QPoint(fcx-1,fcy-1),Qt::LeftButton,Qt::LeftButton);
	else if (e->key() == Qt::Key_Right )
		event = new QMouseEvent(QEvent::MouseMove,QPoint(fcx+1,fcy+1),Qt::LeftButton,Qt::LeftButton);
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
	{
		int ix=csmode;
		char mode = cstype;
		bool start=true;
		
		while ( csmode<m_parser->ufanz )
		{   
			if ( ix==csmode && mode==cstype)
				if (start) start=false;
			else break;
			switch(m_parser->fktext[ix].extstr[0].latin1())
			{  	case 0:
				case 'x':
				case 'y':
				case 'r':
				{
					if ( csmode == m_parser->ufanz-2)
						csmode=0;
					else
						csmode++;
					if ( m_parser->fktext[csmode].f_mode) break;
					continue;
				}
			}
			cstype++;
			
			if ( cstype == 1)
				if ( m_parser->fktext[csmode].f1_mode ) break;
			else cstype++;

			if ( cstype == 2 )
				if ( m_parser->fktext[csmode].f2_mode ) break;
			else cstype++;

			cstype=0;
			if ( csmode == m_parser->ufanz-2)
				csmode=0;
			else
				csmode++;
			if ( m_parser->fktext[csmode].f_mode) break;
		}
		
		/*kdDebug() << "csmode: " << (int)csmode << endl;
		kdDebug() << "cstype: " << (int)cstype << endl;*/
		
		int k=csparam;
		int const ke=m_parser->fktext[csmode].k_anz;
		do
		{
			if ( ++k >= ke)
				k=0;
			
			csparam = k;
			
			if (k==csparam)
				break;
		}
		while(1);
		
		switch (cstype )
		{
			case 0:
				stbar->changeItem(m_parser->fktext[csmode ].extstr,4);
				break;
			case 1:
			{
				QString function = m_parser->fktext[ csmode ].extstr;
				function = function.left(function.find('(')) + '\'';
				stbar->changeItem(function,4);
			}
				break;
			case 2:
			{
				QString function = m_parser->fktext[ csmode ].extstr;
				function = function.left(function.find('(')) + "\'\'";
				stbar->changeItem(function,4);
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

void View::areaUnderGraph(int ix, char p_mode,  double &dmin, double &dmax, QString &str_parameter, QPainter *DC)
{
	double dx, x, y;
	float area=0;
	int rectwidth, rectheight;
	QString fname, fstr;
	areaMin = dmin;
	QPoint p;
	int ly;
	QColor color;
	switch(p_mode)
	{
		case 0: 
			color = m_parser->fktext[ix].color;
			break;
		case 1:
			color = m_parser->fktext[ix].f1_color;
			break;
		case 2:
			color = m_parser->fktext[ix].f2_color;
			break;
		case 3:
			color = m_parser->fktext[ix].anti_color;
			break;
	}
	if ( DC == 0) //screen
	{
		buffer.fill(backgroundcolor);
		DC = new QPainter(&buffer);
		ly=(int)((ymax-ymin)*100.*drskaly/tlgy);
		DC->scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
	}

	if(ix==-1 || ix>=m_parser->ufanz) return ;	    // ungltiger Index

	dx=stepWidth;
	rectwidth = dgr.GetPlotArea().width()/200;
	if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
	{   
		dmin=xmin;
		dmax=xmax;
	}
	
	int i=0;
	// TODO: parameter sliders
	if ( m_parser->fktext[ix].k_anz != 0)
		for ( QStringList::Iterator it = m_parser->fktext[ix].str_parameter.begin(); it != m_parser->fktext[ix].str_parameter.end(); ++it )
	{
		if ( *it == str_parameter)
		{
			m_parser->setparameter(ix, m_parser->fktext[ix].k_liste[i]);
			break;
		}
		i++;
	}
	
	bool forward_direction = true;
	isDrawing=true;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		if ( m_parser->fktext[ix].anti_use_precision )
			dx = (m_parser->fktext[ix].anti_precision)/1000;
		else
			dx=Settings::relativeStepWidth()/1000; //the stepwidth must be small for Euler's metod and not depend on the size on the mainwindow
		progressbar->progress->reset();
		progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
		progressbar->show();
		x = m_parser->fktext[ix].startx; //the initial x-point
	}
	else
		x=dmin;
	while (x>=dmin && x<=dmax)
	{
		if ( p_mode == 3 && stop_calculating)
		{
			x=dmax+1;
			continue;
		}
		errno=0;
		switch(p_mode)
		{
			case 0: 
				y=m_parser->fkt(ix, x);
				break;
			
			case 1:
			{
				y=m_parser->a1fkt(ix, x);
				break;
			}
			case 2:
			{
				y=m_parser->a2fkt(ix, x);
				break;
			}
			case 3:
			{
				y=m_parser->fkt(ix, x);
				m_parser->euler_method(x, y,ix);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated anti-derivative function
					progressbar->increase();
					paintEvent(0);
				}
				break;
			}
		}

		if(errno!=0) continue;	
		
		p.setX(dgr.Transx(x));
		p.setY(dgr.Transy(y));
		if(dgr.xclipflg || dgr.yclipflg)
		{
			if ( y<0)
			{
				//p.setY(dgr.Transy(ymin));
				rectheight = dgr.Transy(0.0)-p.y() ;
			}
			else
			{
				//p.setY(dgr.Transy(ymax));
				rectheight= -1*( p.y()-dgr.Transy(0.0)) ;
			}
			area = area + ( dx*y);
			DC->fillRect(p.x(),p.y(),rectwidth,rectheight,color);
		}
		else
		{
			if ( y<0)
			{
				rectheight =  dgr.Transy(0.0)-p.y();
			}
			else
			{
				rectheight = -1*( p.y()-dgr.Transy(0.0));
			}
			area = area + (dx*y);
			/*kdDebug() << "Area: " << area << endl;
			kdDebug() << "x:" << p.height() << endl;
			kdDebug() << "y:" << p.y() << endl;
			kdDebug() << "*************" << endl;*/
			DC->fillRect(p.x(),p.y(),rectwidth,rectheight,color);
		}
	
		if (forward_direction)
		{
			x=x+dx;
			if (x>dmax && p_mode== 3)
			{
				forward_direction = false;
				x = m_parser->fktext[ix].startx;
			}
		}
		else
			x=x-dx; // go backwards
	}
	if (  progressbar->isVisible())
	{
		progressbar->hide(); // hide the progressbar-widget if it was shown
		if( stop_calculating)
		{
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
			stop_calculating=false;
			return;
		}	
	}
	isDrawing=false;
	
	areaDraw=true;
	areaIx = ix;
	areaPMode = p_mode;
	areaMax = dmax;
	areaParameter = str_parameter;
	
	if ( DC->device() == &buffer) //draw the graphs to the screen
	{
		DC->end();
		setFocus();
		update();
		draw(&buffer,0);
	}
	
	if ( area>0)
		dmin = int(area*1000)/double(1000);
	else
		dmin = int(area*1000)/double(1000)*-1; //don't answer with a negative number
}

bool View::isCalculationStopped()
{
	if ( stop_calculating)
	{
		stop_calculating = true;
		return true;
	}
	else
		return false;
}

void View::updateSliders()
{
	for( int number = 0; number < SLIDER_COUNT; number++ ) sliders[ number ]->hide();
	for( int index = 0; index < m_parser->ufanz; index++ )
		if( m_parser->fktext[ index ].use_slider > -1 ) sliders[ m_parser->fktext[ index ].use_slider ]->show();
}

void View::mnuHide_clicked()
{
	switch (cstype )
	{
		case 0:
			m_parser->fktext[csmode ].f_mode=0;
			break;
		case 1:
			m_parser->fktext[csmode ].f1_mode=0;
			break;
		case 2:
			m_parser->fktext[csmode ].f2_mode=0;
			break;
	}
	drawPlot();
	if (csmode==-1)
		return;
	if ( !m_parser->fktext[csmode ].f_mode && !m_parser->fktext[csmode ].f1_mode && !m_parser->fktext[csmode ].f2_mode) //all graphs for the function are hidden
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
	if ( KMessageBox::questionYesNo(this,i18n("Are you sure you want to remove this function?")) == KMessageBox::Yes )
	{
		m_parser->delfkt( csmode );
		drawPlot();
	}
}
void View::mnuEdit_clicked()
{
	EditFunction* editFunction = new EditFunction( m_parser, this );
	editFunction->setCaption(i18n( "Edit Function Plot" ));
	editFunction->initDialog( csmode );
	if( editFunction->exec() == QDialog::Accepted )
	{
		drawPlot();
	}
}

void View::mnuRectangular_clicked()
{
	if ( zoom_mode == 1)
		zoom_mode = 0;
	else
		zoom_mode = 1;
}
void View::mnuZoomIn_clicked()
{
	if ( zoom_mode == 2)
		zoom_mode = 0;
	else
		zoom_mode = 2;	
}
void View::mnuZoomOut_clicked()
{
	if ( zoom_mode == 3)
		zoom_mode = 0;
	else
		zoom_mode = 3;
}
void View::mnuCenter_clicked()
{
	if ( zoom_mode == 5)
		zoom_mode = 0;
	else
		zoom_mode = 5;	
}
void View::mnuTrig_clicked()
{
	if ( Settings::anglemode()==0 ) //radians
	{
		Settings::setXMin("-6.152285613" );
		Settings::setXMax("6.152285613" );
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
	return;
}