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
void KEditFunction::initDialog( const FunctionType t, int index, int y_index )
{
	m_type = t;
	m_index = index;
	m_y_index = y_index;
	setVisibleWidgets();
	if( m_index == -1 ) clearWidgets();
	else setWidgets();
}

/**
 * Only show Widgets related to the function type.
 * Some WhatsThis texts have to be adjusted, too.
 */
void KEditFunction::setVisibleWidgets()
{
	switch( m_type )
	{
		case Function: 
			this->setCaption( i18n( "Edit Function Plot" ) );
			textLabelX->hide();
			textLabelXF->hide();
			textLabelArgX->hide();
			kLineEditXFunction->hide();
			textLabelY->setText( "" );
			textLabelArgY->setText( "(x) = " );
			QWhatsThis::add( kLineEditYFunction, tr2i18n( "Enter an expression for the function.\n"
				"The dummy variable is x.\n"
				"Example: x^2" ) );
			checkBoxDerivative1->show();
			checkBoxDerivative2->show();
			break;
		case Parametric: 
			this->setCaption( i18n( "Edit Parametric Plot" ) );
			kLineEditXFunction->show();
			textLabelX->show();
			textLabelXF->show();
			textLabelArgX->show();
			textLabelY->setText( "y" );
			textLabelArgY->setText( "(t) = " );
			QWhatsThis::add( kLineEditYFunction, tr2i18n( "Enter an expression for the function.\n"
				"The dummy variable is t.\n"
				"Example: sin(t)" ) );
			checkBoxDerivative1->hide();
			checkBoxDerivative2->hide();
			break;
		case Polar: 
			this->setCaption( i18n( "Edit Polar Plot" ) );
			kLineEditXFunction->hide();
			textLabelX->hide();
			textLabelXF->hide();
			textLabelArgX->hide();
			textLabelY->setText( "r" );
			textLabelArgY->setText( "(theta) = " );
			QWhatsThis::add( kLineEditYFunction, tr2i18n( "Enter an expression for the function.\n"
				"The dummy variable is theta.\n"
				"Example: 3*theta" ) );
			checkBoxDerivative1->show();
			checkBoxDerivative2->show();
	}
	updateGeometry();
}

/**
 * Clear alls widgets values
 */
void KEditFunction::clearWidgets()
{
	kLineEditName->clear();
	kLineEditXFunction->clear();
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxDerivative1->setChecked( false );
	checkBoxDerivative2->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kIntNumInputLineWidth->setValue( m_parser->dicke0 );	
	kColorButtonColor->setColor( "#000000" );
}

/**
 * Fill the dialog's widgets with values from the parser
 */
void KEditFunction::setWidgets()
{
	QString name, expression;
	splitEquation( m_parser->fktext[ m_index ].extstr, name, expression );
	switch( m_type )
	{
		case Function:
			checkBoxDerivative1->setChecked( m_parser->fktext[ m_index ].f1_mode == 1 );
			checkBoxDerivative2->setChecked( m_parser->fktext[ m_index ].f2_mode == 1 );
		case Polar:
			kLineEditName->setText( name );
			kLineEditYFunction->setText( expression );
			break;
		case Parametric:
			kLineEditName->setText( name );
			kLineEditXFunction->setText( expression );
			splitEquation( m_parser->fktext[ m_y_index ].extstr, name, expression );
			kLineEditYFunction->setText( expression );
			break;
	}
	checkBoxHide->setChecked( m_parser->fktext[ m_index ].f_mode == 0 );
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
	if( m_y_index != -1 ) 
	{
		m_parser->delfkt( m_y_index );
		m_y_index = -1;
	}
	
	// find a name not allready used 
	if( kLineEditName->text().isEmpty() )
		kLineEditName->setText( newName() );
	
	int index = m_parser->addfkt( yFunction() );
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
	
	if( kLineEditXFunction->isVisible() )
	{ 
		index = m_parser->addfkt( xFunction() );
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
	}
	
	if( checkBoxHide->isChecked() )
		m_parser->fktext[ index ].f_mode = 0;
	if( checkBoxDerivative1->isChecked() )
		m_parser->fktext[ index ].f1_mode = 1;
	if( checkBoxDerivative2->isChecked() )
		m_parser->fktext[ index ].f2_mode = 1;
	
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


QString KEditFunction::newName()
{
	int i = 0;
	QString name;
	// prepend the correct prefix
	switch( m_type )
	{
		case Function:
			name = "f%1";
			break;
		case Parametric:
			name = "xf%1";
			break;
		case Polar:
			name = "rf%1";
	}
	do
	{
		i++;
	} while( m_parser->getfix( name.arg( i ) ) != -1 );
	
	// cut off prefix again, will be added later
	if( m_type == Parametric || m_type == Polar )
		name = name.right( name.length()-1 );
	return name.arg( i );
}

/**
 * return the well formed function equation
 */
QString KEditFunction::xFunction()
{
	return "x" + kLineEditName->text() + "(t)=" + kLineEditXFunction->text();
}

/**
 * extract function name and expression from a given expression 
 */
void KEditFunction::splitEquation( const QString equation, QString &name, QString &expression )
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
QString KEditFunction::yFunction()
{
	switch( m_type )
	{
		case Function:
			return kLineEditName->text() + "(x)=" + kLineEditYFunction->text();
		case Parametric:
			return "y" + kLineEditName->text() + "(t)=" + kLineEditYFunction->text();
		case Polar:
			return "r" + kLineEditName->text() + "(theta)=" + kLineEditYFunction->text();
	}
}
