/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
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
#include "View.moc"

// other includes
#include <assert.h>
#include <cmath>

//minimum and maximum x range. Should always be accessible.
double View::xmin = 0;
double View::xmax = 0;

View * View::m_self = 0;


View::View(bool const r, bool &mo, KMenu *p, QWidget* parent, KActionCollection *ac, MainDlg * mainDlg )
	: DCOPObject("View"),
	  QWidget( parent, Qt::WStaticContents ),
	  buffer( width(), height() ),
	  m_popupmenu(p),
	  m_modified(mo),
	  m_readonly(r),
	  m_dcop_client(KApplication::kApplication()->dcopClient()),
	  m_ac(ac),
	  m_mainDlg( mainDlg )
{
	m_self = this;
	csmode = csparam = -1;
	cstype = Ufkt::Function;
	areaDraw = false;
	areaUfkt = 0;
	areaPMode = Ufkt::Function;
	areaMin = areaMax = 0.0;
	w = h = 0;
	s = 0.0;
	rootflg = false;
	tlgx = tlgy = drskalx = drskaly = 0.0;;
	stepWidth = 1.0;
	ymin = 0.0;
	ymax = 0.0;
	csxpos = 0.0;
	csypos = 0.0;
	m_trace_x = 0.0;
	m_printHeaderTable = false;
	stop_calculating = false;
	m_minmax = 0;
	isDrawing = false;
	m_popupmenushown = 0;
	m_zoomMode = Normal;
	m_prevCursor = CursorArrow;
	
	m_mousePressTimer = new QTime();
	
	m_parser = new XParser(mo);
	init();
	getSettings();
	
	setMouseTracking(TRUE);
	m_sliderWindow = 0;
	updateSliders();
	
// 	m_popupmenu->addTitle( " " );
	/// \todo fix title on popup menu (should display function name
}

void View::setMinMaxDlg(KMinMax *minmaxdlg)
{
	m_minmax = minmaxdlg;
}

View::~View()
{
	delete m_parser;
	delete m_mousePressTimer;
}

XParser* View::parser()
{
	return m_parser;
}

