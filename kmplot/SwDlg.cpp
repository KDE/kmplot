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

#include "SwDlg.h"
#include "SwDlg.moc"

#define Inherited SwDlgData


SwDlg::SwDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
	QString str;

	//setCaption(i18n("Configure Increment"));
	le_sw->setText( str.setNum( rsw ) );
	le_sw->setFocus();
}

SwDlg::~SwDlg()
{
}

// Slots

void SwDlg::onok()
{
	double w;
	QString str;

	str = le_sw->text();
	w = str.toDouble();
	printf( "%g\n", w );
	if ( w > 0. && w <= 20. )
	{
		rsw = w;
		done( 1 );
	}
	else
		KMessageBox::error( this, i18n( "Wrong input:\n"
		                                "Allowed range is [0-20]." ), "KmPlot" );
}

void SwDlg::oncancel()
{
	done( 0 );
}
