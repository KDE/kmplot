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
#include "RstDlg.h"
#include "RstDlg.moc"


#define Inherited RstDlgData

RstDlg::RstDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{	//setCaption( i18n("Raster") );
	switch ( g_mode )
	{
	case 0:
		rb_r1->setChecked( TRUE );
		break;
	case 1:
		rb_r2->setChecked( TRUE );
		break;
	case 2:
		rb_r3->setChecked( TRUE );
		break;
	case 3:
		rb_r4->setChecked( TRUE );
	}
	farbe.setRgb( GitterFarbe );
}


RstDlg::~RstDlg()
{
}

// Slots

void RstDlg::onok()
{
	QString str;

	if ( rb_r1->isChecked() )
		g_mode = 0;
	else if ( rb_r2->isChecked() )
		g_mode = 1;
	else if ( rb_r3->isChecked() )
		g_mode = 2;
	else if ( rb_r4->isChecked() )
		g_mode = 3;
	str = le_dicke->text();
	GitterDicke = str.toInt();
	GitterFarbe = farbe.rgb();

	if ( cb_default->isChecked() )
	{
		kc->setGroup( "Grid" );

		kc->writeEntry( "Line Width", GitterDicke );
		kc->writeEntry( "Mode", g_mode );
		kc->writeEntry( "Color", farbe );
		kc->sync();
	}

	done( 1 );
}

void RstDlg::oncancel()
{
	done( 0 );
}

void RstDlg::onfarbe()
{
	KColorDialog cdlg;

	cdlg.setColor( farbe );
	cdlg.getColor( farbe );
}

void RstDlg::show()
{
	QString str;

	le_dicke->setText( str.setNum( GitterDicke ) );
	QDialog::show();
}
