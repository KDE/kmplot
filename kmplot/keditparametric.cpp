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
#include <kapplication.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <kdebug.h>

// local includes
#include "keditparametric.h"
#include "keditparametric.moc"
#include "xparser.h"
#include "View.h"

KEditParametric::KEditParametric( XParser* parser, QWidget* parent, const char* name ) : 
	QEditParametric( parent, name )
{
	m_parser = parser;
}

void KEditParametric::initDialog( int x_index, int y_index )
{
	m_x_index = x_index;
	m_y_index = y_index;
	if( m_x_index == -1 ) clearWidgets();
	else setWidgets();
}

void KEditParametric::clearWidgets()
{
	kLineEditName->clear();
	kLineEditXFunction->clear();
	kLineEditYFunction->clear();
	checkBoxHide->setChecked( false );
	checkBoxRange->setChecked( false );
	min->clear();
	max->clear();
	kColorButtonColor->setColor( m_parser->fktext[ m_parser->getNextIndex() ].color0 );
}

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
	kIntNumInputLineWidth->setValue( m_parser->fktext[ m_x_index ].linewidth );
	kColorButtonColor->setColor( m_parser->fktext[ m_x_index ].color );
}

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
	
	// find a name not already used 
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
	else
		m_parser->fktext[ index ].f_mode = 1;
	
	if( checkBoxRange->isChecked() )
	{
		m_parser->fktext[ index ].str_dmin = min->text();
		m_parser->fktext[ index ].dmin = m_parser->eval( min->text() );
		if ( m_parser->errmsg())
		{
			min->setFocus();
			min->selectAll();
			m_parser->delfkt( index );
			return;
		}
		m_parser->fktext[ index ].str_dmax = max->text();
		m_parser->fktext[ index ].dmax = m_parser->eval( max->text() );
		if ( m_parser->errmsg())
		{
			max->setFocus();
			max->selectAll();
			m_parser->delfkt( index );
			return;
		}
		if ( m_parser->fktext[ index ].dmin >=  m_parser->fktext[ index ].dmax)
		{
			KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
			min->setFocus();
			min->selectAll();
			m_parser->delfkt( index );
			return;
		}
		
		if (  m_parser->fktext[ index ].dmin<View::xmin || m_parser->fktext[ index ].dmax>View::xmax )
		{
			KMessageBox::error(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
			min->setFocus();
			min->selectAll();
			m_parser->delfkt( index );
			return;
		}
	}
	
	m_parser->fktext[ index ].linewidth = kIntNumInputLineWidth->value();
	m_parser->fktext[ index ].color = kColorButtonColor->color().rgb();
	
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

QString KEditParametric::xFunction()
{
	return "x" + kLineEditName->text() + "(t)=" + kLineEditXFunction->text();
}

void KEditParametric::splitEquation( const QString equation, QString &name, QString &expression )
{
	int start = 0;
	if( equation[ 0 ] == 'r' || equation[ 0 ] == 'x' || equation[ 0 ] == 'y' ) start++;
	int length = equation.find( '(' ) - start;
	name = equation.mid( start, length );
	
	expression = equation.section( '=', 1, 1 );
}

QString KEditParametric::yFunction()
{
	return "y" + kLineEditName->text() + "(t)=" + kLineEditYFunction->text();
}

const QString KEditParametric::functionItem()
{
	return xFunction()+";"+yFunction();
}

void KEditParametric::slotHelp()
{
	kapp->invokeHelp( "", "kmplot" );
}