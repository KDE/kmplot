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
#include "KOptDlg.h"
#include "KOptDlg.moc"


#define Inherited KOptDlgData

KOptDlg::KOptDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
	QString str;

	ad = AchsenDicke;
	td = TeilstrichDicke;
	tl = TeilstrichLaenge;
	le_ad->setText( str.setNum( ad ) );
	le_td->setText( str.setNum( td ) );
	le_tl->setText( str.setNum( tl ) );
	farbe.setRgb( AchsenFarbe );
}

KOptDlg::~KOptDlg()
{
}

// Slots

void KOptDlg::onok()
{
	QString str;

	str = le_ad->text();
	ad = str.toInt();
	str = le_td->text();
	td = str.toInt();
	str = le_tl->text();
	tl = str.toInt();
	AchsenDicke = ad;
	TeilstrichDicke = td;
	TeilstrichLaenge = tl;
	AchsenFarbe = farbe.rgb();

	if ( cb_default->isChecked() )
	{
		kc->setGroup( "Axes" );

		kc->writeEntry( "Axes Width", ad );
		kc->writeEntry( "Tic Width", td );
		kc->writeEntry( "Tic Length", tl );
		kc->writeEntry( "Color", farbe );
		kc->sync();
	}

	done( 1 );
}

void KOptDlg::oncancel()
{
	done( 0 );
}

void KOptDlg::onfarbe()
{
	KColorDialog cdlg;

	cdlg.setColor( farbe );
	cdlg.getColor( farbe );
}
