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
#include <qbitmap.h>
#include <qcursor.h>
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

View::View(bool & mo, KPopupMenu *p, QWidget* parent, const char* name ) : QWidget( parent, name , WStaticContents ), buffer( width(), height() ), m_popupmenu(p), m_modified(mo)
{   
	m_parser = new XParser(MEMSIZE, STACKSIZE );
	init();
	csflg=0;
	csmode=-1;
	backgroundcolor = Settings::backgroundcolor();
	invertColor(backgroundcolor,inverted_backgroundcolor);
	setBackgroundColor(backgroundcolor);
	setMouseTracking(TRUE);
	for( int number = 0; number < SLIDER_COUNT; number++ )
	{
		sliders[ number ] = new SliderWindow( this, QString( "slider%1" ).arg( number ).latin1(), false, Qt::WStyle_Tool-Qt::WStyle_Maximize );
		sliders[ number ]->setCaption( i18n( "Slider %1" ).arg( number+1 ) );
		connect( sliders[ number ]->slider, SIGNAL( valueChanged( int ) ), this, SLOT( drawPlot() ) );
		QWhatsThis::add( sliders[ number ]->slider, i18n( "Move slider to change the parameter of the function plot connected to this slider." ) );
		QToolTip::add( sliders[ number ]->slider, i18n( "Slider no. %1" ).arg( number ) );
	}
	updateSliders();
	m_popupmenushown = 0;
	m_popupmenu->insertTitle( "",10);
	zoom_mode = 0;
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
{	int lx, ly;
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
		if ( ( (KPrinter* )dev )->option( "app-kmplot-printbackground" ) == "-1" )
			DC.fillRect( dgr.GetFrame(),  backgroundcolor); //draw a colored background
		//DC.end();
		//((QPixmap *)dev)->fill(QColor("#FF00FF"));
		//DC.begin(dev);
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
		areaUnderGraph(areaFktext, areaPMode, areaMin,areaMax, areaParameter, &DC);
		areaDraw = false;
		if (stop_calculating)
			return;
	}
	
	dgr.Plot(&DC);
	PlotArea=dgr.GetPlotArea();
	area=DC.xForm(PlotArea);
	hline.resize(area.width(), 1);
	vline.resize(1, area.height());
	stepWidth=Settings::relativeStepWidth();
	
	isDrawing=true;
	setCursor(Qt::WaitCursor );
	stop_calculating = false;
        
        //for(uint ix=0; ix<m_parser->fktext.count() && !stop_calculating; ++ix)
        for(QValueVector<XParser::FktExt>::iterator fktext=m_parser->fktext.begin(); fktext!=m_parser->fktext.end() && !stop_calculating; ++fktext)
		plotfkt(fktext, &DC);

	isDrawing=false;
	restoreCursor();
	csflg=0;
	DC.end();			// painting done
}


