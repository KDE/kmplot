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


//BEGIN class View
View * View::m_self = 0;

View::View( bool readOnly, bool & modified, KMenu * functionPopup, QWidget* parent, KActionCollection *ac )
	: DCOPObject("View"),
	  QWidget( parent, Qt::WStaticContents ),
	  buffer( width(), height() ),
	  m_popupmenu( functionPopup ),
	  m_modified( modified ),
	  m_readonly( readOnly ),
	  m_dcop_client(KApplication::kApplication()->dcopClient()),
	  m_ac(ac)
{
	assert( !m_self ); // this class should only be constructed once
	m_self = this;
	
	m_currentFunctionID = m_currentFunctionParameter = -1;
	m_currentFunctionPlot = Function::Derivative0;
	m_drawIntegral = false;
	m_width = m_height = 0.0;
	m_scaler = 0.0;
	rootflg = false;
	tlgx = tlgy = drskalx = drskaly = 0.0;
	m_ymin = 0.0;
	m_ymax = 0.0;
	m_trace_x = 0.0;
	m_printHeaderTable = false;
	stop_calculating = false;
	m_minmax = 0;
	isDrawing = false;
	m_popupmenushown = 0;
	m_zoomMode = Normal;
	m_prevCursor = CursorArrow;
	
	m_mousePressTimer = new QTime();
	
	XParser::self( & modified );
	init();
	getSettings();
	
	setMouseTracking(TRUE);
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
	delete XParser::self();
	delete m_mousePressTimer;
}


