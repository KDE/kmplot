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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

// local includes
#include "AttrDlg.h"
#include "AttrDlg.moc"

#define Inherited AttrDlgData

AttrDlg::AttrDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
}

AttrDlg::~AttrDlg()
{
}

// Slots

void AttrDlg::onok()
{
	QString str;

	str = le_dicke->text();
	ps.fktext[ ix ].dicke = str.toInt();
	ps.fktext[ ix ].farbe = farbe.rgb();

	if ( cb_default->isChecked() )
	{
		char s[ 8 ];

		kc->setGroup( "Graphs" );
		kc->writeEntry( "Line Width", ps.fktext[ ix ].dicke );
		sprintf( s, "Color%c", '0' + ix );
		kc->writeEntry( s, farbe );
		kc->sync();
	}

	done( 1 );
}

void AttrDlg::oncancel()
{
	done( 0 );
}

void AttrDlg::onfarbe()
{
	KColorDialog cdlg;

	cdlg.setColor( farbe );
	cdlg.getColor( farbe );
}

void AttrDlg::show()
{
	QString str;

	le_dicke->setText( str.setNum( ps.fktext[ ix ].dicke ) );
	farbe.setRgb( ps.fktext[ ix ].farbe );
	QDialog::show();
}
