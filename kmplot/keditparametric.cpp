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
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>

#include <kdebug.h>

// local includes
#include "keditparametric.h"
#include "keditparametric.moc"
#include "xparser.h"

KEditParametric::KEditParametric( XParser* parser, QWidget* parent, const char* name ) : 
	QEditParametric( parent, name )
{
	m_parser = parser;
}

/**
 * Fill the dialog's widgets with the properties of the parser function number index.
 */
void KEditParametric::initDialog( int x_index, int y_index )
{
	m_x_index = x_index;
	m_y_index = y_index;
	if( m_x_index == -1 ) clearWidgets();
	else setWidgets();
}

/**
 * Clear alls widgets values
 */
void KEditParametric::clearWidgets()
{
	kLineEditName->clear();
	kLineEditXFunction->clear();
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->dicke0 );	
	kColorButtonColor->setColor( m_parser->fktext[ m_parser->getNextIndex() ].farbe0 );
}

/**
 * Fill the dialog's widgets with values from the parser
 */
void KEditParametric::setWidgets()
{
	QString name, expression;
	splitEquation( m_parser->fktext[ m_x_index ].extstr, name, expression );
	kLineEditName->setText( name );
	kLineEditXFunction->setText( expression );
	splitEquation( m_parser->fktext[ m_y_index ].extstr, name, expression );
	kLineEditYFunction->setText( expression );
	checkBoxHide->setChecked( m_parser->fktext[ m_x_index ].f_mode == 0 );
	checkBoxRange->setChecked( m_parser->fktext[ m_x_index ].dmin == m_parser->fktext[ m_x_index ].dmax == 0 );
	min->setText( m_parser->fktext[ m_x_index ].str_dmin );
	max->setText( m_parser->fktext[ m_x_index ].str_dmax );
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_x_index ].dicke );
	kColorButtonColor->setColor( m_parser->fktext[ m_x_index ].farbe );
}

/**
 * Overwrites the dialog's accept() method to make sure, that the user's input is valid.
 */
void KEditParametric::accept()
{
	// if we are editing an existing function, first delete the old one
	if( m_x_index != -1 ) 
	{
		m_parser->delfkt( m_x_index );
		m_x_index = -1;
	}
	if( m_y_index != -1 ) 
	{
		m_parser->delfkt( m_y_index );
		m_y_index = -1;
	}
	
	// find a name not allready used 
	if( kLineEditName->text().isEmpty() )
		kLineEditName->setText( newName() );
	
	int index = m_parser->addfkt( xFunction() );
	if( index == -1 ) 
	{
		m_parser->errmsg();
		this->raise();
		kLineEditXFunction->setFocus();
		kLineEditXFunction->selectAll();
		return;
	}
	m_parser->fktext[ index ].extstr = xFunction();
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
	m_parser->fktext[ index ].farbe = kColorButtonColor->color().rgb();
	
	index = m_parser->addfkt( yFunction() );
	if( index == -1 ) 
	{
		m_parser->errmsg();
		this->raise();
		kLineEditYFunction->setFocus();
		kLineEditYFunction->selectAll();
		return;
	}
	m_parser->fktext[ index ].extstr = yFunction();
	m_parser->getext( index );
	
	// call inherited method
	QEditParametric::accept();
}


QString KEditParametric::newName()
{
	int i = 0;
	QString name;
	// prepend the correct prefix
	name = "xf%1";
	do
	{
		i++;
	} while( m_parser->getfix( name.arg( i ) ) != -1 );
	
	// cut off prefix again, will be added later
	name = name.right( name.length()-1 );
	return name.arg( i );
}

/**
 * return the well formed function equation
 */
QString KEditParametric::xFunction()
{
	return "x" + kLineEditName->text() + "(t)=" + kLineEditXFunction->text();
}

/**
 * extract function name and expression from a given expression 
 */
void KEditParametric::splitEquation( const QString equation, QString &name, QString &expression )
{
	int start = 0;
	if( equation[ 0 ] == 'r' || equation[ 0 ] == 'x' || equation[ 0 ] == 'y' ) start++;
	int length = equation.find( '(' ) - start;
	name = equation.mid( start, length );
	
	expression = equation.section( '=', 1, 1 );
}

/**
 * return the well formed function equation
 */
QString KEditParametric::yFunction()
{
	return "y" + kLineEditName->text() + "(t)=" + kLineEditYFunction->text();
}

const QString KEditParametric::functionItem()
{
	return xFunction()+";"+yFunction();
}
