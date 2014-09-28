/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C)      2006  David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "equationedit.h"
#include "xparser.h"

#include <kicon.h>

#include <QHBoxLayout>
#include <QPushButton>

#include <assert.h>

#include "equationeditor.h"
#include "equationeditorwidget.h"
#include "equationeditwidget.h"
#include "equationhighlighter.h"

CharMap EquationEdit::m_replaceMap;


EquationEdit::EquationEdit( QWidget * parent )
	: QWidget( parent )
{
	m_cleaningText = false;
	m_settingText = false;
	m_forcingRehighlight = false;
	m_inputType = Expression;
	
	m_equationEditWidget = new EquationEditWidget( this );
	m_highlighter = new EquationHighlighter( this );
	m_equation = new Equation( Equation::Cartesian, 0 );
	m_editButton = new QPushButton( QIcon::fromTheme("document-properties"), 0, this );
	setFocusProxy( m_equationEditWidget );
	
	connect( m_equationEditWidget, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
	connect( m_editButton, SIGNAL(clicked()), this, SLOT(invokeEquationEditor()) );
	connect( m_equationEditWidget, SIGNAL(cursorPositionChanged()), this, SLOT(reHighlight()) );
	
	QHBoxLayout * layout = new QHBoxLayout( this );
	layout->setMargin( 0 );
	layout->addWidget( m_equationEditWidget );
	layout->addWidget( m_editButton );
}


void EquationEdit::setTabChain( QWidget *next )
{
	QWidget::setTabOrder( m_equationEditWidget, m_editButton );
	QWidget::setTabOrder( m_editButton, next );
}


void EquationEdit::setEquationType( Equation::Type type )
{
	delete m_equation;
	m_equation = new Equation( type, 0 );
}


void EquationEdit::showEditButton( bool show )
{
	m_editButton->setVisible( show );
}


void EquationEdit::reHighlight()
{
	if ( m_forcingRehighlight )
		return;
	m_forcingRehighlight = true;
	
	m_highlighter->setDocument( 0 );
	m_highlighter->setDocument( m_equationEditWidget->document() );
	
	m_forcingRehighlight = false;
}


void EquationEdit::invokeEquationEditor()
{
	EquationEditor * edit = new EquationEditor( this );
	edit->edit()->setInputType( m_inputType );
	edit->edit()->setEquationType( m_equation->type() );
	edit->edit()->setValidatePrefix( m_validatePrefix );
	edit->edit()->setText( text() );
	
	edit->exec();
	
	setText( edit->text() );
	edit->deleteLater();
	emit editingFinished();
}


void EquationEdit::setInputType( InputType type )
{
	m_inputType = type;
}


double EquationEdit::value( bool * ok )
{
	assert( m_inputType == Expression ); // Can't really get a value of a function as that requires an input
	
	Parser::Error error;
	double value = XParser::self()->eval( text(), & error );
	
	if (ok)
		*ok = (error == Parser::ParseSuccess);
	
	return value;
}


void EquationEdit::slotTextChanged( )
{
	if ( m_forcingRehighlight )
		return;
	
	//BEGIN tidy up mathematical characters
	if ( m_cleaningText )
		return;
	m_cleaningText = true;
	
	QTextDocument * doc = m_equationEditWidget->document();
	
	if ( m_replaceMap.isEmpty() )
	{
		m_replaceMap[ '*' ] = QChar(0x2219);
		m_replaceMap[ '-' ] = MinusSymbol;
		m_replaceMap[ '|' ] = AbsSymbol;
	}
	
	QTextCursor cursor;
	for ( CharMap::iterator i = m_replaceMap.begin(); i != m_replaceMap.end(); ++i )
	{
		int at = 0;
		while ( !(cursor = doc->find( i.key(), at )).isNull() )
		{
			cursor.joinPreviousEditBlock();
			at = cursor.position()+1;
			cursor.deleteChar();
			cursor.insertText( i.value() );
		 	cursor.endEditBlock();
		}
	}
	
	m_cleaningText = false;
	//END tidy up mathematical characters
	
	
	emit textChanged( text() );
	if ( !m_settingText )
		emit textEdited( text() );
}


void EquationEdit::checkTextValidity( )
{
	QString text = m_validatePrefix + EquationEdit::text();
	
	Parser::Error error;
	int intError, errorPosition;
	
	if ( m_inputType == Function ) {
		m_equation->setFstr( text, &intError, & errorPosition );
		error = (Parser::Error) intError;
	}
	else
		XParser::self()->eval( text, & error, & errorPosition );
	
	if ( error == Parser::ParseSuccess )
		setError( QString(), -1 );
	else
		setError( XParser::self()->errorString( error ), errorPosition - m_validatePrefix.length() );
}


void EquationEdit::setError( const QString & message, int position )
{
	m_equationEditWidget->setToolTip( message );
	m_highlighter->setErrorPosition( position );
}


void EquationEdit::setText( const QString & text )
{
	m_settingText = true;
	m_equationEditWidget->setPlainText( text );
	QTextCursor cursor( m_equationEditWidget->textCursor() );
	cursor.movePosition( QTextCursor::End );
	m_equationEditWidget->setTextCursor( cursor );
	m_settingText = false;
}


void EquationEdit::setValidatePrefix( const QString & prefix )
{
	m_validatePrefix = prefix;
	reHighlight();
}


void EquationEdit::wrapSelected( const QString & before, const QString & after )
{
	QTextCursor cursor( m_equationEditWidget->textCursor() );
	QString newText = before + cursor.selectedText() + after;
	cursor.insertText( newText );
	cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, after.length() );
	m_equationEditWidget->setTextCursor( cursor );
}


QString EquationEdit::text() const
{
	return m_equationEditWidget->toPlainText();
}

void EquationEdit::clear()
{
	m_equationEditWidget->clear();
}

void EquationEdit::selectAll()
{
	m_equationEditWidget->selectAll();
}

void EquationEdit::insertText( const QString & text )
{
	m_equationEditWidget->insertPlainText( text );
}
