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
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>

#include <kdebug.h>

// local includes
#include "keditpolar.h"
#include "keditpolar.moc"
#include "xparser.h"

KEditPolar::KEditPolar( XParser* parser, QWidget* parent, const char* name ) : 
	QEditPolar( parent, name )
{
	m_parser = parser;
}

void KEditPolar::initDialog( int index )
{
	m_index = index;
	if( m_index == -1 ) clearWidgets();
	else setWidgets();
}

void KEditPolar::clearWidgets()
{
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->dicke0 );	
	kColorButtonColor->setColor( m_parser->fktext[ m_parser->getNextIndex() ].color );
}

void KEditPolar::setWidgets()
{
	QString function = m_parser->fktext[ m_index ].extstr;
	function = function.right( function.length()-1 );
	kLineEditYFunction->setText( function );
	checkBoxHide->setChecked( m_parser->fktext[ m_index ].f_mode == 0 );
	checkBoxRange->setChecked( m_parser->fktext[ m_index ].dmin == m_parser->fktext[ m_index ].dmax == 0 );
	min->setText( m_parser->fktext[ m_index ].str_dmin );
	max->setText( m_parser->fktext[ m_index ].str_dmax );
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_index ].dicke );
	kColorButtonColor->setColor( m_parser->fktext[ m_index ].color );
}

void KEditPolar::accept()
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
	
	if( checkBoxRange->isChecked() )
	{
		// TODO: check empty boundaries and syntax
		m_parser->fktext[ index ].str_dmin = min->text();
		m_parser->fktext[ index ].dmin = m_parser->eval( min->text() );
		m_parser->fktext[ index ].str_dmax = max->text();
		m_parser->fktext[ index ].dmax = m_parser->eval( max->text() );
	}
	
	m_parser->fktext[ index ].dicke = kIntNumInputLineWidth->value();
	m_parser->fktext[ index ].color = kColorButtonColor->color().rgb();
	
	// call inherited method
	QEditPolar::accept();
}

const QString KEditPolar::functionItem()
{
	return "r" + kLineEditYFunction->text();
}

void KEditPolar::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}
