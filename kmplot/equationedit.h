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

#ifndef EQUATIONEDIT_H
#define EQUATIONEDIT_H

#include "function.h"

#include <kdialog.h>

#include <QMap>
#include <QSyntaxHighlighter>
#include <QTextEdit>


class Equation;
class EquationEdit;
class EquationEditWidget;
class EquationEditor;
class EquationEditorWidget;
class QPushButton;

typedef QMap<QChar, QChar> CharMap;


/**
 * \author David Saxton
 */
class EquationHighlighter : public QSyntaxHighlighter
{
	public:
		EquationHighlighter( EquationEdit * parent );
		~EquationHighlighter();
		
		/**
		 * This is used to indicate the position where the error occurred.
		 * If \p position is negative, then no error will be shown.
		 */
		void setErrorPosition( int position );
		
	protected:
		virtual void highlightBlock( const QString & text );
		
		int m_errorPosition;
		EquationEdit * m_parent;
};


/**
 * The actual line edit.
 */
class EquationEditWidget : public QTextEdit
{
	Q_OBJECT
	
	public:
		EquationEditWidget( EquationEdit * parent );
		
		/**
		 * Call this after changing font size.
		 */
		void recalculateGeometry( const QFont & font );
		
	protected:
		virtual void wheelEvent( QWheelEvent * e );
		virtual void keyPressEvent( QKeyEvent * e );
		virtual void focusOutEvent( QFocusEvent * e );
		virtual void focusInEvent( QFocusEvent * e );
		
		EquationEdit * m_parent;
};


/**
 * A line edit for equations that also does syntax highlighting, error checking,
 * etc, and provides a button for invoking an advanced equation editor.
 * \author David Saxton
 */
class EquationEdit : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( QString text READ text WRITE setText USER true )
	
	public:
		enum InputType
		{
			Function,
			Expression
		};
		
		EquationEdit( QWidget * parent );
		
		/**
		 * Set the current error message and position of the error. This is
		 * used to inform the user if his equation is Ok or not.
		 */
		void setError( const QString & message, int position );
		/**
		 * Check the validity of the current text. Called from
		 * EquationHighlighter when it needs to highlight text.
		 */
		void checkTextValidity();
		/**
		 * Sets whether the equation edit should be storing a function (i.e. of
		 * the form f(x)=[expression]), or just an expression (i.e. without the
		 * f(x)= part). By default, this assumes that an expression is being
		 * stored.
		 */
		void setInputType( InputType type );
		/**
		 * Prepends \p prefix to the start of the text when validating it.
		 */
		void setValidatePrefix( const QString & prefix );
		/**
		 * Hide/show the edit button.
		 */
		void showEditButton( bool show );
		/**
		 * Changes the equation type.
		 */
		void setEquationType( Equation::Type type );
		
		QString text() const { return m_equationEditWidget->toPlainText(); }
		void clear() { m_equationEditWidget->clear(); }
		void setReadOnly( bool set ) { m_equationEditWidget->setReadOnly(set); }
		void selectAll() { m_equationEditWidget->selectAll(); }
		void insertText( const QString & text ) { m_equationEditWidget->insertPlainText( text ); }
		
		/**
		 * Attempts to evaluate the text and return it.
		 */
		double value();
		
	signals:
		void editingFinished();
		void textEdited( const QString & text );
		void textChanged( const QString & text );
		void returnPressed();
		
	public Q_SLOTS:
		void setText( const QString & text );
		/**
		 * Launches a dialog for editing the equation.
		 */
		void invokeEquationEditor();
		
		void reHighlight();
		
	protected Q_SLOTS:
		void slotTextChanged();
		
	protected:
		EquationHighlighter * m_highlighter;
		Equation * m_equation;
		InputType m_inputType;
		bool m_settingText:1;
		bool m_cleaningText:1;
		bool m_forcingRehighlight:1;
		QString m_validatePrefix;
		EquationEditWidget * m_equationEditWidget;
		QPushButton * m_editButton;
		
		static CharMap m_replaceMap;
		
		friend class EquationEditor;
		friend class EquationEditWidget;
		friend class EquationHighlighter;
};


/**
 * A dialog containing an EquationEdit and a variety of buttons to insert
 * special characters.
 * \author David Saxton
 */
class EquationEditor : public KDialog
{
	Q_OBJECT
			
	public:
		EquationEditor( QWidget * parent );
		
		/**
		 * The equation's text, e.g. "f(x) = x^2".
		 */
		QString text() const;
		
	protected slots:
		void editConstants();
		void insertFunction( const QString & text );
		void characterButtonClicked();
		
	protected:
		EquationEditorWidget * m_widget;
		
		friend class EquationEdit;
};


#endif

