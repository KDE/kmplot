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


	kcfg_printHeaderTable = new QCheckBox( i18n( "Print header table" ), this );
	layout->addWidget( kcfg_printHeaderTable );
	layout->addStretch( 1 );
}

void KPrinterDlg::getOptions( QMap<QString, QString>& opts, bool include_def )
{
 	if ( include_def || !kcfg_printHeaderTable->isChecked() )
 		opts[ "app-kmplot-printtable" ] = ( kcfg_printHeaderTable->isChecked() ? "1" : "-1" );
//	Settings::setPrintHeaderTable( kcfg_printHeaderTable->isChecked() );
}

void KPrinterDlg::setOptions( const QMap<QString, QString>& opts )
{
	kcfg_printHeaderTable->setChecked( opts[ "app-kmplot-printtable" ] != "-1" );
//	kcfg_printHeaderTable->setChecked( Settings::printHeaderTable() );
}

bool KPrinterDlg::isValid( QString& msg )
{
	return true;
}

#include "kprinterdlg.moc"