void View::plotfkt(QValueVector<XParser::FktExt>::iterator fktext, QPainter *pDC)
{	
	char p_mode;
	int iy, k, ke, mflg;
	double x, y, dmin, dmax;
	QPoint p1, p2;
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);

        Parser::Ufkt *ufkt = &m_parser->ufkt[m_parser->ixValue(fktext->id)];
        
	char const fktmode=fktext->extstr[0].latin1();
	if(fktmode=='y') return ;
	
	dmin=fktext->dmin;
	dmax=fktext->dmax;
        
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
	double dx;
	if(fktmode=='r') 
		dx=stepWidth*0.05/(dmax-dmin);
	else
		dx=stepWidth*(dmax-dmin)/area.width();
	
	if(fktmode=='x')
                iy = m_parser->ixValue(fktext->id)+1;
	p_mode=0;
	pen.setWidth((int)(fktext->linewidth*s) );
	pen.setColor(fktext->color);
	pDC->setPen(pen);
        
        
	while(1)
	{
                
		k=0;
		ke=fktext->k_liste.count();
		do
		{
                        kdDebug() << "drawing " << ufkt->id << endl;
                        
			if ( p_mode == 3 && stop_calculating)
				break;
			if( fktext->use_slider == -1 )
                        {
                                if ( !fktext->k_liste.isEmpty() )
				    ufkt->setParameter( fktext->k_liste[k] );
                        }
			else
				ufkt->setParameter( sliders[ fktext->use_slider ]->slider->value() );
     
			mflg=2;
			if ( p_mode == 3)
			{
				if ( fktext->integral_use_precision )
					dx =  fktext->integral_precision*(dmax-dmin)/(area.width()*10);
				else

					dx=dx/10;
				progressbar->progress->reset();
				progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
				progressbar->show();
				x = fktext->startx; //the initial x-point
			}
			else
				x=dmin;
			bool forward_direction;

			if (dmin<0 && dmax<0)
				forward_direction = false;
			else
				forward_direction = true;
                                
			if ( p_mode != 0 || fktext->f_mode) // if not the function is hidden
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
						y=m_parser->fkt(ufkt, x);
						m_parser->euler_method(x, y,ufkt, fktext);
						if ( int(x*100)%2==0)
						{
							KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
							progressbar->increase();
							paintEvent(0);
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
					if(mflg>=1)
						p1=p2;
					else
					{
						pDC->drawLine(p1, p2);
						p1=p2;
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

				if (p_mode==3)
				{
					if ( forward_direction)
					{
						x=x+dx;
						if (x>dmax && p_mode== 3)
						{
							forward_direction = false;
							x = fktext->startx;
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
       
		if(fktext->f1_mode==1 && p_mode< 1) //draw the 1st derivative
		{
			p_mode=1;
			pen.setWidth((int)(fktext->f1_linewidth*s) );
			pen.setColor(fktext->f1_color);
			pDC->setPen(pen);
		}
		else if(fktext->f2_mode==1 && p_mode< 2) //draw the 2nd derivative
		{
			p_mode=2;
			pen.setWidth((int)(fktext->f2_linewidth*s) );
			pen.setColor(fktext->f2_color);
			pDC->setPen(pen);
		}
		else if( fktext->integral_mode==1 && p_mode< 3) //draw the integral
		{
			p_mode=3;
			pen.setWidth((int)(fktext->integral_linewidth*s) );
			pen.setColor(fktext->integral_color);
			pDC->setPen(pen);
		}
		else break; //otherwise stop
	}

	if (  progressbar->isVisible())
	{
		progressbar->hide(); // hide the progressbar-widget if it was shown
		if( stop_calculating)
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
	}
	
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
                for(uint ix=0; ix<m_parser->fktext.count() && !stop_calculating; ++ix)
		{
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
{
        double x, y, yn, dx;
        if(rootflg==1)
                return FALSE;
        x=csxpos;
        y=fabs(csypos);
        dx=0.1;
        int const ix = m_parser->ixValue(csmode);
        while(1)
        {
                if((yn=fabs(m_parser->fkt(ix, x-dx))) < y)
                {       x-=dx;
                        y=yn;
                }
                else if((yn=fabs(m_parser->fkt(ix, x+dx))) < y)
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
                        return TRUE;
                }
                if(fabs(dx)<1e-8)
                        return FALSE;
                if(x<xmin || x>xmax)
                        return FALSE;
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
	{	bitBlt(this, area.left(), fcy, &hline, 0, 0, area.width(), 1);
		bitBlt(this, fcx, area.top(), &vline, 0, 0, 1, area.height());
		csflg=0;
	}
	char sx[20], sy[20];
	if(area.contains(e->pos()))
	{
		QPoint ptd, ptl;
		QPainter DC;

		DC.begin(this);
		DC.setWindow(0, 0, w, h);
		DC.setWorldMatrix(wm);
		ptl=DC.xFormDev(e->pos());
                
                QValueVector<Parser::Ufkt>::iterator itu = 0;
                QValueVector<XParser::FktExt>::iterator itf = 0;
                
		if( csmode >= 0 && csmode <= (int)m_parser->fktext.count() )
		{
                        int const ix = m_parser->ixValue(csmode);
                        if (ix!=-1)
                        {
                                itu = &m_parser->ufkt[ix];
                                itf = &m_parser->fktext[ix];
                                if( m_parser->fktext[ csmode ].use_slider == -1 )
                                {
                                        if( !m_parser->fktext[csmode].k_liste.isEmpty() )
			                     m_parser->ufkt[csmode].setParameter(m_parser->fktext[csmode].k_liste[csparam]);
                                }
                                else
                                        m_parser->ufkt[csmode].setParameter(sliders[ m_parser->fktext[csmode].use_slider ]->slider->value() );
                                if ( cstype == 0)
                                        ptl.setY(dgr.Transy(csypos=m_parser->fkt(csmode, csxpos=dgr.Transx(ptl.x()))));
                                else if ( cstype == 1)
                                        ptl.setY(dgr.Transy(csypos=m_parser->a1fkt( &m_parser->ufkt[csmode], csxpos=dgr.Transx(ptl.x()) )));
                                else if ( cstype == 2)
                                        ptl.setY(dgr.Transy(csypos=m_parser->a2fkt(&m_parser->ufkt[csmode], csxpos=dgr.Transx(ptl.x()))));
        
                                if(fabs(csypos)<0.2)
                                {
                                        double x0;
                                        if(root(&x0))
                                        {
                                                QString str="  ";
                                                str+=i18n("root");
                                                stbar->changeItem(str+QString().sprintf(":  x0= %+.5f", x0), 3);
                                                rootflg=1;
                                        }
                                }
                        }
                        else
                        {
                                stbar->changeItem("", 3);
                                rootflg=0;
                        }
		}
		else
		{
			csxpos=dgr.Transx(ptl.x());
			csypos=dgr.Transy(ptl.y());
		}
		ptd=DC.xForm(ptl);
		DC.end();

		sprintf(sx, "  x= %+.2f", (float)dgr.Transx(ptl.x()));//csxpos);
		sprintf(sy, "  y= %+.2f", csypos);
		if(csflg==0)        // Hintergrund speichern
		{
			bitBlt(&hline, 0, 0, this, area.left(), fcy=ptd.y(), area.width(), 1);
			bitBlt(&vline, 0, 0, this, fcx=ptd.x(), area.top(), 1, area.height());
			// Fadenkreuz zeichnen
			QPen pen;
			if ( cstype == -1)
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
						  pen.setColor( itf->color);
						  break;
					   case 1:
						  pen.setColor( itf->f1_color);
						  break;
					   case 2:
						  pen.setColor( itf->f2_color);
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
		setCursor(blankCursor);
	}
	else
	{
		setCursor(arrowCursor);
		sx[0]=sy[0]=0;
	}
	stbar->changeItem(sx, 1);
	stbar->changeItem(sy, 2);
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
		
		double diffx = (xmax-xmin)*(double)Settings::zoomInStep()/100;
		double diffy = (ymax-ymin)*(double)Settings::zoomInStep()/100;
		
		if ( diffx < 0.00001 || diffy < 0.00001)
			return;
		QString str_tmp;	
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

		double diffx = (xmax-xmin)*(((double)Settings::zoomOutStep()/100) +1);
		double diffy = (ymax-ymin)*(((double)Settings::zoomOutStep()/100) +1);
			
		if ( diffx > 1000000 || diffy > 1000000)
			return;
		QString str_tmp;
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
	double const g=tlgy*double(xmax-xmin)/(2*double(ymax-ymin));
	if(e->button()==RightButton) //clicking with the right mouse button
	{
		char function_type;
                QValueVector<Parser::Ufkt>::iterator itu = m_parser->ufkt.begin();
                for( QValueVector<XParser::FktExt>::iterator itf = m_parser->fktext.begin(); itf != m_parser->fktext.end(); ++itf)
                {
                        
			function_type = itf->extstr[0].latin1();
			if ( function_type==0 || function_type=='y' || function_type=='r') 
				continue;
			int k=0;
			int const ke=itf->k_liste.count();
			do
			{
				if( itf->use_slider == -1 )
                                {
                                        if ( !itf->k_liste.isEmpty())
					   itu->setParameter(itf->k_liste[k]);
                                }
				else
					itu->setParameter(sliders[ itf->use_slider ]->slider->value() );
                                           
				if ( function_type=='x' &&  fabs(csxpos-m_parser->fkt(itu, csxpos))< g && itf->extstr.contains('t')==1) //parametric plot
				{
                                        QValueVector<Parser::Ufkt>::iterator ufkt_y = itu+1;
                                        QValueVector<XParser::FktExt>::iterator fktext_y = itf+1;
					if ( fabs(csypos-m_parser->fkt(ufkt_y, csxpos)<g)  && fktext_y->extstr.contains('t')==1)
					{
							if ( csmode == -1)
							{
								csmode=itu->id;
								cstype=0;
								csparam = k;
								m_popupmenushown = 1;
							}
							else
								m_popupmenushown = 2;
							QString y_name( fktext_y->extstr );
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-1),false);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-2),false);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-3),false);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-4),false);
							m_popupmenu->changeTitle(10,fktext_y->extstr+";"+y_name);
							m_popupmenu->exec(QCursor::pos());
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-1),true);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-2),true);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-3),true);
							m_popupmenu->setItemEnabled(m_popupmenu->idAt(m_popupmenu->count()-4),true);
							return;
					}
				}
				if( fabs(csypos-m_parser->fkt(itu, csxpos))< g && itf->f_mode)
				{
					if ( csmode == -1)
					{
						csmode=itu->id;
						cstype=0;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					m_popupmenu->changeTitle(10, itf->extstr);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				
				if(fabs(csypos-m_parser->a1fkt( itu, csxpos))< g && itf->f1_mode)
				{
					if ( csmode == -1)
					{
						csmode=itu->id;
						cstype=1;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					QString function = itf->extstr;
					function = function.left(function.find('(')) + '\'';
					m_popupmenu->changeTitle(10, function);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
				if(fabs(csypos-m_parser->a2fkt(itu, csxpos))< g && itf->f2_mode)
				{
					if ( csmode == -1)
					{
						csmode=itu->id;
						cstype=2;
						csparam = k;
						m_popupmenushown = 1;
					}
					else
						m_popupmenushown = 2;
					QString function = itf->extstr;
					function = function.left(function.find('(')) + "\'\'";
					m_popupmenu->changeTitle(10, function);
					m_popupmenu->exec(QCursor::pos());
					return;
				}
			}
			while(++k<ke);
                        
                        ++itu;
		}
		return;
	}
	if(e->button()!=LeftButton) return ;
	if(csmode>=0) //disable trace mode if trace mode is enable
	{
		csmode=-1;
		stbar->changeItem("",3);
		stbar->changeItem("",4);
		mouseMoveEvent(e);
		return ;
	}
        QValueVector<Parser::Ufkt>::iterator itu = m_parser->ufkt.begin();
        for( QValueVector<XParser::FktExt>::iterator itf = m_parser->fktext.begin(); itf != m_parser->fktext.end(); ++itf)
	{
		switch(itf->extstr[0].latin1())
		{
			case 0: case 'x': case 'y': case 'r': continue;   // Not possible to catch
		}
	
		int k=0;
		int const ke=itf->k_liste.count();
		do
		{
			if( itf->use_slider == -1 )
                        {
                                if ( !itf->k_liste.isEmpty() )
				     itu->setParameter( itf->k_liste[k]);
                        }
			else
				itu->setParameter(sliders[ itf->use_slider ]->slider->value() );
			if(fabs(csypos-m_parser->fkt(itu, csxpos))< g && itf->f_mode)
			{
				csmode=itu->id;
				cstype=0;
				csparam = k;
				m_minmax->selectItem();
				stbar->changeItem( itf->extstr,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a1fkt( itu, csxpos))< g && itf->f1_mode)
			{
				csmode=itu->id;
				cstype=1;
				csparam = k;
				m_minmax->selectItem();
				QString function = itf->extstr;
				function = function.left(function.find('(')) + '\'';
				stbar->changeItem(function,4);
				mouseMoveEvent(e);
				return;
			}
			if(fabs(csypos-m_parser->a2fkt(itu, csxpos))< g && itf->f2_mode)
			{
				csmode=itu->id;
				cstype=2;
				csparam = k;
				m_minmax->selectItem();
				QString function = itf->extstr;
				function = function.left(function.find('(')) + "\'\'";
				stbar->changeItem(function,4);
				mouseMoveEvent(e);
				return;
			}
		}
		while(++k<ke);
                
                ++itu;
	}

	csmode=-1;
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
		
		
		if ( real1x>xmax || real2x>xmax || real1x<xmin  || real2x<xmin  || 
		     real1y>ymax || real2y>ymax || real1y<ymin  || real2y<ymin )
			return; //out of bounds
		
		QString str_tmp;
		//setting new x-boundaries
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
		//setting new y-boundaries
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
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };
	
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
	m_parser->linewidth0 = Settings::gridLineWidth();
	
	backgroundcolor = Settings::backgroundcolor();
	invertColor(backgroundcolor,inverted_backgroundcolor);
	setBackgroundColor(backgroundcolor);
}

