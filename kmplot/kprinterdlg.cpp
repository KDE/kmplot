/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes
#include <kdialog.h>
#include <klocale.h>

// local includes
#include "equationedit.h"
#include "kprinterdlg.h"
#include "parser.h"

KPrinterDlg::KPrinterDlg( QWidget *parent )
		: KPrintDialogPage( parent )
{
	setTitle( i18n( "KmPlot Options" ) );
	
	QGridLayout *layout = new QGridLayout( this );
	layout->setMargin( KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint() );

	printHeaderTable = new QCheckBox( i18n( "Print header table" ), this );
	transparent_background = new QCheckBox( i18n( "Transparent background" ), this );
	
	m_widthEdit = new EquationEdit( this );
	m_heightEdit = new EquationEdit( this );
	
	m_widthEdit->setText( "12" );
	m_heightEdit->setText( "12" );
	
	m_lengthScalingCombo = new QComboBox( this );
	m_lengthScalingCombo->addItem( i18n("Pixels (1/72nd in)") );
	m_lengthScalingCombo->addItem( i18n("Inches (in)") );
	m_lengthScalingCombo->addItem( i18n("Centimeters (cm)") );
	m_lengthScalingCombo->addItem( i18n("Millimeters (mm)") );
	
	m_lengthScalingCombo->setCurrentIndex( 2 ); // default of centermeters
	
	QLabel *widthLabel = new QLabel( i18n("Width:"), this );
	QLabel *heightLabel = new QLabel( i18n("Height:"), this );
	
	layout->addWidget( printHeaderTable, 0, 0, 1, 2 );
	layout->addWidget( transparent_background, 1, 0, 1, 2 );
	layout->addWidget( widthLabel, 2, 0, 1, 1 );
	layout->addWidget( m_widthEdit, 2, 1, 1, 1 );
	layout->addWidget( heightLabel, 3, 0, 1, 1 );
	layout->addWidget( m_heightEdit, 3, 1, 1, 1 );
	layout->addWidget( m_lengthScalingCombo, 4, 1, 1, 1 );
	
	layout->setRowStretch( 5, 1 );
}

void KPrinterDlg::getOptions( QMap<QString, QString>& opts, bool includeDefaults )
{
	if ( includeDefaults || !printHeaderTable->isChecked() )
 		opts[ "app-kmplot-printtable" ] = ( printHeaderTable->isChecked() ? "1" : "-1" );
	if ( includeDefaults || !transparent_background->isChecked() )
		opts[ "app-kmplot-printbackground" ] = ( transparent_background->isChecked() ? "1" : "-1" );
	
	opts[ "app-kmplot-width" ] = QString::number( m_widthEdit->value() * lengthScaling() );
	opts[ "app-kmplot-height" ] = QString::number( m_heightEdit->value() * lengthScaling() );
	
}

void KPrinterDlg::setOptions( const QMap<QString, QString>& opts )
{
	printHeaderTable->setChecked( opts[ "app-kmplot-printtable" ] != "-1" );
	transparent_background->setChecked( opts[ "app-kmplot-printbackground" ] != "-1" );
	
	double width = opts[ "app-kmplot-width" ].toDouble() / lengthScaling();
	double height = opts[ "app-kmplot-height" ].toDouble() / lengthScaling();
	
	m_widthEdit->setText( Parser::number( width ) );
	m_heightEdit->setText( Parser::number( height ) );
}

bool KPrinterDlg::isValid( QString &msg )
{
	bool ok;
	
	m_widthEdit->value( & ok );
	if ( ! ok )
	{
		msg = i18n("Width is invalid");
		return false;
	}
	
	m_heightEdit->value( & ok );
	if ( ! ok )
	{
		msg = i18n("Height is invalid");
		return false;
	}
	
	return true;
}


double KPrinterDlg::scalingToMeter( LengthScaling scaling )
{
	switch ( scaling )
	{
	case Centimeters:
		return 0.01;
	case Millimeters:
		return 0.001;
	case Inches:
		return 0.0254;
	case Pixels:
		return 0.0254 / 72.0;
	}
	
	return 1;
}


double KPrinterDlg::lengthScaling( ) const
{
	LengthScaling scaling = (LengthScaling)m_lengthScalingCombo->currentIndex();
	return scalingToMeter( scaling );
}

#include "kprinterdlg.moc"
