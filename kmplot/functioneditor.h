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
class QTimer;
class Ufkt;
class View;
class XParser;

class FunctionEditor : public QDockWidget
{
	Q_OBJECT
		
	public:
		FunctionEditor( View * view, QWidget * parent );
		virtual ~ FunctionEditor();
		
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
		 * Called when the "Edit [parameter] List" button is clicked.
		 */
		void editParameterList();
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
		
	protected:
		/**
		 * Initialize the wdigets from the cartesian function set in m_function.
		 */
		void initFromCartesian();
		/**
		 * Initialize the widgets from a polar function set in m_function.
		 */
		void initFromPolar();
		/**
		 * Initialize the widgets from a parametric function set in m_functionX
		 * and m_functionY.
		 */
		void initFromParametric();
		/**
		 * Resets all the function editing widgets (i.e. those in the widget
		 * stack, but not the list of functions).
		 */
		void resetFunctionEditing();
		/**
		 * Extract function \p name and \p expression from the given \p equation
		 */
		void splitParametricEquation( const QString equation, QString * name, QString * expression );
		/**
		 * Check, if the cartesian function has 2 arguments (function group).
		 */
		bool cartesianHasTwoArguments( const QString & function );
		/**
		 * Add an extra argument.
		 */
		void fixCartesianArguments( QString * f_str );
		
		/// The main editing widget.
		FunctionEditorWidget * m_editor;
		/**
		 * The Cartesian or Polar function being edited (the Parametric
		 * functions are stored in m_functionX, m_functionY).
		 */
		int m_function;
		/// The x-coordinate of the parametric function being edited.
		int m_functionX;
		/// The y-coordinate of the parametric function being edited.
		int m_functionY;
		View * m_view;
		
		/// Used to ensure only one update when multiple widgets are changed
		/// @{
		QTimer * m_saveCartesianTimer;
		QTimer * m_savePolarTimer;
		QTimer * m_saveParametricTimer;
		/// @}
		/**
		 * Ensure only one update when functions are added or removed, and all
		 * the piece of code that is adding or removing a function to make the
		 * necessary changes first.
		 */
		QTimer * m_syncFunctionListTimer;
		/**
		 * If we are currently editing a cartesian function, this will be set
		 * to its parameter list.
		 */
		QList<ParameterValueItem> m_parameters;
		/**
		 * The list of functions.
		 */
		FunctionListWidget * m_functionList;
};


class FunctionListWidget : public QListWidget
{
	public:
		FunctionListWidget( QWidget * parent, View * view );
		
	protected:
		virtual void dragEnterEvent( QDragEnterEvent * event );
		virtual void dropEvent( QDropEvent * event );
		virtual QMimeData * mimeData( const QList<QListWidgetItem *> items ) const;
		virtual QStringList mimeTypes() const;
		
		View * m_view;
};


class FunctionListItem : public QListWidgetItem
{
	public:
		FunctionListItem( QListWidget * parent, View * view, int function1, int function2 = -1 );
		
		/**
		 * Updates the displayed text and checked state depending on the
		 * function types and states.
		 */
		void update();
		
		int function1() const { return m_function1; }
		int function2() const { return m_function2; }
		
	protected:
		int m_function1;
		int m_function2;
		View * m_view;
};

#endif // FUNCTIONEDITOR_H

