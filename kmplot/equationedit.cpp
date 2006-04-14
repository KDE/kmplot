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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#include "equationedit.h"
#include "xparser.h"

#include <kdebug.h>
#include <QApplication>
#include <QStyleOptionFrame>


//BEGIN class EquationHighlighter
EquationHighlighter::EquationHighlighter( EquationEdit * parent )
	: QSyntaxHighlighter( parent ),
	m_parent( parent )
{
	m_errorPosition = -1;
}


EquationHighlighter::~ EquationHighlighter( )
{
}


void EquationHighlighter::highlightBlock( const QString & text )
{
	m_parent->checkTextValidity();
	
// 	QTextCharFormat number;
// 	number.setForeground( Qt::darkMagenta );
	
	QTextCharFormat bracket;
// 	bracket.setFontWeight( QFont::Bold );
	
	QTextCharFormat normal;
	
	for ( int i = 0; i < text.length(); ++i )
	{
// 		if ( text[i].isDigit() || text[i] == '.' )
// 			setFormat( i, 1, number );
// 		else
			setFormat( i, 1, normal );
	}
	
	if ( m_errorPosition != -1 )
	{
		QTextCharFormat error;
		error.setForeground( Qt::red );
	
		setFormat( m_errorPosition, 1, error );
	}
}


void EquationHighlighter::setErrorPosition( int position )
{
	m_errorPosition = position;
}
//END class EquationHighlighter



//BEGIN class EquationEdit
EquationEdit::EquationEdit( QWidget * parent )
	: QTextEdit( parent )
{
	//BEGIN setting up appearance
	// Set fixed height
	ensurePolished();
	QFontMetrics fm( font() );
	int h = qMax(fm.lineSpacing(), 14) + 4;
	int m = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QStyleOptionFrame opt;
	opt.rect = rect();
	opt.palette = palette();
	opt.state = QStyle::State_None;
	setFixedHeight( h + (2 * m) );
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setContentsMargins( 0, 0, 0, 0 );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setWordWrapMode( QTextOption::NoWrap );
	setLineWrapMode( QTextEdit::NoWrap );
	setTabChangesFocus( true );
	//END setting up appearance
	
	m_settingText = false;
	m_inputType = Expression;
	m_highlighter = new EquationHighlighter( this );
	m_equation = new Equation( Equation::Cartesian, 0 );
	
	connect( this, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
}


void EquationEdit::setInputType( InputType type )
{
	m_inputType = type;
}


void EquationEdit::slotTextChanged( )
{
	emit textChanged( text() );
	if ( !m_settingText )
		emit textEdited( text() );
}


void EquationEdit::checkTextValidity( )
{
	QString text = m_validatePrefix + toPlainText();
	
	bool ok = true;
	
	if ( m_inputType == Function )
		ok = m_equation->setFstr( text );
	else
	{
		XParser::self()->eval( text );
		ok = XParser::self()->errorString().isEmpty();
	}
		
	if ( ok )
		setError( QString(), -1 );
	else
		setError( XParser::self()->errorString(), XParser::self()->errorPosition() - m_validatePrefix.length() );
}


void EquationEdit::wheelEvent( QWheelEvent * e )
{
	e->accept();
}


void EquationEdit::keyPressEvent( QKeyEvent * e )
{
	if ( (e->key() == Qt::Key_Return) ||
			 (e->key() == Qt::Key_Enter) )
	{
		e->accept();
		emit editingFinished();
	}
	else
		QTextEdit::keyPressEvent( e );
}


void EquationEdit::focusOutEvent( QFocusEvent * e )
{
	QTextEdit::focusOutEvent( e );
	emit editingFinished();
}


void EquationEdit::setError( const QString & message, int position )
{
	setToolTip( message );
	m_highlighter->setErrorPosition( position );
}


void EquationEdit::setText( const QString & text )
{
	m_settingText = true;
	setPlainText( text );
	m_settingText = false;
}


void EquationEdit::setValidatePrefix( const QString & prefix )
{
	m_validatePrefix = prefix;
	checkTextValidity();
}
//END class EquationEdit


#include "equationedit.moc"

