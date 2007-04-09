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

#include <kacceleratormanager.h>
#include <kdebug.h>
#include <kicon.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QLocale>
#include <QPushButton>
#include <QStyleOptionFrame>

#include <assert.h>

CharMap EquationEdit::m_replaceMap;


/**
 * The actual line edit.
 */
class EquationEditWidget : public QTextEdit
{
	public:
		EquationEditWidget( EquationEdit * parent );
		
		/**
		 * Call this after changing font size.
		 */
		void recalculateGeometry();
		/**
		 * Whether to clear the selection when focus is lost.
		 */
		void setClearSelectionOnFocusOut( bool doIt ) { m_clearSelectionOnFocusOut = doIt; }
		
	protected:
		void clearSelection();
		
		void wheelEvent( QWheelEvent * e );
		void keyPressEvent( QKeyEvent * e );
		void focusOutEvent( QFocusEvent * e );
		void focusInEvent( QFocusEvent * e );
		
		EquationEdit * m_parent;
		bool m_clearSelectionOnFocusOut;
};


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
	// Good color defaults borrowed from Abakus - thanks! :)
	
	m_parent->checkTextValidity();
	
	if ( text.isEmpty() )
		return;
	
	QTextCharFormat number;
	number.setForeground( QColor( 0, 0, 127 ) );
	
	QTextCharFormat function;
	function.setForeground( QColor( 85, 0, 0 ) );
	
	QTextCharFormat variable;
	variable.setForeground( QColor( 0, 85, 0 ) );
	
	QTextCharFormat matchedParenthesis;
	matchedParenthesis.setBackground( QColor( 255, 255, 183 ) );
	
	QTextCharFormat other;
	
	QStringList variables = m_parent->m_equation->variables();
	QStringList functions = XParser::self()->predefinedFunctions(true) + XParser::self()->userFunctions();
	
	for ( int i = 0; i < text.length(); ++i )
	{
		bool found = false;
		
		foreach ( const QString &var, variables )
		{
			if ( text.indexOf( var, i ) == i )
			{
				setFormat( i, var.length(), variable );
				i += var.length()-1;
				found = true;
				break;
			}
		}
		if ( found )
			continue;
		
		foreach ( const QString &f, functions )
		{
			if ( text.indexOf( f, i ) == i )
			{
				setFormat( i, f.length(), function );
				i += f.length()-1;
				found = true;
				break;
			}
		}
		if ( found )
			continue;
		
		ushort u = text[i].unicode();
		bool isFraction = (u >= 0xbc && u <= 0xbe) || (u >= 0x2153 && u <= 0x215e);
		bool isPower = (u >= 0xb2 && u <= 0xb3) || (u == 0x2070) || (u >= 0x2074 && u <= 0x2079);
		bool isDigit = text[i].isDigit();
		bool isDecimalPoint = text[i] == QLocale().decimalPoint();
		
		if ( isFraction || isPower || isDigit || isDecimalPoint )
			setFormat( i, 1, number );
		
		else
			setFormat( i, 1, other );
	}
	
	
	//BEGIN highlight matched brackets
	int cursorPos = m_parent->m_equationEditWidget->textCursor().position();
	if ( cursorPos < 0 )
		cursorPos = 0;
	
	// Adjust cursorpos to allow for a bracket before the cursor position
	if ( cursorPos >= text.size() )
		cursorPos = text.size()-1;
	else if ( cursorPos > 0 && (text[cursorPos-1] == '(' || text[cursorPos-1] == ')') )
		cursorPos--;
	
	bool haveOpen =  text[cursorPos] == '(';
	bool haveClose = text[cursorPos] == ')';
	
	if ( (haveOpen || haveClose) && m_parent->hasFocus() )
	{
		// Search for the other bracket
		
		int inc = haveOpen ? 1 : -1; // which direction to search in
		
		int level = 0;
		for ( int i = cursorPos; i >= 0 && i < text.size(); i += inc )
		{
			if ( text[i] == ')' )
				level--;
			else if ( text[i] == '(' )
				level++;
			
			if ( level == 0 )
			{
				// Matched!
				setFormat( cursorPos, 1, matchedParenthesis );
				setFormat( i, 1, matchedParenthesis );
				break;
			}
		}
	}
	//END highlight matched brackets
	
	
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
	m_cleaningText = false;
	m_settingText = false;
	m_forcingRehighlight = false;
	m_inputType = Expression;
	
	m_equationEditWidget = new EquationEditWidget( this );
	m_highlighter = new EquationHighlighter( this );
	m_equation = new Equation( Equation::Cartesian, 0 );
	m_editButton = new QPushButton( KIcon("edit"), 0, this );
	setFocusProxy( m_equationEditWidget );
	
	connect( m_equationEditWidget, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
	connect( m_editButton, SIGNAL(clicked()), this, SLOT(invokeEquationEditor()) );
	connect( m_equationEditWidget, SIGNAL(cursorPositionChanged()), this, SLOT(reHighlight()) );
	
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
	edit->m_widget->edit->setInputType( m_inputType );
	edit->m_widget->edit->setEquationType( m_equation->type() );
	edit->m_widget->edit->setValidatePrefix( m_validatePrefix );
	edit->m_widget->edit->setText( text() );
	
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
// 		m_replaceMap[ '*' ] = QChar(0xd7);
		m_replaceMap[ '-' ] = MinusSymbol;
		m_replaceMap[ '|' ] = AbsSymbol;
	}
	
	QTextCursor cursor;
	cursor.beginEditBlock();
	for ( CharMap::iterator i = m_replaceMap.begin(); i != m_replaceMap.end(); ++i )
	{
		int at = 0;
		while ( !(cursor = doc->find( i.key(), at )).isNull() )
		{
			at = cursor.position()+1;
			cursor.deleteChar();
			cursor.insertText( i.value() );
		}
	}
	cursor.endEditBlock();
	
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
	int errorPosition;
	
	if ( m_inputType == Function )
		m_equation->setFstr( text, (int*)& error, & errorPosition );
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
//END class EquationEdit



