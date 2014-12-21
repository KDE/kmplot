/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2007  David Saxton <david@bluehaze.org>
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

#include "calculator.h"
#include "equationedit.h"
#include "equationeditorwidget.h"
#include "xparser.h"

#include <KTextEdit>

#include <QScrollBar>
#include <QVBoxLayout>

//BEGIN class Calculator
Calculator::Calculator( QWidget * parent )
	: KDialog( parent )
{
	setModal( false );
	QWidget * widget = new QWidget( this );
	setMainWidget( widget );
	setCaption( i18n("Calculator") );
	setButtons( Close );
	
	QVBoxLayout *layout = new QVBoxLayout( widget );
	layout->setMargin( 0 );
	
	m_display = new KTextEdit( widget );
	QSizePolicy displaySizePolicy = m_display->sizePolicy();
	displaySizePolicy.setVerticalStretch( 10 );
	displaySizePolicy.setVerticalPolicy( QSizePolicy::MinimumExpanding );
	m_display->setSizePolicy( displaySizePolicy );
	layout->addWidget( m_display );
	
    m_input = new EquationEditorWidget( this );
	layout->addWidget( m_input );
	
	m_display->setReadOnly( true );
	
	connect(m_input->edit, &EquationEdit::returnPressed, this, &Calculator::calculate);
	
	resize( layout->minimumSize() );
	m_input->edit->setFocus();
}


Calculator::~Calculator()
{
	m_input->deleteLater();
}


void Calculator::calculate()
{
	Parser::Error error;
	double value = XParser::self()->eval( m_input->edit->text(), &error );
	
	m_displayText += m_input->edit->text().replace( '<', "&lt;" );
	
	if ( error == Parser::ParseSuccess )
		m_displayText += " = <b>" + Parser::number( value ) + "</b>";
	else
		m_displayText += " = ? <font color=\"blue\">(" + Parser::errorString( error ) + ")</font>";
	
	m_displayText += "<br>";
	
	m_display->document()->setHtml( m_displayText );
	m_display->verticalScrollBar()->setValue( m_display->verticalScrollBar()->maximum() );
	m_input->edit->selectAll();
}
//END class Calculator