void View::init()
{
	getSettings();
        
        if ( m_parser->fktext.isEmpty() )
                return;
        QValueVector<Parser::Ufkt>::iterator it = m_parser->ufkt.begin();
        it->fname="";
        while ( m_parser->ufkt.count() > 1)
            m_parser->Parser::delfkt( &m_parser->ufkt.last() );
            
        m_parser->fktext.clear();
}


void View::progressbar_clicked()
{
	stop_calculating = true;
}

void View::findMinMaxValue(XParser::FktExt *fktext, char p_mode, bool minimum, double &dmin, double &dmax, QString &str_parameter)
{
        Parser::Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(fktext->id) ];
	double x, y;
	double result_x = 0;
	double result_y = 0;
	bool start = true;

        // TODO: parameter sliders
        if ( !fktext->k_liste.isEmpty() )
        {
                int i=0;
                for ( QStringList::Iterator it = fktext->str_parameter.begin(); it != fktext->str_parameter.end(); ++it )
                {
                      if ( *it == str_parameter)
                      {
                             ufkt->setParameter(fktext->k_liste[i]);
                             break;
                      }
                      i++;
                }
        }

	isDrawing=true;
	setCursor(Qt::WaitCursor );
	
	double dx;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		progressbar->progress->reset();
		if ( fktext->integral_use_precision )
			dx = fktext->integral_precision*(dmax-dmin)/(area.width()*10);
		else
			dx=stepWidth*(dmax-dmin)/(area.width()*10); //the stepwidth must be small for Euler's metod
		progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
		progressbar->show();
		x = fktext->startx; //the initial x-point
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
				y=m_parser->fkt(ufkt, x);
				m_parser->euler_method(x, y, ufkt, fktext);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
					progressbar->increase();
					paintEvent(0);
				}
				break;
			}
		}
		
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
		if (p_mode==3)
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode== 3)
				{
					forward_direction = false;
					x = fktext->startx;
				}
			}
			else
				x=x-dx; // go backwards	
		}
		else
			x=x+dx;
	}
	if (  progressbar->isVisible())
		progressbar->hide(); // hide the progressbar-widget if it was shown
	isDrawing=false;
	restoreCursor();
	
	dmin = int(result_x*1000)/double(1000);
	dmax = int(result_y*1000)/double(1000);
}

