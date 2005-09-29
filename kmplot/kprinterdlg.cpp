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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// Qt includes
#include <qlayout.h>

// KDE includes
#include <kdialog.h>
#include <klocale.h>

// local includes
#include "kprinterdlg.h"

KPrinterDlg::KPrinterDlg( QWidget *parent, const char *name )
		: KPrintDialogPage( parent, name )
{
	setTitle( i18n( "KmPlot Options" ) );
	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setMargin( KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint() );


	printHeaderTable = new QCheckBox( i18n( "Print header table" ), this );
	transparent_background = new QCheckBox( i18n( "Transparent background" ), this );
	layout->addWidget( printHeaderTable );
	layout->addWidget( transparent_background );
	layout->addStretch( 1 );
}

void KPrinterDlg::getOptions( QMap<QString, QString>& opts, bool include_def )
{
 	if ( include_def || !printHeaderTable->isChecked() )
 		opts[ "app-kmplot-printtable" ] = ( printHeaderTable->isChecked() ? "1" : "-1" );
	if ( include_def || !transparent_background->isChecked() )
		opts[ "app-kmplot-printbackground" ] = ( transparent_background->isChecked() ? "1" : "-1" );
}

void KPrinterDlg::setOptions( const QMap<QString, QString>& opts )
{
	printHeaderTable->setChecked( opts[ "app-kmplot-printtable" ] != "-1" );
	transparent_background->setChecked( opts[ "app-kmplot-printbackground" ] != "-1" );
}

bool KPrinterDlg::isValid( const QString& )
{
	return true;
}

#include "kprinterdlg.moc"
