/***************************************************************************

    kmplot/kprinterdlg.cpp  -  PrintDialogPage for kmplot priter pptions
                             -------------------
    begin                : 2002-06-21
    email                : bmlmessmer@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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


	m_printtable = new QCheckBox( i18n( "print header table" ), this );
    m_printtable->setChecked( true );
    layout->addWidget( m_printtable );
	layout->addStretch( 1 );
}

void KPrinterDlg::getOptions( QMap<QString,QString>& opts, bool include_def )
{
    if ( include_def || !m_printtable->isChecked() )
        opts[ "app-kmplot-printtable" ] = ( m_printtable->isChecked() ? "1" : "-1" );
}

void KPrinterDlg::setOptions( const QMap<QString,QString>& opts )
{
    m_printtable->setChecked( opts[ "app-kmplot-printtable" ] != "-1" );
}

bool KPrinterDlg::isValid( QString& msg )
{
    return true;
}

#include "kprinterdlg.moc"
