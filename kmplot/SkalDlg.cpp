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

#include "settings.h"
#include "SkalDlg.h"
#include "SkalDlg.moc"

#define Inherited SkalDlgData

SkalDlg::SkalDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
	cb_xtlg->insertItem( tlgxstr, 0 );
	cb_ytlg->insertItem( tlgystr, 0 );
	cb_xdruck->insertItem( drskalxstr, 0 );
	cb_ydruck->insertItem( drskalystr, 0 );
}

SkalDlg::~SkalDlg()
{
}

int SkalDlg::wertholen( double& w, QComboBox *cb )
{
	w = ps.eval( cb->currentText() );
	if ( w <= 0. )
	{
		if ( ps.err != 0 )
			ps.errmsg();
		else
			KMessageBox::error( this, i18n( "Wrong input:\n All terms must result in values greater than 0." ), "KmPlot" );
		errflg = 1;
		return -1;
	}
	return 0;
}

// Slots

void SkalDlg::onok()
{
	double dx, dy, tx, ty, mxmax, mymax;

	if ( wertholen( tx, cb_xtlg ) != 0 )
		return ;
	if ( wertholen( ty, cb_ytlg ) != 0 )
		return ;
	if ( wertholen( dx, cb_xdruck ) != 0 )
		return ;
	if ( wertholen( dy, cb_ydruck ) != 0 )
		return ;

	mxmax = ( xmax - xmin ) * dx / tx;
	mymax = ( ymax - ymin ) * dy / ty;
	if ( mxmax > 16. || mymax > 16. )
	{
		KMessageBox::error( this, i18n( "The plot is too big for paper size Din A4.\n"
		                                "Please correct the printing format." ), "KmPlot" );
		return ;
	}

	tlgx = tx;
	tlgxstr = cb_xtlg->currentText();
	tlgy = ty;
	tlgystr = cb_ytlg->currentText();
	drskalx = dx;
	drskalxstr = cb_xdruck->currentText();
	drskaly = dy;
	drskalystr = cb_ydruck->currentText();

	if ( cb_default->isChecked() )
	{
		Settings::setXScaling( cb_xtlg->currentItem() );
		Settings::setYScaling( cb_ytlg->currentItem() );
		Settings::setXPrinting( cb_xdruck->currentItem() );
		Settings::setYPrinting( cb_ydruck->currentItem() );
	}

	errflg = 0;
	done( 1 );
}

void SkalDlg::oncancel()
{
	done( 0 );
}

void SkalDlg::onxtlgactivated( int ix )
{
	cb_xdruck->setCurrentItem( ix );
}

void SkalDlg::onytlgactivated( int ix )
{
	cb_ydruck->setCurrentItem( ix );
}
