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
#include <qcheckbox.h>

// KDE includes
#include <kcolorbutton.h>
#include <knuminput.h>

// local includes
#include "misc.h"
#include "KOptDlg.h"
#include "KOptDlg.moc"
#include "settings.h"

#define Inherited KOptDlgData

KOptDlg::KOptDlg( QWidget* parent, const char* name ) : Inherited( parent, name, true )
{
	axesLineWidth->setValue( AchsenDicke );
	ticLineWidth->setValue( TeilstrichDicke );
	ticLength->setValue( TeilstrichLaenge );
	color_button->setColor( QColor( AchsenFarbe ) );
}

KOptDlg::~KOptDlg()
{
}

// Slots

void KOptDlg::onok()
{
	AchsenDicke = axesLineWidth->value();
	TeilstrichDicke = ticLineWidth->value();
	TeilstrichLaenge = ticLength->value();
	AchsenFarbe = color_button->color().rgb();

	if ( cb_default->isChecked() )
	{
		Settings::setAxesLineWidth( AchsenDicke );
		Settings::setTicWidth( TeilstrichDicke );
		Settings::setTicLength( TeilstrichLaenge );
		Settings::setAxesColor( AchsenFarbe );
	}

	done( 1 );
}

void KOptDlg::oncancel()
{
	done( 0 );
}
