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
#include <qstringlist.h>

// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>

#include <kdebug.h>

// local includes
#include "keditfunction.h"
#include "keditfunction.moc"
#include "xparser.h"

KEditFunction::KEditFunction( XParser* parser, QWidget* parent, const char* name ) : 
	QEditFunction( parent, name )
{
	m_parser = parser;
}

/**
 * Fill the dialog's widgets with the properties of the parser function number index.
 */
void KEditFunction::initDialog( int index )
{
	m_index = index;
	if( m_index == -1 ) clearWidgets();
	else setWidgets();
}

/**
 * Clear alls widgets values
 */
void KEditFunction::clearWidgets()
{
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxDerivative1->setChecked( false );
	checkBoxDerivative2->setChecked( false );
	hasParameters->setChecked( false );
	parameters->clear();
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->dicke0 );	
	kColorButtonColor->setColor( m_parser->fktext[ m_parser->getNextIndex() ].farbe );
}

/**
 * Fill the dialog's widgets with values from the parser
 */
void KEditFunction::setWidgets()
{
	kLineEditYFunction->setText( m_parser->fktext[ m_index ].extstr );
	checkBoxDerivative1->setChecked( m_parser->fktext[ m_index ].f1_mode == 1 );
	checkBoxDerivative2->setChecked( m_parser->fktext[ m_index ].f2_mode == 1 );
	checkBoxHide->setChecked( m_parser->fktext[ m_index ].f_mode == 0 );
	hasParameters->setChecked( m_parser->fktext[ m_index ].k_anz != 0 );
	QStringList listOfParameters;
	for( int k_index = 0; k_index < m_parser->fktext[ m_index ].k_anz; k_index++ )
	{
		listOfParameters += QString::number( m_parser->fktext[ m_index ].k_liste[ k_index ] );
	}
	parameters->setText( listOfParameters.join( "," ) );
	checkBoxRange->setChecked( m_parser->fktext[ m_index ].dmin == m_parser->fktext[ m_index ].dmax == 0 );
	min->setText( m_parser->fktext[ m_index ].str_dmin );
	max->setText( m_parser->fktext[ m_index ].str_dmax );
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_index ].dicke );
	kColorButtonColor->setColor( m_parser->fktext[ m_index ].farbe );
}

/**
 * Overwrites the dialog's accept() method to make sure, that the user's input is valid.
 */
void KEditFunction::accept()
{
	// if we are editing an existing function, first delete the old one
	if( m_index != -1 ) 
	{
		m_parser->delfkt( m_index );
		m_index = -1;
	}
	
	int index = m_parser->addfkt( functionItem() );
	if( index == -1 ) 
	{
		m_parser->errmsg();
		this->raise();
		kLineEditYFunction->setFocus();
		kLineEditYFunction->selectAll();
		return;
	}
	m_parser->fktext[ index ].extstr = functionItem();
	m_parser->getext( index );
	
	if( checkBoxHide->isChecked() )
		m_parser->fktext[ index ].f_mode = 0;
	if( checkBoxDerivative1->isChecked() )
		m_parser->fktext[ index ].f1_mode = 1;
	if( checkBoxDerivative2->isChecked() )
		m_parser->fktext[ index ].f2_mode = 1;
		
	if( hasParameters->isChecked() )
	{
		QStringList listOfParameters = QStringList::split( ",", parameters->text() );
		m_parser->fktext[ index ].k_anz = 0;
		for( QStringList::Iterator it = listOfParameters.begin(); it != listOfParameters.end(); ++it )
		{
			m_parser->fktext[ index ].k_liste[ m_parser->fktext[ index ].k_anz ] = 
				( *it ).toDouble();
			m_parser->fktext[ index ].k_anz++;
		}
	}
	
	if( checkBoxRange->isChecked() )
	{
		// TODO: check empty boundaries and syntax
		m_parser->fktext[ index ].str_dmin = min->text();
		m_parser->fktext[ index ].dmin = m_parser->eval( min->text() );
		m_parser->fktext[ index ].str_dmax = max->text();
		m_parser->fktext[ index ].dmax = m_parser->eval( max->text() );
	}
	
	m_parser->fktext[ index ].dicke = kIntNumInputLineWidth->value();
	m_parser->fktext[ index ].farbe = kColorButtonColor->color().rgb();
	
	// call inherited method
	QEditFunction::accept();
}

/**
 * return the well formed function equation for the listbox in FktDlg
 */
const QString KEditFunction::functionItem()
{
	return kLineEditYFunction->text();
}

void KEditFunction::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}
