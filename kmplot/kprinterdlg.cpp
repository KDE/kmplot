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


	m_printtable = new QCheckBox( i18n( "Print header table" ), this );
	m_printtable->setChecked( true );
	layout->addWidget( m_printtable );
	layout->addStretch( 1 );
}

void KPrinterDlg::getOptions( QMap<QString, QString>& opts, bool include_def )
{
	if ( include_def || !m_printtable->isChecked() )
		opts[ "app-kmplot-printtable" ] = ( m_printtable->isChecked() ? "1" : "-1" );
}

void KPrinterDlg::setOptions( const QMap<QString, QString>& opts )
{
	m_printtable->setChecked( opts[ "app-kmplot-printtable" ] != "-1" );
}

bool KPrinterDlg::isValid( QString& msg )
{
	return true;
}

#include "kprinterdlg.moc"
