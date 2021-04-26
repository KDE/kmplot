/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* SPDX-FileCopyrightText: 2007 David Saxton <david@bluehaze.org>
*
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* SPDX-License-Identifier: GPL-2.0-or-later
*
*/

#include "calculator.h"
#include "equationedit.h"
#include "equationeditorwidget.h"
#include "xparser.h"

#include <KTextEdit>
#include <KConfigGroup>

#include <QDialogButtonBox>
#include <QScrollBar>
#include <QVBoxLayout>

//BEGIN class Calculator
Calculator::Calculator( QWidget * parent )
	: QDialog( parent )
{
	setModal( false );
	QWidget * widget = new QWidget( this );
	setWindowTitle( i18nc("@title:window", "Calculator") );

	QVBoxLayout *layout = new QVBoxLayout( this );

	m_display = new KTextEdit( widget );
	QSizePolicy displaySizePolicy = m_display->sizePolicy();
	displaySizePolicy.setVerticalStretch( 10 );
	displaySizePolicy.setVerticalPolicy( QSizePolicy::MinimumExpanding );
	m_display->setSizePolicy( displaySizePolicy );
	layout->addWidget( m_display );

	m_input = new EquationEditorWidget( this );
	layout->addWidget( m_input );

	m_display->setReadOnly( true );

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &Calculator::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &Calculator::reject);
	layout->addWidget( buttonBox );

	connect(m_input->edit, &EquationEdit::returnPressed, this, &Calculator::calculate);

	// Set minimum size and margin to avoid cutting the right side
	resize( 1.05*layout->minimumSize() );
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
