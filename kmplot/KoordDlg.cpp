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
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qcheckbox.h>

// KDE includes
#include <kcolorbutton.h>
#include <kmessagebox.h>
#include <knuminput.h>

// local includes
#include "KoordDlg.h"
#include "KoordDlg.moc"
#include "settings.h"


#define Inherited KoordDlgData

KoordDlg::KoordDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
	le_xmin->setText( xminstr );
	le_xmax->setText( xmaxstr );
	le_ymin->setText( yminstr );
	le_ymax->setText( ymaxstr );
	kdx = koordx;
	kdy = koordy;
	setAxes();
	cb_beschr->setChecked( mode & LABEL );
	axesLineWidth->setValue( axesThickness );
	ticLineWidth->setValue( gradThickness );
	ticLength->setValue( gradLength );
	color_button->setColor( QColor( axesColor ) );
}

KoordDlg::~KoordDlg()
{
}

void KoordDlg::setAxes()
{
	bg_xachse->setButton( kdx );
	bg_yachse->setButton( kdy );
}

int KoordDlg::wertholen( double& w, QLineEdit *le )
{
	w = ps.eval( le->text() );
	if ( ps.err != 0 )
	{
		ps.errmsg();
		errflg = 1;
		return -1;
	}
	return 0;
}

// Slots

void KoordDlg::onok()
{
	int m;

	koordx = kdx;
	switch ( kdx )
	{
	case 0:
		mxmin = -8.0;
		mxmax = 8.0;
		break;
	case 1:
		mxmin = -5.0;
		mxmax = 5.0;
		break;
	case 2:
		mxmin = 0.0;
		mxmax = 16.0;
		break;
	case 3:
		mxmin = 0.0;
		mxmax = 10.0;
		break;
	case 4:
		if ( wertholen( mxmin, le_xmin ) != 0 )
			return ;
		if ( wertholen( mxmax, le_xmax ) != 0 )
			return ;
		if ( mxmin >= mxmax )
		{
			KMessageBox::error( this, i18n( "Wrong input:\nxmin > xmax" ), "KmPlot" );
			return ;
		}
	}
	koordy = kdy;
	switch ( kdy )
	{
	case 0:
		mymin = -8.0;
		mymax = 8.0;
		break;
	case 1:
		mymin = -5.0;
		mymax = 5.0;
		break;
	case 2:
		mymin = 0.0;
		mymax = 16.0;
		break;
	case 3:
		mymin = 0.0;
		mymax = 10.0;
		break;
	case 4:
		if ( wertholen( mymin, le_ymin ) != 0 )
			return ;
		if ( wertholen( mymax, le_ymax ) != 0 )
			return ;
		if ( mymin >= mymax )
		{
			KMessageBox::error( this, i18n( "Wrong input:\nymin > ymax" ), "KmPlot" );
			return ;
		}
	}

	xmin = mxmin;
	xmax = mxmax;
	ymin = mymin;
	ymax = mymax;
	xminstr = le_xmin->text();
	xmaxstr = le_xmax->text();
	yminstr = le_ymin->text();
	ymaxstr = le_ymax->text();
	errflg = 0;
	if ( cb_beschr->isChecked() )
	{
		m = 1;
		mode |= LABEL;
	}
	else
	{
		m = 0;
		mode &= ~LABEL;
	}
	
	axesThickness = axesLineWidth->value();
	gradThickness = ticLineWidth->value();
	gradLength = ticLength->value();
	axesColor = color_button->color().rgb();

	if ( cb_default->isChecked() )
	{
		Settings::setXRange( koordx );
		Settings::setYRange( koordy );
		Settings::setXMin( xminstr );
		Settings::setXMax( xmaxstr );
		Settings::setYMin( yminstr );
		Settings::setYMax( ymaxstr );
		Settings::setShowLabel( m == 1 );
		Settings::setAxesLineWidth( axesThickness );
		Settings::setTicWidth( gradThickness );
		Settings::setTicLength( gradLength );
		Settings::setAxesColor( axesColor );
	}
	done( 1 );
}

void KoordDlg::oncancel()
{
	done( 0 );
}

/*
void KoordDlg::onoptions()
{
	KOptDlg odlg;

	odlg.exec();
}
*/

void KoordDlg::xclicked( int ix )
{
	kdx = ix;
}

void KoordDlg::yclicked( int iy )
{
	kdy = iy;
}

void KoordDlg::onXChanged()
{
	kdx = 4;
	setAxes();
}

void KoordDlg::onYChanged()
{
	kdy = 4;
	setAxes();
}

void KoordDlg::onHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}
