/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ller
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
#include <qcheckbox.h>
#include <qlineedit.h>

// KDE includes
#include <kcolorbutton.h>

// local includes
#include "misc.h"
#include "KOptDlg.h"
#include "KOptDlg.moc"

#define Inherited KOptDlgData

KOptDlg::KOptDlg( QWidget* parent, const char* name ) : Inherited( parent, name, true )
{
	ad = AchsenDicke;
	td = TeilstrichDicke;
	tl = TeilstrichLaenge;
	le_ad->setText( QString::number( ad ) );
	le_td->setText( QString::number( td ) );
	le_tl->setText( QString::number( tl ) );
	color_button->setColor( QColor( AchsenFarbe ) );
}

KOptDlg::~KOptDlg()
{
}

// Slots

void KOptDlg::onok()
{
	AchsenDicke = le_ad->text().toInt();
	TeilstrichDicke = le_td->text().toInt();
	TeilstrichLaenge = le_tl->text().toInt();
	AchsenFarbe = color_button->color().rgb();

	if ( cb_default->isChecked() )
	{
		kc->setGroup( "Axes" );

		kc->writeEntry( "Axes Width", AchsenDicke );
		kc->writeEntry( "Tic Width", TeilstrichDicke );
		kc->writeEntry( "Tic Length", TeilstrichLaenge );
		kc->writeEntry( "Color", QColor( AchsenFarbe ) );
		kc->sync();
	}

	done( 1 );
}

void KOptDlg::oncancel()
{
	done( 0 );
}