//BEGIN class EquationEditWidget
EquationEditWidget::EquationEditWidget( EquationEdit * parent )
	: QTextEdit( parent )
{
	m_clearSelectionOnFocusOut = true;
	m_parent = parent;
	recalculateGeometry();
}


void EquationEditWidget::recalculateGeometry()
{
	// Set fixed height
	ensurePolished();
	QFontMetrics fm( document()->defaultFont() );
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
		// Still pass these keys to QTextEdit, in case the user has to scroll
		// up/down the text
		if ( e->key() == Qt::Key_Up )
			emit m_parent->upPressed();
		else if ( e->key() == Qt::Key_Down )
			emit m_parent->downPressed();
		
		QTextEdit::keyPressEvent( e );
	}
}


void EquationEditWidget::focusOutEvent( QFocusEvent * e )
{
	QTextEdit::focusOutEvent( e );
	
	if ( m_clearSelectionOnFocusOut )
		clearSelection();
	m_parent->reHighlight();
	
	emit m_parent->editingFinished();
}


void EquationEditWidget::focusInEvent( QFocusEvent * e )
{
	QTextEdit::focusOutEvent( e );

	m_parent->reHighlight();
	if ( e->reason() == Qt::TabFocusReason )
		selectAll();
}


void EquationEditWidget::clearSelection( )
{
	QTextCursor cursor = textCursor();
	if ( !cursor.hasSelection() )
		return;
	
	cursor.clearSelection();
	setTextCursor( cursor );
}
//END class EquationEditWidget



//BEGIN class EquationEditor
EquationEditor::EquationEditor( QWidget * parent )
	: KDialog( parent )
{	
	m_widget = new EquationEditorWidget( this );
	m_widget->edit->showEditButton( false );
	m_widget->edit->m_equationEditWidget->setClearSelectionOnFocusOut( false );
	m_widget->layout()->setMargin( 0 );
	setMainWidget( m_widget );
	
	setCaption( i18n("Equation Editor") );
	setButtons( Close );
	showButtonSeparator( true );
	
	QFont font;
	double pointSize = font.pointSizeF() * 1.1;
	font.setPointSizeF( pointSize );
	m_widget->edit->m_equationEditWidget->document()->setDefaultFont( font );
	m_widget->edit->m_equationEditWidget->recalculateGeometry();
	
	QFont buttonFont;
	buttonFont.setPointSizeF( font.pointSizeF() * 1.1 );
	
	QList<QToolButton *> buttons = m_widget->findChildren<QToolButton *>();
	foreach ( QToolButton * w, buttons )
	{
		KAcceleratorManager::setNoAccel( w );
		
		connect( w, SIGNAL(clicked()), this, SLOT(characterButtonClicked()) );
		
		// Also increase the font size, since the fractions, etc are probably not that visible
		// at the default font size
		w->setFont( buttonFont );
	}
	
	connect( m_widget->constantsButton, SIGNAL(clicked()), MainDlg::self(), SLOT(editConstants()) );
	connect( m_widget->functionList, SIGNAL(activated(const QString &)), this, SLOT(insertFunction(const QString &)) );
	connect( m_widget->constantList, SIGNAL(activated(int)), this, SLOT(insertConstant(int)) );
	
	m_widget->functionList->addItems( XParser::self()->predefinedFunctions(false) );
	
	connect( m_widget->edit, SIGNAL(returnPressed()), this, SLOT(accept()) );
	
	connect( XParser::self()->constants(), SIGNAL(constantsChanged()), this, SLOT(updateConstantList()) );
	updateConstantList();
	
	// Now make the dialog good looking
	resize( layout()->minimumSize() );
}


void EquationEditor::updateConstantList( )
{
	QStringList items;
	
	// The first item text is "Insert constant..."
	items << m_widget->constantList->itemText(0);
	
	ConstantList constants = XParser::self()->constants()->list( Constant::All );
	for ( ConstantList::iterator it = constants.begin(); it != constants.end(); ++it )
	{
		QString text = it.key() + " = " + it.value().value.expression();
		items << text;
	}
	
	m_widget->constantList->clear();
	m_widget->constantList->addItems( items );
}


QString EquationEditor::text() const
{
	return m_widget->edit->text();
}


void EquationEditor::insertFunction( const QString & function )
{
	if ( m_widget->functionList->currentIndex() == 0 )
		return;

	m_widget->functionList->setCurrentIndex( 0 );
	m_widget->edit->wrapSelected( function + '(', ")" );
	m_widget->edit->setFocus();
}


void EquationEditor::insertConstant( int index )
{
	if ( index == 0 )
		return;

	ConstantList constants = XParser::self()->constants()->list( Constant::All );
	
	if ( constants.size() < index )
		return;
	
	// Don't forget that index==0 corresponds to "Insert constant..."
	
	ConstantList::iterator it = constants.begin();
	int at = 0;
	while ( ++at < index )
		++it;
	
	QString constant = it.key();
	
	m_widget->constantList->setCurrentIndex( 0 );
	m_widget->edit->insertText( constant );
	m_widget->edit->setFocus();
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