void View::getYValue(XParser::FktExt *fktext, char p_mode,  double x, double &y, QString &str_parameter)
{
        Parser::Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(fktext->id) ];
	// TODO: parameter sliders
	if ( !fktext->k_liste.isEmpty() )
        {
                int i=0;
                for ( QStringList::Iterator it = fktext->str_parameter.begin(); it != fktext->str_parameter.end(); ++it )
                {
		      if ( *it == str_parameter)
		      {
			     ufkt->setParameter(fktext->k_liste[i]);
			     break;
		      }
		      i++;
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
			double dmin = fktext->dmin;
			double dmax = fktext->dmax;
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
			if ( fktext->integral_use_precision )
				dx = fktext->integral_precision*(dmax-dmin)/(area.width()/10);
			else
				dx=stepWidth*(dmax-dmin)/(area.width()/10); //the stepwidth must be small for Euler's metod
			
			stop_calculating = false;
			isDrawing=true;
			setCursor(Qt::WaitCursor );
			bool target_found=false;
			progressbar->progress->reset();
			progressbar->progress->setTotalSteps ((int) double((dmax-dmin)/dx)/2 );
			progressbar->show();
			x = fktext->startx; //the initial x-point
			while (x>=dmin && !stop_calculating && !target_found)
			{
				y=m_parser->fkt(ufkt, x);
				m_parser->euler_method(x, y,ufkt, fktext);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
					progressbar->increase();
					paintEvent(0);
				}

				if ( (x+dx > target && forward_direction) || ( x+dx < target && !forward_direction)) //right x-value is found
					target_found = true;



				if (forward_direction)
				{
					x=x+dx;
					if (x>dmax)
					{
						forward_direction = false;
						x = fktext->startx;
					}
				}
				else
					x=x-dx; // go backwards
			}
			if (  progressbar->isVisible())
				progressbar->hide(); // hide the progressbar-widget if it was shown

			isDrawing=false;
			restoreCursor();
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
                QValueVector<XParser::FktExt>::iterator it = &m_parser->fktext[m_parser->ixValue(csmode)];
		int const ke=it->k_liste.count();
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
                                switch(m_parser->fktext[csmode].extstr[0].latin1())
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
								        if ( m_parser->fktext[csmode].f_mode )
									       found=true;
								        break;
							 case (1):
								        if ( m_parser->fktext[csmode].f1_mode )
									       found=true;
								        break;
							 case (2):
								        if ( m_parser->fktext[csmode].f2_mode )
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
                                
                             if ( ++it == m_parser->fktext.end())
                                it = m_parser->fktext.begin();
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
				stbar->changeItem(m_parser->fktext[csmode ].extstr,4);
				break;
			case 1:
			{
				QString function = m_parser->fktext[ csmode ].extstr;
				function = function.left(function.find('(')) + '\'';
				stbar->changeItem(function,4);
                                break;
			}
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

void View::areaUnderGraph(XParser::FktExt *fktext, char const p_mode,  double &dmin, double &dmax, QString &str_parameter, QPainter *DC)
{
        Parser::Ufkt *ufkt = &m_parser->ufkt[ m_parser->ixValue(fktext->id) ];
	double x, y;
	float calculated_area=0;
	int rectheight;
	areaMin = dmin;
	QPoint p;
	QColor color;
	switch(p_mode)
	{
		case 0: 
			color = fktext->color;
			break;
		case 1:
			color = fktext->f1_color;
			break;
		case 2:
			color = fktext->f2_color;
			break;
		case 3:
			color = fktext->integral_color;
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
        if ( !fktext->k_liste.isEmpty() )
        {
                int i=0;
                for ( QStringList::Iterator it = fktext->str_parameter.begin(); it != fktext->str_parameter.end(); ++it )
                {
                      if ( *it == str_parameter)
                      {
                             ufkt->setParameter(fktext->k_liste[i]);
                             break;
                      }
                      i++;
                }
        }
	double dx;
	if ( p_mode == 3)
	{
		stop_calculating = false;
		if ( fktext->integral_use_precision )
			dx = fktext->integral_precision*(dmax-dmin)/(area.width()*10);
		else
			dx=stepWidth*(dmax-dmin)/(area.width()*10); //the stepwidth must be small for Euler's metod
		progressbar->progress->reset();
		progressbar->progress->setTotalSteps ( (int)double((dmax-dmin)/dx)/2 );
		progressbar->show();
		x = fktext->startx; //the initial x-point
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
	while ((x>=dmin && x<=dmax) ||  (p_mode == 3 && x>=dmin && !forward_direction) || (p_mode == 3 && x<=dmax && forward_direction))
	{
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
			{
				y=m_parser->a1fkt( ufkt, x);
				break;
			}
			case 2:
			{
				y=m_parser->a2fkt( ufkt, x);
				break;
			}
			case 3:
			{
				y=m_parser->fkt(ufkt, x);
				m_parser->euler_method(x, y, ufkt, fktext);
				if ( int(x*100)%2==0)
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
					progressbar->increase();
					paintEvent(0);
				}
				break;
			}
		}
		
		p.setX(dgr.Transx(x));
		p.setY(dgr.Transy(y));
		if (dmin<=x && x<=dmax)
		{
			if(dgr.xclipflg || dgr.yclipflg)
			{
				if ( y<0)
				{
					//p.setY(dgr.Transy(ymin));
					rectheight = origoy-p.y() ;
				}
				else
				{
					//p.setY(dgr.Transy(ymax));
					rectheight= -1*( p.y()-origoy) ;
				}
				calculated_area = calculated_area + ( dx*y);
				DC->fillRect(p.x(),p.y(),rectwidth,rectheight,color);
			}
			else
			{
				if ( y<0)
				{
					rectheight =  origoy-p.y();
				}
				else
				{
					rectheight = -1*( p.y()-origoy);
				}
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
					x = fktext->startx;
				}
			}
			else
				x=x-dx; // go backwards	
		}
		else
			x=x+dx;
	}
	if (  progressbar->isVisible())
	{
		progressbar->hide(); // hide the progressbar-widget if it was shown
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
	
	
	areaFktext = fktext;
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
		sliders[ number ]->hide();
	for( uint index = 0; index < m_parser->fktext.count(); index++ )
		if( m_parser->fktext[ index ].use_slider > -1  &&  (m_parser->fktext[ index ].f_mode ||  m_parser->fktext[index].f1_mode || m_parser->fktext[ index ].f2_mode || m_parser->fktext[ index ].integral_mode))
			sliders[ m_parser->fktext[ index ].use_slider ]->show();
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
	m_modified = true;
	updateSliders();
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
		char const function_type = m_parser->fktext[csmode].extstr[0].latin1();
                int const ix = m_parser->ixValue(csmode);
                if ( ix == -1)
                        return;
		if ( function_type == 'x')  // a parametric function
                        m_parser->delfkt(ix ); //remove the y-function
                
		m_parser->delfkt( ix );
                        
		drawPlot();
		if ( function_type != 'x' &&  function_type != 'y' && function_type != 'r' ) 
			updateSliders();
		m_modified = true;
	}
}
void View::mnuEdit_clicked()
{
	if ( m_parser->fktext[csmode].extstr[0] == 'x') // a parametric function
	{
		int y_index = csmode+1; //the y-function
		if ( y_index == (int)m_parser->fktext.count())
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
void View::mnuNoZoom_clicked()
{
	setCursor(Qt::ArrowCursor);
	zoom_mode = 0;	
}

void View::mnuRectangular_clicked()
{
	setCursor(Qt::CrossCursor);
	zoom_mode = 1;
}
void View::mnuZoomIn_clicked()
{
	setCursor( QCursor( SmallIcon( "magnify", 32), 10, 10 ) );
	zoom_mode = 2;	
}

void View::mnuZoomOut_clicked()
{
	setCursor( QCursor( SmallIcon( "lessen", 32), 10, 10 ) );
	zoom_mode = 3;
}
void View::mnuCenter_clicked()
{
	setCursor(Qt::PointingHandCursor);
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
		case 0:  //no zoom
			setCursor(Qt::ArrowCursor);
			break;
		case 1: //rectangle zoom
			setCursor(Qt::CrossCursor);
			break;
		case 2: //zoom in
			setCursor( QCursor( SmallIcon( "magnify", 32), 10, 10 ) );
			break;
		case 3: //zoom in
			setCursor( QCursor( SmallIcon( "lessen", 32), 10, 10 ) );
			break;
		case 5: //center a point
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
};