void View::draw( QPaintDevice * dev, PlotMedium medium )
{
	double lx, ly;
	double sf;
	QRect rc;
	QPainter DC;    // our painter
	DC.begin(dev);    // start painting widget
	rc=DC.viewport();
	m_width = rc.width();
	m_height = rc.height();
	
	switch ( medium )
	{
		case Screen:
		{
			ref=QPoint(120, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			DC.scale( m_height/(ly+2*ref.y()), m_height/(ly+2*ref.y()));
			if ( ( QPointF(lx+2*ref.x(), ly) * DC.matrix() ).x() > DC.viewport().right())
			{
				DC.resetMatrix();
				DC.scale( m_width/(lx+2*ref.x()), m_width/(lx+2*ref.x()));
			}
			wm = DC.matrix();
			m_scaler=( QPoint(1000, 0) * DC.matrix() ).x()/1000.;
			dgr.Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			break;
		}
		
		case Printer:
		{
			sf=72./254.;        // 72dpi
			ref=QPoint(100, 100);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			DC.scale(sf, sf);
			m_scaler = 1.;
			m_printHeaderTable = ( ( KPrinter* ) dev )->option( "app-kmplot-printtable" ) != "-1";
			drawHeaderTable( &DC );
			dgr.Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			if ( ( (KPrinter* )dev )->option( "app-kmplot-printbackground" ) == "-1" )
				DC.fillRect( dgr.GetFrame(),  backgroundcolor); //draw a colored background
			//DC.end();
			//((QPixmap *)dev)->fill(QColor("#FF00FF"));
			//DC.begin(dev);
			break;
		}
		
		case SVG:
		{
			ref=QPoint(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			dgr.Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			DC.translate(-dgr.GetFrame().left(), -dgr.GetFrame().top());
			m_scaler=1.;
			break;
		}
		
		case Pixmap:
		{
			sf=180./254.;								// 180dpi
			ref=QPoint(0, 0);
			lx=((m_xmax-m_xmin)*100.*drskalx/tlgx);
			ly=((m_ymax-m_ymin)*100.*drskaly/tlgy);
			dgr.Create( ref, lx, ly, m_xmin, m_xmax, m_ymin, m_ymax );
			DC.end();
			*((QPixmap *)dev) = QPixmap( (int)(dgr.GetFrame().width()*sf), (int)(dgr.GetFrame().height()*sf) );
			((QPixmap *)dev)->fill(backgroundcolor);
			DC.begin(dev);
			DC.translate(-dgr.GetFrame().left()*sf, -dgr.GetFrame().top()*sf);
			DC.scale(sf, sf);
			m_scaler=1.;
			break;
		}
	}

	dgr.updateSettings();
// 	kDebug() << "tlgx="<<tlgx<<" tlgy="<<tlgy<<endl;
	dgr.Skal( tlgx, tlgy );

	DC.setRenderHint( QPainter::Antialiasing, true );
	dgr.Plot(&DC);
	
	PlotArea=dgr.GetPlotArea();
	area=DC.matrix().mapRect(PlotArea);
	
	isDrawing=true;
	updateCursor();
	stop_calculating = false;
	
	// Antialiasing slows down rendering a lot, so turn it off if we are
	// sliding the view about
	DC.setRenderHint( QPainter::Antialiasing, m_zoomMode != Translating );
// 	DC.setRenderHint( QPainter::Antialiasing, false );
// 	if ( m_zoomMode != Translating )
// 		kDebug() << "##############################\n";
	DC.setClipping( true );
	DC.setClipRect( PlotArea );
	foreach ( Function * ufkt, XParser::self()->m_ufkt )
	{
		if ( stop_calculating )
			break;
		
		plotfkt(ufkt, &DC);
	}
	DC.setClipping( false );

	isDrawing=false;
	updateCursor();
	DC.end();   // painting done
}


double View::value( Equation * eq, Function::PMode mode, double x )
{
	switch ( mode )
	{
		case Function::Derivative0:
			return XParser::self()->fkt( eq, x );
			
		case Function::Derivative1:
			return XParser::self()->a1fkt( eq, x, (m_xmax-m_xmin)/1e3 );
			
		case Function::Derivative2:
			return XParser::self()->a2fkt( eq, x, (m_xmax-m_xmin)/1e3 );
			
		case Function::Integral:
			return XParser::self()->euler_method( x, eq );
	}
	
	kWarning() << k_funcinfo << "Unknown mode!\n";
	return 0.0;
}


QPointF View::realValue( Function * function, Function::PMode mode, double x )
{
	assert( function );
	
	switch ( function->type() )
	{
		case Function::Cartesian:
		{
			double y = value( function->eq[0], mode, x );
			return QPointF( x, y );
		}
			
		case Function::Polar:
		{
			double y = value( function->eq[0], mode, x );
			return QPointF( y * cos(x), y * sin(x) );
		}
		
		case Function::Parametric:
		{
			double X = value( function->eq[0], mode, x );
			double Y = value( function->eq[1], mode, x );
			return QPointF( X, Y );
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
				
			case Function::Cartesian:
				max = m_xmax;
				break;
		}
	}
	
	if ( (function->type() == Function::Cartesian) && (max > m_xmax) )
		max = m_xmax;
	
	return max;
}


void View::plotfkt(Function *ufkt, QPainter *pDC)
{
	int k, ke, mflg;
	
	double dmin = getXmin( ufkt );
	double dmax = getXmax( ufkt );
	
	double base_dx = (dmax-dmin)/area.width();
	if ( (ufkt->type() == Function::Parametric) || (ufkt->type() == Function::Polar) )
		base_dx *= 0.01;
	
	// Increase speed while translating the view
	bool quickDraw = ( m_zoomMode == Translating );
	if ( quickDraw )
		base_dx *= 4.0;
	
	double dx = base_dx;
	
	Function::PMode p_mode = Function::Derivative0;
	
	double x;
	QPointF p1, p2;
	
	while(1)
	{
		pDC->setPen( penForPlot( ufkt, p_mode, pDC->renderHints() & QPainter::Antialiasing ) );
		
		k=0;
		ke=ufkt->parameters.count();
		do
		{
			bool drawIntegral = m_drawIntegral &&
					(m_integralDrawSettings.functionID == int(ufkt->id)) &&
					(m_integralDrawSettings.pMode == p_mode) &&
					((k >= ufkt->parameters.size()) || (m_integralDrawSettings.parameter == ufkt->parameters[k].expression()));
			
			if ( p_mode == Function::Derivative0 && !ufkt->f0.visible )
				break; // skip to the next one as function is hidden
			
			if ( p_mode == Function::Integral && stop_calculating)
				break;
			
			if( ufkt->use_slider == -1 )
			{
				if ( !ufkt->parameters.isEmpty() )
					ufkt->setParameter( ufkt->parameters[k].value() );
			}
			else
			{
				if ( m_sliderWindow )
					ufkt->setParameter( m_sliderWindow->value( ufkt->use_slider ) );
			}

			mflg=2;
			if ( p_mode == Function::Integral )
			{
				if ( ufkt->integral_use_precision )
// 					if ( Settings::useRelativeStepWidth() )
						dx =  ufkt->integral_precision*(dmax-dmin)/area.width();
// 					else
// 						dx =  ufkt->integral_precision;
				startProgressBar((int)double((dmax-dmin)/dx)/2);
				x = ufkt->eq[0]->oldx = ufkt->startx.value(); //the initial x-point
				ufkt->eq[0]->oldy = ufkt->starty.value();
				ufkt->eq[0]->oldyprim = ufkt->integral_precision;
			}
			else
				x=dmin;
			bool forward_direction;

			if (dmin<0 && dmax<0)
				forward_direction = false;
			else
				forward_direction = true;
			
			while ((x>=dmin && x<=dmax) ||  (p_mode == Function::Integral && x>=dmin && !forward_direction) || (p_mode == Function::Integral && x<=dmax && forward_direction))
			{
				if ( p_mode == Function::Integral && stop_calculating)
				{
					p_mode = Function::Derivative1;
					x=dmax+1;
					continue;
				}
				
				p2 = dgr.toPixel( realValue( ufkt, p_mode, x ) );
				
				if ( p_mode == Function::Integral && (int(x*100)%2==0) )
				{
					KApplication::kApplication()->processEvents(); //makes the program usable when drawing a complicated integral function
					increaseProgressBar();
				}
				
				bool dxAtMinimum = (dx <= base_dx*(5e-5));
				bool dxAtMaximum = (dx >= base_dx*(5e+1));
				bool dxTooBig = false;
				bool dxTooSmall = false;
					
				if ( dgr.xclipflg || dgr.yclipflg )
				{
					p1=p2;
				}
				else
				{
					if ( (mflg<=1) && ((ufkt->type() == Function::Parametric) || (ufkt->type() == Function::Polar)) )
					{
						QPointF p1_pixel = p1 * pDC->matrix();
						QPointF p2_pixel = p2 * pDC->matrix();
						QRectF bound = QRectF( p1_pixel, QSizeF( (p2_pixel-p1_pixel).x(), (p2_pixel-p1_pixel).y() ) ).normalized();
							
						if ( QRectF( area ).intersects( bound ) )
						{
							double length = QLineF( p1_pixel, p2_pixel ).length();
							dxTooBig = !dxAtMinimum && (length > (quickDraw ? 40.0 : 4.0));
							dxTooSmall = !dxAtMaximum && (length < (quickDraw ? 10.0 : 1.0));
// 							kDebug() << "p1_pixel="<<p1_pixel.toPoint()<<" p2_pixel="<<p2_pixel.toPoint()<<" tooBig="<<dxTooBig<<" tooSmall="<<dxTooSmall<<" dx="<<dx<<" length="<<length<<endl;
						}
						else
						{
// 							kDebug() << "area="<< area << " bound="<<bound.toRect()<<endl;
							dxTooSmall = !dxAtMaximum;
						}
					}
						
					if ( !dxTooBig )
					{
						if(mflg<=1)
						{
							if ( drawIntegral && (x >= m_integralDrawSettings.dmin) && (x <= m_integralDrawSettings.dmax) )
								pDC->drawRect( QRectF( p1, QSizeF( p2.x()-p1.x(), p2.y() - dgr.yToPixel( 0 ) ) ) );
							else
								pDC->drawLine( p1, p2 );
						}
						p1=p2;
					}
					mflg=0;
				}

				if ( p_mode == Function::Integral )
				{
					if ( forward_direction)
					{
						x=x+dx;
						if (x>dmax && p_mode == Function::Integral )
						{
							forward_direction = false;
							x = ufkt->eq[0]->oldx = ufkt->startx.value();
							ufkt->eq[0]->oldy = ufkt->starty.value();
							ufkt->eq[0]->oldyprim = ufkt->integral_precision;
							mflg=2;
						}
					}
					else
						x=x-dx; // go backwards
				}
				else
				{
					if ( dxTooBig )
						dx *= 0.5;
					else
					{
						if ( dxTooSmall )
							dx *= 2.0;
						x=x+dx;
					}
				}
			}
		}
		while(++k<ke);

		// Advance to the next appropriate p_mode
		if (ufkt->f1.visible==1 && p_mode< Function::Derivative1)
			p_mode=Function::Derivative1; //draw the 1st derivative
		else if(ufkt->f2.visible==1 && p_mode< Function::Derivative2)
			p_mode=Function::Derivative2; //draw the 2nd derivative
		else if( ufkt->integral.visible==1 && p_mode < Function::Integral)
			p_mode=Function::Integral; //draw the integral
		else
			break; // no derivatives or integrals left to draw
	}
	if ( stopProgressBar() )
		if( stop_calculating)
			KMessageBox::error(this,i18n("The drawing was cancelled by the user."));
}


QPen View::penForPlot( Function *ufkt, Function::PMode p_mode, bool antialias ) const
{
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);
// 	pen.setStyle( Qt::DashLine );
	
	double lineWidth_mm;
	
	switch ( p_mode )
	{
		case Function::Derivative0:
			lineWidth_mm = ufkt->f0.lineWidth;
			pen.setColor(ufkt->f0.color);
			break;
			
		case Function::Derivative1:
			lineWidth_mm = ufkt->f1.lineWidth;
			pen.setColor(ufkt->f1.color);
			break;
			
		case Function::Derivative2:
			lineWidth_mm = ufkt->f2.lineWidth;
			pen.setColor(ufkt->f2.color);
			break;
			
		case Function::Integral:
			lineWidth_mm = ufkt->integral.lineWidth;
			pen.setColor(ufkt->integral.color);
			break;
	}
	
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
		//for(uint ix=0; ix<XParser::self()->countFunctions() && !stop_calculating; ++ix)
// 		for(QVector<Function>::iterator it=XParser::self()->ufkt.begin(); it!=XParser::self()->ufkt.end() && !stop_calculating; ++it)
		foreach ( Function * it, XParser::self()->m_ufkt )
		{
			for ( unsigned i = 0; i < 2; ++ i )
			{
				if ( stop_calculating )
					break;
				
				if ( !it->eq[i] )
					continue;
				
				QString fstr = it->eq[i]->fstr();
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
	}
}


void View::setpi(QString *s)
{
	int i;
	QChar c(960);

	while((i=s->indexOf('p')) != -1)
		s->replace(i, 2, &c, 1);
}


bool View::root(double *x0, Equation *it)
{
	if(rootflg)
		return FALSE;
	
	int k = 0; // iteration count
	int max_k = 50; // maximum number of iterations
	double max_y = 1e-14; // the largest value of y which is deemed a root found
	
	*x0 = m_crosshairPosition.x();
	double y = m_crosshairPosition.y();
	bool tooBig = true;
	
// 	kDebug() << "Initial: ("<<*x0<<","<<y<<")\n";
	
	do
	{
		double df = XParser::self()->a1fkt( it, *x0, (m_xmax-m_xmin)/1e3 );
// 		kDebug() << "df1="<<df<<endl;
// 		if ( qAbs(df) < 1e-6 )
// 			df = 1e-6 * ((df < 0) ? -1 : 1);
// 		kDebug() << "df2="<<df<<endl;
		
		*x0 -= y / df;
		y = XParser::self()->fkt( it, *x0 );
		
// 		kDebug() << "k="<<k<<": ("<<*x0<<","<<y<<")\n";
		
		tooBig = (qAbs(y) > max_y);
	}
	while ( (k++<max_k) && tooBig );
	
	// We continue calculating until |y| < max_y; this may result in k reaching
	// max_k. However, if |y| is reasonably small (even if reaching max_k),
	// we consider it a root.
	return ( qAbs(y) < 1e-6 );
}

void View::paintEvent(QPaintEvent *)
{
	QPainter p;
	p.begin(this);
	
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
		QPointF tl( dgr.xToPixel( m_animateZoomRect.left() ), dgr.yToPixel( m_animateZoomRect.top() ) );
		QPointF br( dgr.xToPixel( m_animateZoomRect.right() ), dgr.yToPixel( m_animateZoomRect.bottom() ) );
		p.drawRect( QRectF( tl, QSizeF( br.x()-tl.x(), br.y()-tl.y() ) ) );
		p.restore();
	}
	else if ( shouldShowCrosshairs() )
	{
		updateCrosshairPosition();
		
		Function * it = XParser::self()->functionWithID( m_currentFunctionID );
			
			// Fadenkreuz zeichnen [draw the cross-hair]
		QPen pen;
		if ( !it )
			pen.setColor(inverted_backgroundcolor);
		else
		{
			switch (m_currentFunctionPlot)
			{
				case Function::Derivative0:
					pen.setColor( it->f0.color);
					break;
				case Function::Derivative1:
					pen.setColor( it->f1.color);
					break;
				case Function::Derivative2:
					pen.setColor( it->f2.color);
					break;
				case Function::Integral:
					pen.setColor( it->integral.color);
					break;
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
	
	if ( m_popupmenushown>0)
		return;

	if (isDrawing)
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
			updateCursor();
			update();
		}
		return;
	}
	
	rootflg = false;
	
	bool hadFunction = (m_currentFunctionID != -1 );
	
	updateCrosshairPosition();
	
	if( !m_readonly && e->button()==Qt::RightButton) //clicking with the right mouse button
	{
		getPlotUnderMouse();
		Function * function = XParser::self()->functionWithID( m_currentFunctionID );
		if ( function )
		{
			QString popupTitle;
			
			if ( function->type() == Function::Parametric )
			{
				popupTitle = function->eq[0]->fstr() + ";" + function->eq[1]->fstr();
			}
			else switch ( m_currentFunctionPlot )
			{
				case Function::Derivative0:
					popupTitle = function->eq[0]->fstr();
					break;
					
				case Function::Derivative1:
					popupTitle = function->eq[0]->fname() + "\'";
					break;
					
				case Function::Derivative2:
					popupTitle = function->eq[0]->fname() + "\'\'";
					break;
					
				case Function::Integral:
					popupTitle = function->eq[0]->fname().toUpper();
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
	
	if(m_currentFunctionID>=0) //disable trace mode if trace mode is enable
	{
		m_currentFunctionID=-1;
		setStatusBar("",3);
		setStatusBar("",4);
		mouseMoveEvent(e);
		return ;
	}
	
	getPlotUnderMouse();
	Function * function = XParser::self()->functionWithID( m_currentFunctionID );
	if ( function )
	{
		m_minmax->selectItem();
		
		if ( function->type() == Function::Parametric )
			setStatusBar( function->eq[0]->fstr() + ';' + function->eq[1]->fstr(), 4 );
		
		else if ( function->type() == Function::Polar )
			setStatusBar(function->eq[0]->fstr(),4);
		
		else
		{
			// cartesian plot
		
			switch ( m_currentFunctionPlot )
			{
				case Function::Derivative0:
					setStatusBar(function->eq[0]->fstr(),4);
					break;
			
				case Function::Derivative1:
					setStatusBar( function->eq[0]->fname() + "\'", 4 );
					break;
			
				case Function::Derivative2:
					setStatusBar( function->eq[0]->fname() + "\'\'", 4 );
					break;
			
				case Function::Integral:
					// can't trace integral
					return;
			}
		}
			
		// csxpos, csypos would have been set by getPlotUnderMouse()
		QPointF ptd( dgr.toPixel( m_crosshairPosition ) );
		QPoint globalPos = mapToGlobal( (ptd * wm).toPoint() );
		QCursor::setPos( globalPos );
		return;
	}
	
	// user didn't click on a plot; so we prepare to enter translation mode
	m_currentFunctionID=-1;
	m_zoomMode = AboutToTranslate;
	m_prevDragMousePos = e->pos();
}


void View::getPlotUnderMouse()
{
	m_currentFunctionID = -1;
	m_currentFunctionParameter = 0;
	m_currentFunctionPlot = Function::Derivative0;
	m_trace_x = 0.0;
	
	int best_id;
	double best_distance = 1e30; // a nice large number
	QPointF best_cspos;
	
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		int k=0;
		int const ke=it->parameters.count();
		do
		{
			if( it->use_slider == -1 )
			{
				if ( !it->parameters.isEmpty())
					it->setParameter( it->parameters[k].value() );
			}
			else
			{
				if ( m_sliderWindow )
					it->setParameter(  m_sliderWindow->value( it->use_slider ) );
			}
			
			for ( int i = Function::Derivative0; i <= Function::Derivative2; ++i )
			{
				switch ( i )
				{
					case Function::Derivative0:
						if ( !it->f0.visible )
							continue;
						break;
							
					case Function::Derivative1:
						if ( !it->f1.visible || (it->type() != Function::Cartesian) )
							continue;
						break;
							
					case Function::Derivative2:
						if ( !it->f2.visible || (it->type() != Function::Cartesian) )
							continue;
						break;
				}
					
				double best_x = getClosestPoint( m_crosshairPosition, it, (Function::PMode)i );
				double distance = pixelDistance( m_crosshairPosition, it, (Function::PMode)i, best_x );
					
				if ( distance < best_distance )
				{
					best_distance = distance;
					best_id = it->id;
					m_currentFunctionPlot = (Function::PMode)i;
					m_currentFunctionParameter = k;
					m_trace_x = best_x;
					best_cspos = realValue( it, m_currentFunctionPlot, best_x );
				}
			}
		}
		while(++k<ke);
	}
	
	if ( best_distance < 30.0 )
	{
		m_currentFunctionID = best_id;
		m_crosshairPosition = best_cspos;
	}
}


double View::getClosestPoint( const QPointF & pos, Function * function, Function::PMode mode )
{
	double best_x = 0.0;
	
	if ( function->type() == Function::Cartesian )
	{
		double best_pixel_x = 0.0;
		
		QPointF pixelPos = dgr.toPixel( pos, CDiagr::ClipInfinite );
		
		double dmin = getXmin( function );
		double dmax = getXmax( function );
		
		double stepSize = (m_xmax-m_xmin)/1e3;
		
		// Algorithm in use here: Work out the shortest distance between the
		// line joining (x0,y0) to (x1,y1) and the given point (real_x,real_y)
		
		double x = dmin;
		double y0 = value( function->eq[0], mode, x );
		
		double best_distance = 1e20; // a large distance
		
		while ( x <= dmax )
		{
			x += stepSize;
			
			double y1 = XParser::self()->fkt( function->eq[0], x );
			
			double _x0 = dgr.xToPixel( x-stepSize, CDiagr::ClipInfinite );
			double _x1 = dgr.xToPixel( x, CDiagr::ClipInfinite );
			
			double _y0 = dgr.yToPixel( y0, CDiagr::ClipInfinite );
			double _y1 = dgr.yToPixel( y1, CDiagr::ClipInfinite );
			
			double k = (_y1-_y0)/(_x1-_x0);
			
			double closest_x;
			if ( k == 0 )
				closest_x = _x0;
			else
				closest_x = (pixelPos.y() + pixelPos.x()/k + k*_x0 - _y0) / (k + 1.0/k);
			
			double closest_y = dgr.yToPixel( value( function->eq[0], mode, dgr.xToReal( closest_x ) ), CDiagr::ClipInfinite );
			
			double dfx = qAbs( closest_x - pixelPos.x() );
			double dfy = qAbs( closest_y - pixelPos.y() );
			
			double distance = sqrt( dfx*dfx + dfy*dfy );
			if ( distance < best_distance )
			{
				best_distance = distance;
				best_pixel_x = closest_x;
			}
		}
		
		best_x = dgr.xToReal( best_pixel_x );
	}
	else
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
				double distance = pixelDistance( pos, function, mode, x );
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


double View::pixelDistance( const QPointF & pos, Function * function, Function::PMode mode, double x )
{
	QPointF f = realValue( function, mode, x );
	QPointF df = dgr.toPixel( pos, CDiagr::ClipInfinite ) - dgr.toPixel( f, CDiagr::ClipInfinite );
					
	return std::sqrt( df.x()*df.x() + df.y()*df.y() );
}


QString View::posToString( double x, double delta ) const
{
	assert( delta != 0.0 );
	
	int decimalPlaces = 1-int(log(delta)/log(10.0));
	
	QString number;
	if ( decimalPlaces >= 0 )
		number = QString::number( x, 'f', decimalPlaces );
	else
		number = QString::number( x/(pow(10.0,decimalPlaces)), 'f', 0 ) + QString( -decimalPlaces, '0' );
	
	if ( x > 0.0 )
		number.prepend('+');
	
	return number;
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
		sx = "x = " + posToString( m_crosshairPosition.x(), (m_xmax-m_xmin)/1e3 );
		sy = "y = " + posToString( m_crosshairPosition.y(), (m_ymax-m_ymin)/1e3 );
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
			m_currentFunctionID=-1;
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
	m_crosshairPosition = dgr.toReal( ptl );
	
	Function * it = XParser::self()->functionWithID( m_currentFunctionID );
	
	if ( it && crosshairPositionValid( it ) )
	{
		// The user currently has a plot selected, with the mouse in a valid position
		
		if( it->use_slider == -1 )
		{
			if( !it->parameters.isEmpty() )
				it->setParameter( it->parameters[m_currentFunctionParameter].value() );
		}
		else
		{
			if ( m_sliderWindow )
				it->setParameter( m_sliderWindow->value( it->use_slider ) );
		}
		
		if ( (it->type() == Function::Parametric) ||
					(it->type() == Function::Polar) )
		{
			
			// Should we increase or decrease t to get closer to the mouse?
			double dx[2] = { -0.00001, +0.00001 };
			double d[] = { 0.0, 0.0 };
			for ( int i = 0; i < 2; ++ i )
				d[i] = pixelDistance( m_crosshairPosition, it, Function::Derivative0, m_trace_x + dx[i] );
			
			double prev_best = pixelDistance( m_crosshairPosition, it, Function::Derivative0, m_trace_x );
			double current_dx = dx[(d[0] < d[1]) ? 0 : 1]*1e3;
			
			while ( true )
			{	
				double new_distance = pixelDistance( m_crosshairPosition, it, Function::Derivative0, m_trace_x + current_dx );
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
			
			m_crosshairPosition = realValue( it, Function::Derivative0, m_trace_x );
		}
		else
		{
			// cartesian plot
			
			m_crosshairPosition.setY( value( it->eq[0], m_currentFunctionPlot, m_crosshairPosition.x() ) );
			ptl.setY(dgr.yToPixel( m_crosshairPosition.y() ));

			if ( m_crosshairPosition.y()<m_ymin || m_crosshairPosition.y()>m_ymax) //the ypoint is not visible
			{
				out_of_bounds = true;
			}
			else if(fabs(dgr.yToReal(ptl.y())) < (m_ymax-m_ymin)/80)
			{
				double x0;
				if ( root( &x0, it->eq[0] ) )
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
		
		ptl = dgr.toPixel( m_crosshairPosition );
		QPoint globalPos = mapToGlobal( (ptl * wm).toPoint() );
		QCursor::setPos( globalPos );
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
	double realx = dgr.xToReal((mousePos * wm.inverted()).x());
	double realy = dgr.yToReal((mousePos * wm.inverted()).y());

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
	double real1x = dgr.xToReal(p.x() );
	double real1y = dgr.yToReal(p.y() );
	p = rect.bottomRight();
	double real2x = dgr.xToReal(p.x() );
	double real2y = dgr.yToReal(p.y() );
	
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
	double _real1x = dgr.xToReal(p.x() );
	double _real1y = dgr.yToReal(p.y() );
	p = rect.bottomRight();
	double _real2x = dgr.xToReal(p.x() );
	double _real2y = dgr.yToReal(p.y() );
	
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
	double rdx = dgr.xToReal( dx / m_scaler ) - dgr.xToReal( 0.0 );
	double rdy = dgr.yToReal( dy / m_scaler ) - dgr.yToReal( 0.0 );
	
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
	}
}

void View::setScaling()
{
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };
	
	assert( (Settings::xScaling >= 0) && (Settings::xScaling() < 9) );
	assert( (Settings::yScaling >= 0) && (Settings::yScaling() < 9) );

	if( Settings::xScaling() == 8) //automatic x-scaling
    {
		tlgx = double(m_xmax-m_xmin)/16;
        tlgxstr = units[ Settings::xScaling() ];
    }
	else
	{
		tlgxstr = units[ Settings::xScaling() ];
		tlgx = XParser::self()->eval( tlgxstr );
	}

	if( Settings::yScaling() == 8)  //automatic y-scaling
    {
		tlgy = double(m_ymax-m_ymin)/16;
        tlgystr = units[ Settings::yScaling() ];
    }
	else
	{
		tlgystr = units[ Settings::yScaling() ];
		tlgy = XParser::self()->eval( tlgystr );
	}

	drskalxstr = units[ Settings::xPrinting() ];
	drskalx = XParser::self()->eval( drskalxstr );
	drskalystr = units[ Settings::yPrinting() ];
	drskaly = XParser::self()->eval( drskalystr );
}

void View::getSettings()
{
// 	kDebug() << "###############################" << k_funcinfo << endl;
	
	coordToMinMax( Settings::xRange(), Settings::xMin(), Settings::xMax(), m_xmin, m_xmax );
	coordToMinMax( Settings::yRange(), Settings::yMin(), Settings::yMax(), m_ymin, m_ymax );
	setScaling();
	
	XParser::self()->setAngleMode( (Parser::AngleMode)Settings::anglemode() );

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
	QList<int> functionIDs = XParser::self()->m_ufkt.keys();
	foreach ( int id, functionIDs )
		XParser::self()->removeFunction( id );
	getSettings();
}


void View::stopDrawing()
{
	if (isDrawing)
		stop_calculating = true;
}

QPointF View::findMinMaxValue(Function *ufkt, Function::PMode p_mode, ExtremaType type, double dmin, double dmax, const QString &str_parameter)
{
	assert( ufkt->type() == Function::Cartesian );
	
	double x = 0;
	double y = 0;
	double result_x = 0;
	double result_y = 0;
	bool start = true;

	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QList<Value>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression() == str_parameter)
			{
				ufkt->setParameter( (*it).value() );
				break;
			}
		}
	}

	double dx;
	if ( p_mode == Function::Integral )
	{
		if ( ufkt->integral_use_precision )
		{
			if ( ufkt->integral_use_precision )
				dx = ufkt->integral_precision*(dmax-dmin)/area.width();
			else
				dx = ufkt->integral_precision;
		}
		else
		{
			if ( ufkt->integral_use_precision )
				dx = (dmax-dmin)/area.width();
			else
				dx = 1.0;
		}
		
		dmin = ufkt->eq[0]->oldx = ufkt->startx.value(); //the initial x-point
		ufkt->eq[0]->oldy = ufkt->starty.value();
		ufkt->eq[0]->oldyprim = ufkt->integral_precision;
	}
	else
		dx = (dmax-dmin)/area.width();
	
	x=dmin;

	bool forward_direction;
	if (dmin<0 && dmax<0)
		forward_direction = false;
	else
		forward_direction = true;
	while ((x>=dmin && x<=dmax) ||  (p_mode == Function::Integral && x>=dmin && !forward_direction) || (p_mode == Function::Integral && x<=dmax && forward_direction))
	{
		if ( p_mode == Function::Integral && stop_calculating)
		{
			p_mode = Function::Derivative1;
			x=dmax+1;
			continue;
		}
		y = value( ufkt->eq[0], p_mode, x );
		
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
		if (p_mode==Function::Integral)
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>dmax && p_mode== Function::Integral )
				{
					forward_direction = false;
					x = ufkt->eq[0]->oldx = ufkt->startx.value();
					ufkt->eq[0]->oldy = ufkt->starty.value();
					ufkt->eq[0]->oldyprim = ufkt->integral_precision;
				}
			}
			else
				x=x-dx; // go backwards
		}
		else
			x=x+dx;
	}
	
	return QPointF( result_x, result_y );
}


double View::getYValue( Function *ufkt, Function::PMode p_mode, double x, double y, const QString &str_parameter )
{
	assert( ufkt->type() == Function::Cartesian );
	
	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QList<Value>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression() == str_parameter)
			{
				ufkt->setParameter( (*it).value() );
				break;
			}
		}
	}
	
	if ( p_mode != Function::Integral )
		return value( ufkt->eq[0], p_mode, x );
	
	double dmin = getXmin( ufkt );
	double dmax = getXmax( ufkt );
	const double target = x; //this is the x-value the user had chosen
	bool forward_direction;
	if ( target>=0)
		forward_direction = true;
	else
		forward_direction = false;

	if(dmin==dmax) //no special plot range is specified. Use the screen border instead.
	{
		dmin=m_xmin;
		dmax=m_xmax;
	}

	double dx;
	if ( ufkt->integral_use_precision )
		dx = ufkt->integral_precision*(dmax-dmin)/area.width();
	else
		dx=(dmax-dmin)/area.width();
	
	x = ufkt->eq[0]->oldx = ufkt->startx.value(); //the initial x-point
	ufkt->eq[0]->oldy = ufkt->starty.value();
	ufkt->eq[0]->oldyprim = ufkt->integral_precision;
	
	while ( x>=dmin )
	{
		y = XParser::self()->euler_method( x, ufkt->eq[0] );
		if ( (x+dx > target && forward_direction) || ( x+dx < target && !forward_direction)) //right x-value is found
			return y;
		
		if (forward_direction)
		{
			x=x+dx;
			if (x>dmax)
			{
				forward_direction = false;
				x = ufkt->eq[0]->oldx = ufkt->startx.value();
				ufkt->eq[0]->oldy = ufkt->starty.value();
				ufkt->eq[0]->oldyprim = ufkt->integral_precision;
			}
		}
		else
			x=x-dx; // go backwards
	}
	
	kWarning() << k_funcinfo << "Could not find y value!\n";
	return 0.0;
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
	
	if (m_currentFunctionID==-1 )
		return;

	QMouseEvent *event;
	if (e->key() == Qt::Key_Left )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() - QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Right )
		event = new QMouseEvent( QEvent::MouseMove, m_crosshairPixelCoords.toPoint() + QPoint(1,1), Qt::LeftButton, Qt::LeftButton, 0 );
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) //switch graph in trace mode
	{
		QMap<int, Function*>::iterator it = XParser::self()->m_ufkt.find( m_currentFunctionID );
		int const ke=(*it)->parameters.count();
		if (ke>0)
		{
			m_currentFunctionParameter++;
			if (m_currentFunctionParameter >= ke)
				m_currentFunctionParameter=0;
		}
		if (m_currentFunctionParameter==0)
		{
			int const old_m_currentFunctionID=m_currentFunctionID;
			Function::PMode const old_m_currentFunctionPlot = m_currentFunctionPlot;
			bool start = true;
			bool found = false;
			while ( 1 )
			{
				if ( old_m_currentFunctionID==m_currentFunctionID && !start)
				{
					m_currentFunctionPlot=old_m_currentFunctionPlot;
					break;
				}
				kDebug() << "m_currentFunctionID: " << m_currentFunctionID << endl;
				switch ( (*it)->type() )
				{
					case Function::Parametric:
				case Function::Polar:
					break;
				default:
				{
					//going through the function, the first and the second derivative
					for ( m_currentFunctionPlot = (Function::PMode)0; m_currentFunctionPlot < 3; m_currentFunctionPlot = (Function::PMode)(m_currentFunctionPlot+1) )
// 					for (m_currentFunctionPlot=0;m_currentFunctionPlot<3;m_currentFunctionPlot++) 
					{
							if (start)
							{
								if ( m_currentFunctionPlot==Function::Derivative2)
									m_currentFunctionPlot=Function::Derivative0;
								else
									m_currentFunctionPlot = (Function::PMode)(old_m_currentFunctionPlot+1);
								start=false;
							}
							kDebug() << "   m_currentFunctionPlot: " << (int)m_currentFunctionPlot << endl;
						switch (m_currentFunctionPlot)
						{
							case Function::Derivative0:
								if ((*it)->f0.visible )
									found=true;
								break;
							case Function::Derivative1:
								if ( (*it)->f1.visible )
									found=true;
								break;
							case Function::Derivative2:
								if ( (*it)->f2.visible )
									found=true;
								break;
							case Function::Integral:
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

				if ( ++it == XParser::self()->m_ufkt.end())
					it = XParser::self()->m_ufkt.begin();
				m_currentFunctionID = (*it)->id;
			}
		}

		kDebug() << "************************" << endl;
		kDebug() << "m_currentFunctionID: " << (int)m_currentFunctionID << endl;
		kDebug() << "m_currentFunctionPlot: " << (int)m_currentFunctionPlot << endl;
		kDebug() << "m_currentFunctionParameter: " << m_currentFunctionParameter << endl;

		//change function in the statusbar
		switch (m_currentFunctionPlot )
		{
			case Function::Derivative0:
			{
				QString text = (*it)->eq[0]->fstr();
				QString fstr1 = (*it)->eq[1]->fstr();
				if ( !fstr1.isEmpty() )
					text += ";" + fstr1;
				setStatusBar( text, 4 );
				break;
			}
				
			case Function::Derivative1:
			{
				setStatusBar( (*it)->eq[0]->fname() + "\'", 4 );
				break;
			}
			
			case Function::Derivative2:
			{
				setStatusBar( (*it)->eq[0]->fname() + "\'\'", 4 );
				break;
			}
			
			case Function::Integral:
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


double View::areaUnderGraph( IntegralDrawSettings s )
{
	assert( s.dmin < s.dmax );
	
	Function * ufkt = XParser::self()->functionWithID( s.functionID );
	assert( ufkt );

	// TODO: parameter sliders
	if ( !ufkt->parameters.isEmpty() )
	{
		for ( QList<Value>::Iterator it = ufkt->parameters.begin(); it != ufkt->parameters.end(); ++it )
		{
			if ( (*it).expression() == s.parameter )
			{
				ufkt->setParameter( (*it).value() );
				break;
			}
		}
	}
	
	double dx;
	if ( s.pMode == Function::Integral )
	{
		if ( ufkt->integral_use_precision )
			dx = ufkt->integral_precision*(s.dmax-s.dmin)/area.width();
		else
			dx = (s.dmax-s.dmin)/area.width();
		
		s.dmin = ufkt->eq[0]->oldx = ufkt->startx.value(); //the initial x-point
		ufkt->eq[0]->oldy = ufkt->starty.value();
		ufkt->eq[0]->oldyprim = ufkt->integral_precision;
	}
	else
		dx = (s.dmax-s.dmin)/area.width();
	
	// Make sure that we calculate the exact area (instead of missing out a
	// vertical slither at the end) by making sure dx tiles the x-range
	// a whole number of times
	int intervals = qRound( (s.dmax-s.dmin)/dx );
	dx = (s.dmax-s.dmin) / intervals;
	
	bool forward_direction = (s.dmin>=0) || (s.dmax>=0);
	double calculated_area=0;
	double x = s.dmin;
	
// 	while ( (x>=dmin && x<=dmax) ||  (p_mode == Function::Integral && x>=dmin && !forward_direction) || (p_mode == Function::Integral && x<=dmax && forward_direction))
// 	{
	for ( int i = 0; i <= intervals; ++i )
	{
		double y = value( ufkt->eq[0], s.pMode, x );
		
		// Trapezoid rule for integrals: only add on half for the first and last value
		if ( (i == 0) || (i == intervals) )
			calculated_area += 0.5*dx*y;
		else
			calculated_area += dx*y;

		if ( s.pMode == Function::Integral )
		{
			if ( forward_direction)
			{
				x=x+dx;
				if (x>s.dmax && s.pMode == Function::Integral )
				{
					forward_direction = false;
					x = ufkt->eq[0]->oldx = ufkt->startx.value();
					ufkt->eq[0]->oldy = ufkt->starty.value();
					ufkt->eq[0]->oldyprim = ufkt->integral_precision;
				}
			}
			else
				x=x-dx; // go backwards
		}
		else
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
// 	for(QVector<Function>::iterator it=XParser::self()->ufkt.begin(); it!=XParser::self()->ufkt.end(); ++it)
	foreach ( Function * it, XParser::self()->m_ufkt )
	{
		if( it->use_slider > -1  &&  (it->f0.visible || it->f1.visible || it->f2.visible || it->integral.visible))
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
    if ( m_currentFunctionID == -1 )
      return;

	Function *ufkt = XParser::self()->m_ufkt[ m_currentFunctionID ];
	switch (m_currentFunctionPlot )
	{
		case Function::Derivative0:
			ufkt->f0.visible=0;
			break;
		case Function::Derivative1:
			ufkt->f1.visible=0;
			break;
		case Function::Derivative2:
			ufkt->f2.visible=0;
			break;
		case Function::Integral:
			break;
	}
	MainDlg::self()->functionEditor()->functionsChanged();
	drawPlot();
	m_modified = true;
	updateSliders();
	if (m_currentFunctionID==-1)
		return;
	if ( !ufkt->f0.visible && !ufkt->f1.visible && !ufkt->f2.visible) //all graphs for the function are hidden
	{
		m_currentFunctionID=-1;
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
    if ( m_currentFunctionID == -1 )
      return;

	Function *ufkt =  XParser::self()->m_ufkt[ m_currentFunctionID ];
	Function::Type function_type = ufkt->type();
	if (!XParser::self()->removeFunction( ufkt ))
		return;

	if (m_currentFunctionID!=-1) // if trace mode is enabled
	{
		m_currentFunctionID=-1;
		QMouseEvent *event = new QMouseEvent( QMouseEvent::KeyPress, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, 0 );
		mousePressEvent(event); //leave trace mode
		delete event;
	}
		
	drawPlot();
	if ( function_type == Function::Cartesian )
		updateSliders();
	m_modified = true;
}
void View::mnuEdit_clicked()
{
	MainDlg::self()->functionEditor()->setCurrentFunction( m_currentFunctionID );
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
	
	Function * it = XParser::self()->functionWithID( m_currentFunctionID );
	
	QPoint mousePos = mapFromGlobal( QCursor::pos() );
	
	return ( underMouse() && area.contains( mousePos ) && (!it || crosshairPositionValid( it )) );
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
//END class View



//BEGIN class IntegralDrawSettings
IntegralDrawSettings::IntegralDrawSettings()
{
	functionID = -1;
	pMode = Function::Derivative0;
	dmin = dmax = 0.0;
}
//END class IntegralDrawSettings