void View::draw(QPaintDevice *dev, int form)
{
	double lx, ly;
	double sf;
	QRect rc;
	QPainter DC;    // our painter
	DC.begin(dev);    // start painting widget
	rc=DC.viewport();
	w=rc.width();
	h=rc.height();

	if(form==0)          // screen
	{
		ref=QPoint(120, 100);
		lx=((xmax-xmin)*100.*drskalx/tlgx);
		ly=((ymax-ymin)*100.*drskaly/tlgy);
		DC.scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
		if ( ( QPointF(lx+2*ref.x(), ly) * DC.matrix() ).x() > DC.viewport().right())
		{
			DC.resetMatrix();
			DC.scale((float)w/(float)(lx+2*ref.x()), (float)w/(float)(lx+2*ref.x()));
		}
		wm = DC.matrix();
		s=( QPoint(1000, 0) * DC.matrix() ).x()/1000.;
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
	}
	else if(form==1)        // printer
	{
		sf=72./254.;        // 72dpi
		ref=QPoint(100, 100);
		lx=((xmax-xmin)*100.*drskalx/tlgx);
		ly=((ymax-ymin)*100.*drskaly/tlgy);
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
		lx=((xmax-xmin)*100.*drskalx/tlgx);
		ly=((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.translate(-dgr.GetFrame().left(), -dgr.GetFrame().top());
		s=1.;
	}
	else if(form==3)								// bmp, png
	{
		sf=180./254.;								// 180dpi
		ref=QPoint(0, 0);
		lx=((xmax-xmin)*100.*drskalx/tlgx);
		ly=((ymax-ymin)*100.*drskaly/tlgy);
		dgr.Create( ref, lx, ly, xmin, xmax, ymin, ymax );
		DC.end();
		*((QPixmap *)dev) = QPixmap( (int)(dgr.GetFrame().width()*sf), (int)(dgr.GetFrame().height()*sf) );
		((QPixmap *)dev)->fill(backgroundcolor);
		DC.begin(dev);
		DC.translate(-dgr.GetFrame().left()*sf, -dgr.GetFrame().top()*sf);
		DC.scale(sf, sf);
		s=1.;
	}

	dgr.updateSettings();
// 	kDebug() << "tlgx="<<tlgx<<" tlgy="<<tlgy<<endl;
	dgr.Skal( tlgx, tlgy );

	if ( form!=0 && areaDraw)
	{
		areaUnderGraph(areaUfkt, areaPMode, areaMin,areaMax, areaParameter, &DC);
		areaDraw = false;
		if (stop_calculating)
			return;
	}

	DC.setRenderHint( QPainter::Antialiasing, true );
	dgr.Plot(&DC);
	
	PlotArea=dgr.GetPlotArea();
	area=DC.matrix().mapRect(PlotArea);
	stepWidth=Settings::stepWidth();
	
// 	assert( stepWidth != 0.0 );
	if ( stepWidth == 0.0 )
	{
		kWarning() << k_funcinfo << "Zero stepwidth!\n";
		stepWidth = 1.0;
	}

	isDrawing=true;
	updateCursor();
	stop_calculating = false;
	
	// Antialiasing slows down rendering a lot, so turn it off if we are
	// sliding the view about
	DC.setRenderHint( QPainter::Antialiasing, m_zoomMode != Translating );
// 	if ( m_zoomMode != Translating )
// 		kDebug() << "##############################\n";
	DC.setClipping( true );
	DC.setClipRect( PlotArea );
	foreach ( Ufkt * ufkt, m_parser->m_ufkt )
	{
		if ( stop_calculating )
			break;
		
		if ( !ufkt->fname.isEmpty() )
			plotfkt(ufkt, &DC);
	}
	DC.setClipping( false );

	isDrawing=false;
	updateCursor();
	DC.end();   // painting done
}


void View::plotfkt(Ufkt *ufkt, QPainter *pDC)
{
	int k, ke, mflg;

	QChar const fktmode=ufkt->fstr[0];
	if ( fktmode == 'y' )
		return;
	
	double dmin = ufkt->dmin;
	if(!ufkt->usecustomxmin)
	{
		if(fktmode=='r')
			dmin=0.;
		else if ( (fktmode == 'x') || (fktmode == 'y') )
			dmin = -M_PI;
		else
			dmin = xmin;
	}
	if ( dmin < xmin )
		dmin = xmin;
	
	double dmax = ufkt->dmax;
	if(!ufkt->usecustomxmax)
	{
		if(fktmode=='r')
			dmax = 2*M_PI;
		else if ( (fktmode == 'x') || (fktmode == 'y') )
			dmax = M_PI;
		else
			dmax = xmax;
	}
	if ( dmax > xmax )
		dmax = xmax;
	
	double dx;
	if(fktmode=='r')
	{
		if ( Settings::useRelativeStepWidth() )
			dx=stepWidth*0.05/(dmax-dmin);
		else
			dx=stepWidth;
	}
	else
	{
		if ( Settings::useRelativeStepWidth() )
			dx=stepWidth*(dmax-dmin)/area.width();
		else
			dx=stepWidth;
	}
	assert( dx != 0.0 );

	int iy = -1;
	if(fktmode=='x')
		iy = ufkt->id+1;
	
	Ufkt::PMode p_mode = Ufkt::Function;
	
	double x, y = 0.0;
	QPointF p1, p2;
	
	while(1)
	{
		pDC->setPen( penForPlot( ufkt, p_mode, pDC->renderHints() & QPainter::Antialiasing ) );
		
		k=0;
		ke=ufkt->parameters.count();
		do
		{
			if ( p_mode == Ufkt::Integral && stop_calculating)
				break;
			if( ufkt->use_slider == -1 )
			{
				if ( !ufkt->parameters.isEmpty() )
					ufkt->setParameter( ufkt->parameters[k].value );
			}
			else
			{
				if ( m_sliderWindow )
					ufkt->setParameter( m_sliderWindow->value( ufkt->use_slider ) );
			}

			mflg=2;
			if ( p_mode == Ufkt::Integral )
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
			}
			else
				x=dmin;
			bool forward_direction;

			if (dmin<0 && dmax<0)
				forward_direction = false;
			else
				forward_direction = true;

			if ( p_mode != Ufkt::Function || ufkt->f_mode) // if not the function is hidden
			{
				while ((x>=dmin && x<=dmax) ||  (p_mode == Ufkt::Integral && x>=dmin && !forward_direction) || (p_mode == Ufkt::Integral && x<=dmax && forward_direction))
				{
					if ( p_mode == Ufkt::Integral && stop_calculating)
					{
						p_mode = Ufkt::Derivative1;
						x=dmax+1;
						continue;
					}
					switch(p_mode)
					{
						case Ufkt::Function:
							y=m_parser->fkt(ufkt, x);
							break;
						case Ufkt::Derivative1:
							y=m_parser->a1fkt(ufkt, x);
							break;
						case Ufkt::Derivative2:
							y=m_parser->a2fkt(ufkt, x);
							break;
						case Ufkt::Integral:
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
						p2.setX(dgr.TransxToPixel( y*cos(x), false ));
						p2.setY(dgr.TransyToPixel( y*sin(x), false ));
					}
					else if(fktmode=='x')
					{
						p2.setX(dgr.TransxToPixel( y, false ));
						p2.setY(dgr.TransyToPixel( m_parser->fkt(iy, x), false ));
					}
					else
					{
						p2.setX(dgr.TransxToPixel( x, false ));
						p2.setY(dgr.TransyToPixel( y, false ));
					}

					if ( dgr.xclipflg || dgr.yclipflg )
					{
						p1=p2;
					}
					else
					{
						if(mflg<=1)
							pDC->drawLine(p1, p2);
						p1=p2;
						mflg=0;
					}

					if ( p_mode == Ufkt::Integral )
					{
						if ( forward_direction)
						{
							x=x+dx;
							if (x>dmax && p_mode == Ufkt::Integral )
							{
								forward_direction = false;
								x = ufkt->oldx = ufkt->startx;
								ufkt->oldy = ufkt->starty;
								ufkt->oldyprim = ufkt->integral_precision;
// 								paintEvent(0);
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
		}
		while(++k<ke);

		// Advance to the next appropriate p_mode
		if (ufkt->f1_mode==1 && p_mode< Ufkt::Derivative1)
			p_mode=Ufkt::Derivative1; //draw the 1st derivative
		else if(ufkt->f2_mode==1 && p_mode< Ufkt::Derivative2)
			p_mode=Ufkt::Derivative2; //draw the 2nd derivative
		else if( ufkt->integral_mode==1 && p_mode < Ufkt::Integral)
			p_mode=Ufkt::Integral; //draw the integral
		else
			break; // no derivatives or integrals left to draw
	}
	if ( stopProgressBar() )
		if( stop_calculating)
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
}


QPen View::penForPlot( Ufkt *ufkt, Ufkt::PMode p_mode, bool antialias ) const
{
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);
// 	pen.setStyle( Qt::DashLine );
	
	double lineWidth_mm;
	
	switch ( p_mode )
	{
		case 0:
			lineWidth_mm = ufkt->linewidth;
			pen.setColor(ufkt->color);
			break;
			
		case 1:
			lineWidth_mm = ufkt->f1_linewidth;
			pen.setColor(ufkt->f1_color);
			break;
			
		case 2:
			lineWidth_mm = ufkt->f2_linewidth;
			pen.setColor(ufkt->f2_color);
			break;
			
		case 3:
			lineWidth_mm = ufkt->integral_linewidth;
			pen.setColor(ufkt->integral_color);
			break;
			
		default:
			assert( !"Unknown p_mode" );
			break;
	}
	
	double width = mmToPenWidth( lineWidth_mm, antialias );
	if ( (width*s < 3) && !antialias )
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
		w = std::floor(w*s)/s;
	}
	
	return w;
}


void View::drawHeaderTable(QPainter *pDC)
{
	QString alx, aly, atx, aty, dfx, dfy;

	if( m_printHeaderTable )
	{
		pDC->translate(250., 150.);
		pDC->setPen(QPen(Qt::black, (int)(5.*s)));
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
		//for(uint ix=0; ix<m_parser->countFunctions() && !stop_calculating; ++ix)
// 		for(QVector<Ufkt>::iterator it=m_parser->ufkt.begin(); it!=m_parser->ufkt.end() && !stop_calculating; ++it)
		foreach ( Ufkt * it, m_parser->m_ufkt )
		{
			if ( stop_calculating )
				break;
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

	while((i=s->indexOf('p')) != -1)
		s->replace(i, 2, &c, 1);
}


bool View::root(double *x0, Ufkt *it)
{
	if(rootflg)
		return FALSE;
	
	int k = 0; // iteration count
	int max_k = 100; // maximum number of iterations
	double max_y = 1e-9; // the largest value of y which is deemed a root found
	
	*x0 = csxpos;
	double y = csypos;
	bool tooBig = true;
	
	do
	{
		*x0 -= y / m_parser->a1fkt( it, *x0 );
		y = m_parser->fkt( it, *x0 );
		
		tooBig = (qAbs(y) > max_y);
	}
	while ( (k++<max_k) && tooBig );
	
	return ! tooBig;
}

void View::paintEvent(QPaintEvent *)
{
	QPainter p;
	p.begin(this);
	
// 	bitBlt( this, 0, 0, &buffer, 0, 0, width(), height() );
	p.drawPixmap( QPoint( 0, 0 ), buffer );
	
	// the current cursor position in widget coordinates
	QPoint mousePos = mapFromGlobal( QCursor::pos() );
	
	if ( (m_zoomMode == ZoomInDrawing) || (m_zoomMode == ZoomOutDrawing) )
	{
		QPalette palette;
		QColor highlightColor = palette.highlight().color();
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
		QPointF tl( dgr.TransxToPixel( m_animateZoomRect.left() ), dgr.TransyToPixel( m_animateZoomRect.top() ) );
		QPointF br( dgr.TransxToPixel( m_animateZoomRect.right() ), dgr.TransyToPixel( m_animateZoomRect.bottom() ) );
		p.drawRect( QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) ) );
		p.restore();
	}
	else if ( shouldShowCrosshairs() )
	{
		updateCrosshairPosition();
		
		Ufkt * it = m_parser->functionWithID( csmode );
			
			// Fadenkreuz zeichnen [draw the cross-hair]
		QPen pen;
		if ( !it )
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
			if ( pen.color() == backgroundcolor) // if the "Fadenkreuz" [cross-hair] has the same color as the background, the "Fadenkreuz" [cross-hair] will have the inverted color of background so you can see it easier
				pen.setColor(inverted_backgroundcolor);
		}
		p.setPen( pen );
		p.Lineh( area.left(), m_crosshairPixelCoords.y(), area.right() );
		p.Linev( m_crosshairPixelCoords.x(), area.bottom(), area.top());
	}
	
	p.end();
}

void View::resizeEvent(QResizeEvent *)
{
	if (isDrawing) //stop drawing integrals
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
	buffer.fill(backgroundcolor);
	draw(&buffer, 0);
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


bool View::csxposValid( Ufkt * plot ) const
{
	if ( !plot )
		return false;
	
	// only relevant for cartesian plots - assume true for none
	if ( plot->fstr.startsWith('r') ||
			plot->fstr.startsWith('x') ||
			plot->fstr.startsWith('y') )
		return true;
	
	bool lowerOk = ((!plot->usecustomxmin) || (plot->usecustomxmin && csxpos>plot->dmin));
	bool upperOk = ((!plot->usecustomxmax) || (plot->usecustomxmax && csxpos<plot->dmax));
	
	return lowerOk && upperOk;
}


void View::mousePressEvent(QMouseEvent *e)
{
	m_mousePressTimer->start();
	
	if ( m_popupmenushown>0)
		return;

	if (isDrawing)
	{
		stop_calculating = true; //stop drawing
		return;
	}
	
	if ( m_zoomMode != Normal )
		return;
	
	bool hadFunction = (csmode != -1 );
	
	updateCrosshairPosition();
	
	if( !m_readonly && e->button()==Qt::RightButton) //clicking with the right mouse button
	{
		getPlotUnderMouse();
		Ufkt * function = m_parser->functionWithID( csmode );
		if ( function )
		{
			QString popupTitle;
			
			QChar function_type = function->fstr[0].latin1();
			if ( function_type == 'x' )
			{
				// parametric function
				Ufkt * ufkt_y = m_parser->functionWithID( csmode+1 );
				assert( ufkt_y );
				popupTitle = function->fstr + ";" + ufkt_y->fstr;
			}
			else switch ( cstype )
			{
				case Ufkt::Function:
					popupTitle = function->fstr;
					break;
					
				case Ufkt::Derivative1:
					popupTitle = function->fstr.left( function->fstr.indexOf('(') ) + '\'';
					break;
					
				case Ufkt::Derivative2:
					popupTitle = function->fstr.left( function->fstr.indexOf('(') ) + "\'\'";
					break;
					
				case Ufkt::Integral:
					popupTitle = function->fstr.left( function->fstr.indexOf('(') ).toUpper();
					break;
			}
			
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
	
	if(csmode>=0) //disable trace mode if trace mode is enable
	{
		csmode=-1;
		setStatusBar("",3);
		setStatusBar("",4);
		mouseMoveEvent(e);
		return ;
	}
	
	getPlotUnderMouse();
	Ufkt * function = m_parser->functionWithID( csmode );
	if ( function )
	{
		QChar function_type = function->fstr[0].latin1();
		
		if ( function_type == 'x' )
		{
			// parametric plot
			m_minmax->selectItem();
			setStatusBar(function->fstr,4);
			
			// csxpos, csypos would have been set by getPlotUnderMouse()
			QPointF ptd( dgr.TransxToPixel( csxpos ), dgr.TransyToPixel( csypos ) );
			QPoint globalPos = mapToGlobal( (ptd * wm).toPoint() );
			QCursor::setPos( globalPos );
			return;
		}
		
		else if ( function_type == 'r' )
		{
			// polar plot
			m_minmax->selectItem();
			setStatusBar(function->fstr,4);
			
			// csxpos, csypos would have been set by getPlotUnderMouse()
			QPointF ptd( dgr.TransxToPixel( csxpos ), dgr.TransyToPixel( csypos ) );
			QPoint globalPos = mapToGlobal( (ptd * wm).toPoint() );
			QCursor::setPos( globalPos );
			return;
		}
		
		else
		{
			// cartesian plot
		
			switch ( cstype )
			{
				case Ufkt::Function:
				{
					m_minmax->selectItem();
					setStatusBar(function->fstr,4);
					mouseMoveEvent(e);
					return;
				}
			
				case Ufkt::Derivative1:
				{
					m_minmax->selectItem();
					QString fstr = function->fstr;
					fstr = fstr.left(fstr.indexOf('(')) + '\'';
					setStatusBar(fstr,4);
					mouseMoveEvent(e);
					return;
				}
			
				case Ufkt::Derivative2:
				{
					m_minmax->selectItem();
					QString fstr = function->fstr;
					fstr = fstr.left(fstr.indexOf('(')) + "\'\'";
					setStatusBar(fstr,4);
					mouseMoveEvent(e);
					return;
				}
			
				case Ufkt::Integral:
				{
					// can't trace integral
					return;
				}
			}
		}
	}
	
	// user didn't click on a plot; so we prepare to enter translation mode
	csmode=-1;
	m_zoomMode = AboutToTranslate;
	m_prevDragMousePos = e->pos();
}


void View::getPlotUnderMouse()
{
	csmode = -1;
	csparam = 0;
	cstype = Ufkt::Function;
	m_trace_x = 0.0;
	
	double const g=tlgy*double(xmax-xmin)/(2*double(ymax-ymin));
	
	foreach ( Ufkt * it, m_parser->m_ufkt )
	{
		QChar function_type = it->fstr[0];
		if ( function_type=='y' || it->fname.isEmpty())
			continue;
		if ( !csxposValid( it ) )
			continue;
		
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
				if ( m_sliderWindow )
					it->setParameter(  m_sliderWindow->value( it->use_slider ) );
			}

			if ( function_type=='x' && it->fstr.contains('t')==1 )
			{
				if ( !it->f_mode )
					continue;
				
				//parametric plot
				
				Ufkt * ufkt_y = m_parser->functionWithID( it->id + 1 );
				assert( ufkt_y );
				
				double best_x = getClosestPoint( csxpos, csypos, it, ufkt_y );
				
				if ( pixelDistance( csxpos, csypos, it, ufkt_y, best_x ) < 30.0 )
				{
					csmode=it->id;
					cstype=Ufkt::Function;
					csparam = k;
					m_trace_x = best_x;
					csxpos = m_parser->fkt( it, best_x );
					csypos = m_parser->fkt( ufkt_y, best_x );
					return;
				}
			}
			else if ( function_type == 'r' )
			{
				if ( !it->f_mode )
					continue;
				
				// polar plot
				
				double best_x = getClosestPoint( csxpos, csypos, it, 0 );
				
				if ( pixelDistance( csxpos, csypos, it, 0, best_x ) < 30.0 )
				{
					csmode=it->id;
					cstype=Ufkt::Function;
					csparam = k;
					m_trace_x = best_x;
					csxpos = m_parser->fkt( it, best_x ) * cos(best_x);
					csypos = m_parser->fkt( it, best_x ) * sin(best_x);
					return;
				}
			}
			else if( fabs(csypos-m_parser->fkt(it, csxpos))< g && it->f_mode)
			{
				csmode=it->id;
				cstype = Ufkt::Function;
				csparam = k;
				return;
			}
			else if(fabs(csypos-m_parser->a1fkt( it, csxpos))< g && it->f1_mode)
			{
				csmode=it->id;
				cstype = Ufkt::Derivative1;
				csparam = k;
				return;
			}
			else if(fabs(csypos-m_parser->a2fkt(it, csxpos))< g && it->f2_mode)
			{
				csmode=it->id;
				cstype = Ufkt::Derivative2;
				csparam = k;
				return;
			}
		}
		while(++k<ke);
	}
}


double View::getClosestPoint( double real_x, double real_y, Ufkt * function1, Ufkt * function2 )
{
	// either polar or parametric function
	bool isPolar = !function2;
	
	double minX = function1->usecustomxmin ? function1->dmin : (isPolar ? 0.0 : -M_PI);
	double maxX = function1->usecustomxmax ? function1->dmax : (isPolar ? 2.0*M_PI : -M_PI);
	double stepSize = 0.01;
	
	double best_x = 0.0;
	
	while ( stepSize > 0.0000009 )
	{
		double best_distance = 1e20; // a large distance
				
		double x = minX;
		while ( x <= maxX )
		{
			double distance = pixelDistance( real_x, real_y, function1, function2, x );
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
	
	return best_x;
}


double View::pixelDistance( double real_x, double real_y, Ufkt * function1, Ufkt * function2, double x )
{
	double fx, fy;
	
	if ( function2 )
	{
		// parametric function
		fx = m_parser->fkt( function1, x );
		fy = m_parser->fkt( function2, x );
	}
	else
	{
		// polar function
		fx = m_parser->fkt( function1, x ) * cos(x);
		fy = m_parser->fkt( function1, x ) * sin(x);
	}
					
	double dfx = dgr.TransxToPixel( real_x ) - dgr.TransxToPixel( fx );
	double dfy = dgr.TransyToPixel( real_y ) - dgr.TransyToPixel( fy );
					
	return std::sqrt( dfx*dfx + dfy*dfy );
}


void View::mouseMoveEvent(QMouseEvent *e)
{
	if ( isDrawing )
		return;
	
	bool inBounds = updateCrosshairPosition();
	if ( !rootflg )
		setStatusBar("", 3);
	
	QString sx, sy;
	
	if ( inBounds )
	{
// 		sx.sprintf( "  x= %+.2f", csxpos );
// 		sy.sprintf( "  y= %+.2f", csypos );
		if ( qAbs(csxpos) > 1e4 )
			sx = QString( "x = %1" ).arg( csxpos, 0, 'f', 0 );
		else
			sx = QString( "x = %1" ).arg( csxpos, 0, 'g', 3 );
		
		if ( qAbs(csypos) > 1e4 )
			sy = QString( "y = %1" ).arg( csypos, 0, 'f', 0 );
		else
			sy = QString( "y = %1" ).arg( csypos, 0, 'g', 3 );
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
			csmode=-1;
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
	csxpos = dgr.TransxToReal( ptl.x() );
	csypos = dgr.TransyToReal( ptl.y() );
	
	Ufkt * it = m_parser->functionWithID( csmode );
	
	if ( it && csxposValid( it ) )
	{
		// The user currently has a plot selected, with the mouse in a valid position
		
		if( it->use_slider == -1 )
		{
			if( !it->parameters.isEmpty() )
				it->setParameter( it->parameters[csparam].value );
		}
		else
		{
			if ( m_sliderWindow )
				it->setParameter( m_sliderWindow->value( it->use_slider ) );
		}
		
		QChar function_type = it->fstr[0];
		
		if ( function_type == 'x' )
		{
			// parametric plot
			
			Ufkt * ufkt_y = m_parser->functionWithID( it->id + 1 );
			assert( ufkt_y );
			
			// Should we increase or decrease t to get closer to the mouse?
			double dt[2] = { -0.00001, +0.00001 };
			double d[] = { 0.0, 0.0 };
			for ( int i = 0; i < 2; ++ i )
				d[i] = pixelDistance( csxpos, csypos, it, ufkt_y, m_trace_x + dt[i] );
			
			unsigned best_i = (d[0] < d[1]) ? 0 : 1;
			
			// how much t gets us the closest?
			double prev_best = d[best_i];
			m_trace_x += 2.0 * dt[best_i];
			while ( true )
			{	
				double new_distance = pixelDistance( csxpos, csypos, it, ufkt_y, m_trace_x + dt[best_i] );
				if ( new_distance < prev_best )
				{
					prev_best = new_distance;
					m_trace_x += dt[best_i];
				}
				else
					break;
			}
			
			double min = it->usecustomxmin ? it->dmin : -M_PI;
			double max = it->usecustomxmax ? it->dmax : M_PI;
// 			kDebug() << "m_trace_x="<<m_trace_x<<" max="<<max<<endl;
			if ( m_trace_x > max )
			{
// 				kDebug() << "bigger\n";
				m_trace_x  = max;
			}
			else if ( m_trace_x < min )
				m_trace_x = min;
			
			csxpos = m_parser->fkt( it, m_trace_x );
			csypos = m_parser->fkt( ufkt_y, m_trace_x );
			ptl = QPointF( dgr.TransxToPixel( csxpos ), dgr.TransyToPixel( csypos ) );
			QPoint globalPos = mapToGlobal( (ptl * wm).toPoint() );
			QCursor::setPos( globalPos );
		}
		else if ( function_type == 'r' )
		{
			// polar plot
			
			// Should we increase or decrease x to get closer to the mouse?
			double dx[2] = { -0.00001, +0.00001 };
			double d[] = { 0.0, 0.0 };
			for ( int i = 0; i < 2; ++ i )
				d[i] = pixelDistance( csxpos, csypos, it, 0, m_trace_x + dx[i] );
			
			unsigned best_i = (d[0] < d[1]) ? 0 : 1;
			
			// how much x gets us the closest?
			double prev_best = d[best_i];
			m_trace_x += 2.0 * dx[best_i];
			while ( true )
			{	
				double new_distance = pixelDistance( csxpos, csypos, it, 0, m_trace_x + dx[best_i] );
				if ( new_distance < prev_best )
				{
					prev_best = new_distance;
					m_trace_x += dx[best_i];
				}
				else
					break;
			}
			
			double min = it->usecustomxmin ? it->dmin : 0.0;
			double max = it->usecustomxmax ? it->dmax : 2.0*M_PI;
			if ( m_trace_x > max )
				m_trace_x  = max;
			else if ( m_trace_x < min )
				m_trace_x = min;
			
			csxpos = m_parser->fkt( it, m_trace_x ) * cos(m_trace_x);
			csypos = m_parser->fkt( it, m_trace_x ) * sin(m_trace_x);
			ptl = QPointF( dgr.TransxToPixel( csxpos ), dgr.TransyToPixel( csypos ) );
			QPoint globalPos = mapToGlobal( (ptl * wm).toPoint() );
			QCursor::setPos( globalPos );
		}
		else
		{
			// cartesian plot
			
			switch ( cstype )
			{
				case Ufkt::Function:
					csypos = m_parser->fkt( it, csxpos );
					ptl.setY(dgr.TransyToPixel( csypos ));
					break;
					
				case Ufkt::Derivative1:
					csypos = m_parser->a1fkt( it, csxpos );
					ptl.setY(dgr.TransyToPixel( csypos ));
					break;
					
				case Ufkt::Derivative2:
					csypos = m_parser->a2fkt( it, csxpos );
					ptl.setY(dgr.TransyToPixel( csypos ));
					break;
					
				case Ufkt::Integral:
					break;
			}

			if ( csypos<ymin || csypos>ymax) //the ypoint is not visible
			{
				out_of_bounds = true;
			}
			else if(fabs(dgr.TransyToReal(ptl.y())) < (xmax-xmin)/80)
			{
				double x0;
				if(root(&x0, it))
				{
					QString str="  ";
					str+=i18n("root");
					setStatusBar(str+QString().sprintf(":  x0 = %+.5f", x0), 3);
					rootflg=true;
				}
			}
			else
				rootflg=false;
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
// 	kDebug() << "area="<<area<<" m_mousePressTimer->elapsed()="<<m_mousePressTimer->elapsed()<<endl;
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
	
	updateCursor();
}


void View::zoomIn( const QPoint & mousePos, double zoomFactor )
{
	double realx = dgr.TransxToReal((mousePos * wm.inverted()).x());
	double realy = dgr.TransyToReal((mousePos * wm.inverted()).y());

	double diffx = (xmax-xmin)*zoomFactor;
	double diffy = (ymax-ymin)*zoomFactor;

	if ( diffx < 1e-8 || diffy < 1e-8 )
		return;
	
	animateZoom( QRectF( realx-diffx, realy-diffy, 2.0*diffx, 2.0*diffy ) );
}


void View::zoomIn( const QRect & zoomRect )
{
	QRect rect = wm.inverted().mapRect( zoomRect );

	QPoint p = rect.topLeft();
	double real1x = dgr.TransxToReal(p.x() );
	double real1y = dgr.TransyToReal(p.y() );
	p = rect.bottomRight();
	double real2x = dgr.TransxToReal(p.x() );
	double real2y = dgr.TransyToReal(p.y() );
	
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
	double _real1x = dgr.TransxToReal(p.x() );
	double _real1y = dgr.TransyToReal(p.y() );
	p = rect.bottomRight();
	double _real2x = dgr.TransxToReal(p.x() );
	double _real2y = dgr.TransyToReal(p.y() );
	
	double kx = (_real1x-_real2x)/(xmin-xmax);
	double lx = _real1x - (kx * xmin);
	
	double ky = (_real1y-_real2y)/(ymax-ymin);
	double ly = _real1y - (ky * ymax);
	
	double real1x = (xmin-lx)/kx;
	double real2x = (xmax-lx)/kx;
	
	double real1y = (ymax-ly)/ky;
	double real2y = (ymin-ly)/ky;
	
	animateZoom( QRectF( QPointF( real1x, real1y ), QSizeF( real2x-real1x, real2y-real1y ) ) );
}


void View::animateZoom( const QRectF & _newCoords )
{
	QRectF oldCoords( xmin, ymin, xmax-xmin, ymax-ymin );
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
	
	xmin = newCoords.left();
	xmax = newCoords.right();
	ymin = newCoords.top();
	ymax = newCoords.bottom();
	
	Settings::setXMin( Parser::number( xmin ) );
	Settings::setXMax( Parser::number( xmax ) );
	Settings::setYMin( Parser::number( ymin ) );
	Settings::setYMax( Parser::number( ymax ) );

	Settings::setXRange(4); //custom x-range
	Settings::setYRange(4); //custom y-range
	
	setScaling();
	
	drawPlot(); //update all graphs
	
	m_zoomMode = Normal;
}


void View::translateView( int dx, int dy )
{
	double rdx = dgr.TransxToReal( dx / s ) - dgr.TransxToReal( 0.0 );
	double rdy = dgr.TransyToReal( dy / s ) - dgr.TransyToReal( 0.0 );
	
	xmin += rdx;
	xmax += rdx;
	ymin += rdy;
	ymax += rdy;
	
	Settings::setXMin( Parser::number( xmin ) );
	Settings::setXMax( Parser::number( xmax ) );
	Settings::setYMin( Parser::number( ymin ) );
	Settings::setYMax( Parser::number( ymax ) );
	
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
		min = m_parser->eval( minStr );
		max = m_parser->eval( maxStr );
	}
}

void View::setScaling()
{
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };
	
	assert( (Settings::xScaling >= 0) && (Settings::xScaling() < 9) );
	assert( (Settings::yScaling >= 0) && (Settings::yScaling() < 9) );

	if( Settings::xScaling() == 8) //automatic x-scaling
    {
		tlgx = double(xmax-xmin)/16;
        tlgxstr = units[ Settings::xScaling() ];
// 		kDebug() << "xmax="<<xmax<<" xmin="<<xmin<<" tlgx="<<tlgx<<endl;
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
// 	kDebug() << "###############################" << k_funcinfo << endl;
	
	coordToMinMax( Settings::xRange(), Settings::xMin(), Settings::xMax(), xmin, xmax );
	coordToMinMax( Settings::yRange(), Settings::yMin(), Settings::yMax(), ymin, ymax );
	setScaling();
	
	m_parser->setAngleMode( Settings::anglemode() );

	backgroundcolor = Settings::backgroundcolor();
	if ( !backgroundcolor.isValid() )
		backgroundcolor = Qt::white;
	
	invertColor(backgroundcolor,inverted_backgroundcolor);
	
// 	setBackgroundColor(backgroundcolor);
	QPalette palette;
	palette.setColor( backgroundRole(), backgroundcolor );
	setPalette(palette);
}

void View::init()
{
	QList<int> functionIDs = m_parser->m_ufkt.keys();
	foreach ( int id, functionIDs )
		m_parser->delfkt( id );
	getSettings();
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
		for ( QList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression == str_parameter)
			{
				ufkt->setParameter( (*it).value );
				break;
			}
		}
	}

	isDrawing=true;
	updateCursor();

	double dx;
	if ( p_mode == Ufkt::Integral )
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
// 		paintEvent(0);
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
	while ((x>=dmin && x<=dmax) ||  (p_mode == Ufkt::Integral && x>=dmin && !forward_direction) || (p_mode == Ufkt::Integral && x<=dmax && forward_direction))
	{
		if ( p_mode == Ufkt::Integral && stop_calculating)
		{
			p_mode = Ufkt::Derivative1;
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
			kDebug() << "x " << x << endl;
			kDebug() << "y " << y << endl;
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
		if (p_mode==Ufkt::Integral)
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode== Ufkt::Integral )
				{
					forward_direction = false;
					x = ufkt->oldx = ufkt->startx;
					ufkt->oldy = ufkt->starty;
					ufkt->oldyprim = ufkt->integral_precision;
// 					paintEvent(0);
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
	updateCursor();
	
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
		for ( QList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
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
		updateCursor();
		bool target_found=false;
		startProgressBar((int) double((dmax-dmin)/dx)/2);
		x = ufkt->oldx = ufkt->startx; //the initial x-point
		ufkt->oldy = ufkt->starty;
		ufkt->oldyprim = ufkt->integral_precision;
// 		paintEvent(0);
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
// 					paintEvent(0);
				}
			}
			else
				x=x-dx; // go backwards
		}
		stopProgressBar();
		isDrawing=false;
		updateCursor();
		break;
	}
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
	
	if (isDrawing)
	{
		stop_calculating=true;
		return;
	}
	
	if (csmode==-1 )
		return;

	QMouseEvent *event;
	if (e->key() == Qt::Key_Left )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() - QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Right )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() + QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) //switch graph in trace mode
	{
		QMap<int, Ufkt*>::iterator it = m_parser->m_ufkt.find( csmode );
		int const ke=(*it)->parameters.count();
		if (ke>0)
		{
			csparam++;
			if (csparam >= ke)
				csparam=0;
		}
		if (csparam==0)
		{
			int const old_csmode=csmode;
			Ufkt::PMode const old_cstype = cstype;
			bool start = true;
			bool found = false;
			while ( 1 )
			{
				if ( old_csmode==csmode && !start)
				{
					cstype=old_cstype;
					break;
				}
				kDebug() << "csmode: " << csmode << endl;
				switch((*it)->fstr[0].latin1())
				{
				case 'x':
				case 'y':
				case 'r':
					break;
				default:
				{
					//going through the function, the first and the second derivative
					for ( cstype = (Ufkt::PMode)0; cstype < 3; cstype = (Ufkt::PMode)(cstype+1) )
// 					for (cstype=0;cstype<3;cstype++) 
					{
							if (start)
							{
								if ( cstype==Ufkt::Derivative2)
									cstype=Ufkt::Function;
								else
									cstype = (Ufkt::PMode)(old_cstype+1);
								start=false;
							}
							kDebug() << "   cstype: " << (int)cstype << endl;
						switch (cstype)
						{
							case Ufkt::Function:
								if ((*it)->f_mode )
									found=true;
								break;
							case Ufkt::Derivative1:
								if ( (*it)->f1_mode )
									found=true;
								break;
							case Ufkt::Derivative2:
								if ( (*it)->f2_mode )
									found=true;
								break;
							case Ufkt::Integral:
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

				if ( ++it == m_parser->m_ufkt.end())
					it = m_parser->m_ufkt.begin();
				csmode = (*it)->id;
			}
		}

		kDebug() << "************************" << endl;
		kDebug() << "csmode: " << (int)csmode << endl;
		kDebug() << "cstype: " << (int)cstype << endl;
		kDebug() << "csparam: " << csparam << endl;

		//change function in the statusbar
		switch (cstype )
		{
			case Ufkt::Function:
				setStatusBar((*it)->fstr,4);
				break;
				
			case Ufkt::Derivative1:
			{
				QString function = (*it)->fstr;
				function = function.left(function.indexOf('(')) + '\'';
				setStatusBar(function,4);
				break;
			}
			
			case Ufkt::Derivative2:
			{
				QString function = (*it)->fstr;
				function = function.left(function.indexOf('(')) + "\'\'";
				setStatusBar(function,4);
				break;
			}
			
			case Ufkt::Integral:
				break;
		}
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint(), Qt::LeftButton, Qt::LeftButton, 0 );
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

void View::areaUnderGraph( Ufkt *ufkt, Ufkt::PMode p_mode,  double &dmin, double &dmax, const QString &str_parameter, QPainter *DC )
{
	double x, y = 0;
	float calculated_area=0;
	double rectheight;
	areaMin = dmin;
	QPointF p;
	QColor color;
	switch(p_mode)
	{
		case Ufkt::Function:
			color = ufkt->color;
			break;
		case Ufkt::Derivative1:
			color = ufkt->f1_color;
			break;
		case Ufkt::Derivative2:
			color = ufkt->f2_color;
			break;
		case Ufkt::Integral:
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
		for ( QList<ParameterValueItem>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression == str_parameter)
			{
				ufkt->setParameter((*it).value);
				break;
			}
		}
	}
	double dx;
	if ( p_mode == Ufkt::Integral )
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


	double const origoy = dgr.TransyToPixel(0.0);
	double const rectwidth = dgr.TransxToPixel(dx)- dgr.TransxToPixel(0.0)+1;

	isDrawing=true;
	updateCursor();

	bool forward_direction;
	if (dmin<0 && dmax<0)
		forward_direction = false;
	else
		forward_direction = true;
	while ((x>=dmin && x<=dmax) ||  (p_mode == Ufkt::Integral && x>=dmin && !forward_direction) || (p_mode == Ufkt::Integral && x<=dmax && forward_direction))
	{
		if ( p_mode == Ufkt::Integral && stop_calculating)
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
			case Ufkt::Function:
				y=m_parser->fkt( ufkt, x);
				break;
	
			case Ufkt::Derivative1:
				y=m_parser->a1fkt( ufkt, x);
				break;
			case Ufkt::Derivative2:
				y=m_parser->a2fkt( ufkt, x);
				break;
			case Ufkt::Integral:
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

		p.setX(dgr.TransxToPixel(x));
		p.setY(dgr.TransyToPixel(y));
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
					DC->fillRect( QRectF( p.x(), p.y(), rectwidth, rectheight ), color);
				}
			}
			else
			{
				if ( y<0)
					rectheight =  origoy-p.y();
				else
					rectheight = -1*( p.y()-origoy);
				
				calculated_area = calculated_area + (dx*y);
				/*kDebug() << "Area: " << area << endl;
				kDebug() << "x:" << p.height() << endl;
				kDebug() << "y:" << p.y() << endl;
				kDebug() << "*************" << endl;*/
				
				DC->fillRect( QRectF( p.x(),p.y(),rectwidth,rectheight ), color );
			}
		}

		if ( p_mode == Ufkt::Integral )
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode == Ufkt::Integral )
				{
					forward_direction = false;
					x = ufkt->oldx = ufkt->startx;
					ufkt->oldy = ufkt->starty;
					ufkt->oldyprim = ufkt->integral_precision;
// 					paintEvent(0);
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
			updateCursor();
			return;
		}
	}
	isDrawing=false;
	updateCursor();


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

	// Why did I comment out negating the area?
	// There are two types of user - those who understand the concept of an
	// integral, and those that don't.
	// (1) Those who know what an integral is will expect to get a negative area
	// if it is calculated as such.
	// (2) Those who don't will probably be expecting the "negative" parts of
	// area (i.e. where f(x) < 0) to be included as positive as well. So the
	// integral (where f(x)<0) won't make sense for them anyway.
	
// 	if ( calculated_area>0)
		dmin = calculated_area;
// 	else
// 		dmin = calculated_area*-1; //don't answer with a negative number
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
// 	for(QVector<Ufkt>::iterator it=m_parser->ufkt.begin(); it!=m_parser->ufkt.end(); ++it)
	foreach ( Ufkt * it, m_parser->m_ufkt )
	{
		if (it->fname.isEmpty() ) continue;
		if( it->use_slider > -1  &&  (it->f_mode || it->f1_mode || it->f2_mode || it->integral_mode))
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
    if ( csmode == -1 )
      return;

	Ufkt *ufkt = m_parser->m_ufkt[ csmode ];
	switch (cstype )
	{
		case Ufkt::Function:
			ufkt->f_mode=0;
			break;
		case Ufkt::Derivative1:
			ufkt->f1_mode=0;
			break;
		case Ufkt::Derivative2:
			ufkt->f2_mode=0;
			break;
		case Ufkt::Integral:
			break;
	}
	mainDlg()->functionEditor()->functionsChanged();
	drawPlot();
	m_modified = true;
	updateSliders();
	if (csmode==-1)
		return;
	if ( !ufkt->f_mode && !ufkt->f1_mode && !ufkt->f2_mode) //all graphs for the function are hidden
	{
		csmode=-1;
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
    if ( csmode == -1 )
      return;

	Ufkt *ufkt =  m_parser->m_ufkt[ csmode ];
	QChar const function_type = ufkt->fstr[0];
	if (!m_parser->delfkt( ufkt ))
		return;

	if (csmode!=-1) // if trace mode is enabled
	{
		csmode=-1;
		QMouseEvent *event = new QMouseEvent( QMouseEvent::KeyPress, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, 0 );
		mousePressEvent(event); //leave trace mode
		delete event;
	}
		
	drawPlot();
	if ( function_type != 'x' &&  function_type != 'y' && function_type != 'r' )
		updateSliders();
	m_modified = true;
}
void View::mnuEdit_clicked()
{
	mainDlg()->functionEditor()->setCurrentFunction( csmode );
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
	if ( Settings::anglemode()==0 )
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
	Cursor newCursor;
	
	if ( isDrawing && (m_zoomMode != Translating) )
		newCursor = CursorWait;
	
	else switch (m_zoomMode)
	{
		case AnimatingZoom:
			newCursor = CursorArrow;
			break;
			
		case Normal:
			if ( shouldShowCrosshairs() )
				newCursor = CursorBlank;
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
	
	Ufkt * it = m_parser->functionWithID( csmode );
	
	QPoint mousePos = mapFromGlobal( QCursor::pos() );
	
	return ( underMouse() && area.contains( mousePos ) && (!it || csxposValid( it )) );
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
		QDataStream arg( &parameters,QIODevice::WriteOnly);
		arg.setVersion(QDataStream::Qt_3_1);
		arg << text << id;
		m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","setStatusBarText(QString,int)", parameters);
	}
}
void View::startProgressBar(int steps)
{
	QByteArray data;
	QDataStream stream( &data,QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_3_1);
	stream << steps;
	m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","startProgressBar(int)", data);
}
bool View::stopProgressBar()
{
	DCOPCString	 replyType;
	QByteArray replyData;
	m_dcop_client->call(m_dcop_client->appId(), "KmPlotShell","stopProgressBar()", QByteArray(), replyType, replyData);
	bool result;
	QDataStream stream( &replyData,QIODevice::ReadOnly);
	stream.setVersion(QDataStream::Qt_3_1);
	stream >> result;
	return result;
}
void View::increaseProgressBar()
{
	m_dcop_client->send(m_dcop_client->appId(), "KmPlotShell","increaseProgressBar()", QByteArray());
}

void View::slidersWindowClosed()
{
	m_menuSliderAction->setChecked(false);
}
