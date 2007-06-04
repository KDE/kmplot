/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998-2002  Klaus-Dieter M�ler <kd.moeller@t-online.de>
*                    2004  Fredrik Edemar <f_edemar@linux.se>
*                    2006  David Saxton <david@bluehaze.org>
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

#ifndef FUNCTIONEDITOR_H
#define FUNCTIONEDITOR_H

#include "parser.h"

#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMimeData>

class FunctionEditorWidget;
class FunctionListItem;
class FunctionListWidget;
class KMenu;
class QTimer;
class Function;

class FunctionEditor : public QDockWidget
{
	Q_OBJECT
		
	public:
		FunctionEditor( KMenu * createNewPlotsMenu, QWidget * parent );
		virtual ~ FunctionEditor();
		
		/**
		 * Highlights the function item for the given function.
		 */
		void setCurrentFunction( int functionID );
		
	public slots:
		/**
		 * Deletes the current selected function.
		 */
		void deleteCurrent();
		/**
		 * Creates a new cartesian function.
		 */
		void createCartesian();
		/**
		 * Creates a new parametric function.
		 */
		void createParametric();
		/**
		 * Creates a new polar function.
		 */
		void createPolar();
		/**
		 * Creates a new implicit function.
		 */
		void createImplicit();
		/**
		 * Creates a new differential function.
		 */
		void createDifferential();
		/**
		 * Called when the list of functions in the parser changes.
		 */
		void functionsChanged();
		
	protected slots:
		/**
		 * Called when a function in the list is selected.
		 */
		void functionSelected( QListWidgetItem * function );
		/**
		 * Called when the user changes a function widget.
		 */
		void save();
		/**
		 * Updates the list of functions (called when a function is added or
		 * removed from Parser).
		 */
		void syncFunctionList();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * cartesian function.
		 */
		void saveCartesian();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * polar function.
		 */
		void savePolar();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * parametric function.
		 */
		void saveParametric();
		/**
		 * Called when the user edits any of the widgets relating to an
		 * implicit function.
		 */
		void saveImplicit();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * differential function.
		 */
		void saveDifferential();
		
		
	protected:
		/**
		 * Initialize the wdigets from the cartesian function set in
		 * m_functionID.
		 */
		void initFromCartesian();
		/**
		 * Initialize the widgets from a polar function set in m_functionID.
		 */
		void initFromPolar();
		/**
		 * Initialize the widgets from a parametric function set in
		 * m_functionID.
		 */
		void initFromParametric();
		/**
		 * Initialize the widgets from an implicit function set in m_functionID.
		 */
		void initFromImplicit();
		/**
		 * Initialize the widgets from a differential function set in m_functionID.
		 */
		void initFromDifferential();
		/**
		 * Resets all the function editing widgets (i.e. those in the widget
		 * stack, but not the list of functions).
		 */
		void resetFunctionEditing();
		/**
		 * Extract function \p name and \p expression from the given
		 * \p equation - for implicit functions.
		 */
		void splitImplicitEquation( const QString &equation, QString * name, QString * expression );
		/**
		 * Used at end of the save* functions.
		 * \a tempFunction is the function to copy the settings from.
		 */
		void saveFunction( Function * tempFunction );
		/**
		 * Called from the create* functions - finishes of saving of the function.
		 */
		void createFunction( const QString & eq0, const QString & eq1, Function::Type type );

		/**
		 * The main editing widget.
		 */
		FunctionEditorWidget * m_editor;
		/**
		 * The id of the function currently being edited.
		 */
		int m_functionID;
		
		/**
		 * Used to ensure only one update when multiple widgets are changed.
		 * A timer for each function type.
		 */
		QTimer * m_saveTimer[5];
		/**
		 * Ensure only one update when functions are added or removed, and all
		 * the piece of code that is adding or removing a function to make the
		 * necessary changes first.
		 */
		QTimer * m_syncFunctionListTimer;
		/**
		 * The list of functions.
		 */
		FunctionListWidget * m_functionList;
};


class FunctionListWidget : public QListWidget
{
	public:
		FunctionListWidget( QWidget * parent );
		
	protected:
		virtual void dragEnterEvent( QDragEnterEvent * event );
		virtual void dropEvent( QDropEvent * event );
		virtual QMimeData * mimeData( const QList<QListWidgetItem *> items ) const;
		virtual QStringList mimeTypes() const;
};


class FunctionListItem : public QListWidgetItem
{
	public:
		FunctionListItem( QListWidget * parent, int function );
		
		/**
		 * Updates the displayed text and checked state depending on the
		 * function types and states.
		 */
		void update();
		
		int function() const { return m_function; }
		
	protected:
		int m_function;
};


#endif // FUNCTIONEDITOR_H

