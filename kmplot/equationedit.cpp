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
#include "maindlg.h"
#include "ui_equationeditor.h"
#include "xparser.h"

#include <kdebug.h>
#include <kicon.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleOptionFrame>

#include <assert.h>


class EquationEditorWidget : public QWidget, public Ui::EquationEditor
{
	public:
		EquationEditorWidget( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};


//BEGIN class EquationHighlighter
EquationHighlighter::EquationHighlighter( EquationEdit * parent )
	: QSyntaxHighlighter( parent->m_equationEditWidget ),
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
	: QWidget( parent )
{
	m_settingText = false;
	m_inputType = Expression;
	
	m_equationEditWidget = new EquationEditWidget( this );
	m_highlighter = new EquationHighlighter( this );
	m_equation = new Equation( Equation::Cartesian, 0 );
	
	connect( m_equationEditWidget, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
	
	m_editButton = new QPushButton( KIcon("edit"), 0, this );
// 	m_editButton = new QPushButton( "...", this );
	connect( m_editButton, SIGNAL(clicked()), this, SLOT(invokeEquationEditor()) );
	
	QHBoxLayout * layout = new QHBoxLayout( this );
	layout->setMargin( 0 );
	layout->addWidget( m_equationEditWidget );
	layout->addWidget( m_editButton );
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


void EquationEdit::invokeEquationEditor()
{
	EquationEditor * edit = new EquationEditor( text(), this );
	edit->m_widget->edit->setInputType( m_inputType );
	edit->m_widget->edit->setValidatePrefix( m_validatePrefix );
	
	edit->exec();
	
	setText( edit->text() );
	edit->deleteLater();
	emit editingFinished();
}


void EquationEdit::setInputType( InputType type )
{
	m_inputType = type;
}


double EquationEdit::value( )
{
	assert( m_inputType == Expression ); // Can't really get a value of a function as that requires an input
	
	return XParser::self()->eval( text() );
}


void EquationEdit::slotTextChanged( )
{
	emit textChanged( text() );
	if ( !m_settingText )
		emit textEdited( text() );
}


void EquationEdit::checkTextValidity( )
{
	QString text = m_validatePrefix + EquationEdit::text();
	
	bool ok = true;
	
	if ( m_inputType == Function )
		ok = m_equation->setFstr( text );
	else
	{
		XParser::self()->eval( text );
		ok = XParser::self()->errorString().isEmpty();
	}
	
// 	kDebug() << k_funcinfo << "ok="<<ok<<" XParser::self()->errorPosition()="<<XParser::self()->errorPosition()<<endl;
		
	if ( ok )
		setError( QString(), -1 );
	else
		setError( XParser::self()->errorString(), XParser::self()->errorPosition() - m_validatePrefix.length() );
}


void EquationEdit::setError( const QString & message, int position )
{
	setToolTip( message );
	m_highlighter->setErrorPosition( position );
}


void EquationEdit::setText( const QString & text )
{
	m_settingText = true;
	m_equationEditWidget->setPlainText( text );
	m_equationEditWidget->textCursor().movePosition( QTextCursor::End );
	m_settingText = false;
}


void EquationEdit::setValidatePrefix( const QString & prefix )
{
	m_validatePrefix = prefix;
	checkTextValidity();
}
//END class EquationEdit



//BEGIN class EquationEditWidget
EquationEditWidget::EquationEditWidget( EquationEdit * parent )
	: QTextEdit( parent )
{
	m_parent = parent;
	recalculateGeometry( font() );
}


void EquationEditWidget::recalculateGeometry( const QFont & font )
{
	// Set fixed height
	ensurePolished();
	QFontMetrics fm( font );
	int h = qMax(fm.lineSpacing(), 14) + 6;
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
}


void EquationEditWidget::wheelEvent( QWheelEvent * e )
{
	e->accept();
}


void EquationEditWidget::keyPressEvent( QKeyEvent * e )
{
	if ( (e->key() == Qt::Key_Return) ||
			 (e->key() == Qt::Key_Enter) )
	{
		e->accept();
		emit m_parent->editingFinished();
		emit m_parent->returnPressed();
	}
	else
	{
		QTextEdit::keyPressEvent( e );
	}
}


void EquationEditWidget::focusOutEvent( QFocusEvent * e )
{
	QTextEdit::focusOutEvent( e );
	emit m_parent->editingFinished();
}
//END class EquationEdit



//BEGIN class EquationEditor
EquationEditor::EquationEditor( const QString & equation, QWidget * parent )
	: KDialog( parent )
{	
	m_widget = new EquationEditorWidget( this );
	m_widget->edit->showEditButton( false );
	m_widget->layout()->setMargin( 0 );
	setMainWidget( m_widget );
	
	setCaption( i18n("Equation Editor") );
	setButtons( Close );
	
	QFont font;
	font.setPointSizeF( font.pointSizeF() * 1.4 );
	m_widget->edit->m_equationEditWidget->setCurrentFont( font );
	m_widget->edit->m_equationEditWidget->recalculateGeometry( font );
	m_widget->edit->setText( equation );
	
	QList<QToolButton *> buttons = m_widget->findChildren<QToolButton *>();
	foreach ( QToolButton * w, buttons )
		connect( w, SIGNAL(clicked()), this, SLOT(characterButtonClicked()) );
	
	connect( m_widget->constantsButton, SIGNAL(clicked()), this, SLOT(editConstants()) );
	connect( m_widget->functionList, SIGNAL(activated(const QString &)), this, SLOT(insertFunction(const QString &)) );
	
	m_widget->functionList->addItems( XParser::self()->predefinedFunctions() );
	
	// Constant editing doesn't work atm
	m_widget->constantsButton->setEnabled( false );
	
	connect( m_widget->edit, SIGNAL(returnPressed()), this, SLOT(accept()) );
}


QString EquationEditor::text() const
{
	return m_widget->edit->text();
}


void EquationEditor::editConstants()
{
	MainDlg::self()->showConstantsEditor();
}


void EquationEditor::insertFunction( const QString & function )
{
	m_widget->functionList->setCurrentItem( 0 );
	m_widget->edit->insertText( function + "()" );
}


void EquationEditor::characterButtonClicked()
{
	const QToolButton * tb = static_cast<const QToolButton *>(sender());
	
	// Something (I can't work out what) is 'helpfully' inserting an ampersand (for keyboard acceleration).
	// Get rid of it.
	m_widget->edit->insertText( tb->text().remove( '&' ) );
}
//END class EquationEditor



#include "equationedit.moc"

